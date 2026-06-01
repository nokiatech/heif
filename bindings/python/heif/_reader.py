"""
Pythonic wrappers over the raw reader / error-slot C surface.

This module is the *only* public surface of the binding.  Everything
in :mod:`heif._ffi` is an implementation detail.

Design notes
------------

* **Errors.**  Every C call goes through :func:`_check`, which
  raises :class:`HeifError` populated from the thread-local error slot
  on any non-zero return.  After raising, the slot is cleared so
  subsequent successful calls cannot accidentally surface stale
  diagnostics.
* **Resource ownership.**  ``Reader`` is a context manager.  The
  underlying C handle is created lazily on ``__enter__`` (or first
  attribute access outside a ``with`` block) and destroyed
  deterministically on ``__exit__`` / ``close()`` / ``__del__``.
* **Image objects are cheap views.**  ``Image`` carries only the
  ``image_id`` plus a back-reference to the parent ``Reader``.  All
  per-image C calls go through the parent handle, so ``Image``
  objects do not need their own cleanup.
* **``master_images`` is a tuple, not a generator.**  Computed once
  on access and cached.  This matches user expectations
  (``len(reader.master_images)``, indexing, slicing) and aligns with
  the per-handle lazy cache the C side already maintains.
"""

from __future__ import annotations

from typing import NamedTuple, Optional, Tuple

from ._ffi import drain_last_error, ffi, fourcc_to_str, lib

_OK = 0


class ItemProperty(NamedTuple):
    """A property descriptor associated with an image item.

    Mirrors ``HEIF::ItemPropertyInfo`` from the C++ surface.

    Attributes
    ----------
    property_id : int
        The raw ``HEIF::PropertyId`` payload.  Pass back into any
        typed accessor (``Image.rotation``, ``Image.color_profile``, ...)
        -- but bindings normally use the high-level helpers on
        :class:`Image` directly and never touch the id.
    fourcc : str
        4-character box type, e.g. ``"irot"``, ``"ispe"``, ``"colr"``,
        ``"pixi"``, ``"pasp"``.  Sentinel ``"raw "`` (with trailing
        space) for unrecognised box types.
    essential : bool
        True iff the file marks this property as essential -- a
        binding that does not understand an essential property
        MUST refuse to render the item per ISO/IEC 23008-12.
    """

    property_id: int
    fourcc: str
    essential: bool


class ColorProfile(NamedTuple):
    """The colour-information property of an image, in normalised form.

    Exactly one of the two payload shapes is populated:

    * ``profile_type == "nclx"`` -> ``(primaries, transfer, matrix,
      full_range)`` are valid; ``icc_bytes`` is ``b""``.
    * ``profile_type in ("rICC", "prof")`` -> ``icc_bytes`` is the
      ICC profile payload; the nclx fields are 0.

    Bindings that want a single canonical "what colour space is
    this?" answer should branch on ``profile_type`` first.
    """

    profile_type: str
    primaries: int
    transfer: int
    matrix: int
    full_range: bool
    icc_bytes: bytes


class PixiChannel(NamedTuple):
    """A single channel descriptor from the v3 ``pixi`` property's
    ``ChannelData`` array.

    Surfaces the size-prefix (caller-allocated POD struct + size-on-entry)
    accessor ``heif_reader_get_property_pixi_channel_at`` plus the
    ChannelLabel size-probe accessor as a Python NamedTuple.

    Attributes
    ----------
    channel_idc : int
        Channel identifier code (ISO/IEC 23008-12 v3).
        ``0``=Unused, ``1``=Unspecified, ``2``=Y/R,
        ``3``=U(Cb)/G, ``4``=V(Cr)/B, ``5``=Alpha,
        ``6``=Depth, ``7``=Fourth (K).
        This is the field bindings need to disambiguate
        YCbCr 4:2:0 from RGB from grayscale+alpha.
    component_format : int
        Codec-defined ComponentFormat byte.
    subsampling_flag : bool
        True iff ``subsampling_type`` / ``subsampling_location``
        carry meaningful values.
    channel_label_flag : bool
        True iff a ChannelLabel is associated with this channel
        (``label`` will be a non-empty string).
    subsampling_type : int
        Codec-defined subsampling type (e.g. YCbCr 4:2:0/4:2:2/4:4:4).
        ``0`` when ``subsampling_flag`` is False.
    subsampling_location : int
        Codec-defined chroma siting.  ``0`` when
        ``subsampling_flag`` is False.
    label : str
        UTF-8 ChannelLabel string; empty string when the channel
        has no label.
    """

    channel_idc: int
    component_format: int
    subsampling_flag: bool
    channel_label_flag: bool
    subsampling_type: int
    subsampling_location: int
    label: str


#  reader C-facade sweep -- value types
#
#  Each NamedTuple below mirrors a wire struct from the C
#  header.  The Python side never reads/writes the trailing
#  `_reserved` padding directly; cffi handles the byte layout and we
#  read named fields off `cdata`.  The NamedTuple is the *Pythonic*
#  view -- bindings should consume these and never touch the raw
#  cffi cdata.

class ContentLightLevel(NamedTuple):
    """HDR10 content-light-level metadata (`'clli'`)."""
    max_content_light_level: int
    max_pic_average_light_level: int


class MasteringDisplayColorVolume(NamedTuple):
    """HDR10 mastering-display colour-volume metadata (`'mdcv'`).

    `display_primaries` is a tuple of three (x, y) chromaticity
    pairs in (R, G, B) order, each in 0.00002 units per
    SMPTE ST 2086.  `white_point` likewise.  Luminance fields are
    in 0.0001 cd/m^2.
    """
    display_primaries: Tuple[Tuple[int, int], Tuple[int, int], Tuple[int, int]]
    white_point: Tuple[int, int]
    max_display_mastering_luminance: int
    min_display_mastering_luminance: int


class ContentColorVolume(NamedTuple):
    """Extended HDR colour-volume metadata (`'cclv'`).

    Each `*_present` flag gates whether the corresponding payload
    field carries a meaningful value.
    """
    primaries_present: bool
    min_luminance_present: bool
    max_luminance_present: bool
    avg_luminance_present: bool
    primaries: Tuple[Tuple[int, int], Tuple[int, int], Tuple[int, int]]
    min_luminance_value: int
    max_luminance_value: int
    avg_luminance_value: int


class AmbientViewingEnvironment(NamedTuple):
    """Ambient-viewing-environment metadata (`'amve'`)."""
    ambient_illuminance: int
    ambient_light_x: int
    ambient_light_y: int


class ReferenceViewingEnvironment(NamedTuple):
    """Reference-viewing-environment metadata (`'reve'`)."""
    surround_luminance: int
    surround_light_x: int
    surround_light_y: int
    periphery_luminance: int
    periphery_light_x: int
    periphery_light_y: int


class NominalDiffuseWhite(NamedTuple):
    """Nominal-diffuse-white luminance (`'ndwt'`)."""
    diffuse_white_luminance: int


class CleanAperture(NamedTuple):
    """Clean-aperture rectangle (`'clap'`).

    All eight fields are signed-rational pairs.  Width and height
    are the visible aperture's dimensions; offsets are from the
    coded-image centre.
    """
    width_n: int
    width_d: int
    height_n: int
    height_d: int
    horizontal_offset_n: int
    horizontal_offset_d: int
    vertical_offset_n: int
    vertical_offset_d: int


class OinfPtl(NamedTuple):
    """Profile-tier-level entry from an ``'oinf'`` property."""
    general_profile_space: int
    general_tier_flag: int
    general_profile_idc: int
    general_level_idc: int
    general_profile_compatibility_flags: int
    general_constraint_indicator_flags: int


class OinfOpLayer(NamedTuple):
    """Layer entry inside an ``'oinf'`` operating point."""
    ptl_idx: int
    layer_id: int
    is_output_layer: bool
    is_alternate_output_layer: bool


class OinfOp(NamedTuple):
    """Operating point from an ``'oinf'`` property."""
    output_layer_set_idx: int
    max_temporal_id: int
    max_chroma_format: int
    max_bit_depth_minus8: int
    max_pic_width: int
    max_pic_height: int
    min_pic_width: int
    min_pic_height: int
    avg_frame_rate: int
    max_bit_rate: int
    avg_bit_rate: int
    constant_frame_rate: int
    frame_rate_info_flag: bool
    bit_rate_info_flag: bool
    layers: Tuple["OinfOpLayer", ...]


class OinfLayer(NamedTuple):
    """Layer-info entry from an ``'oinf'`` property."""
    dependent_layer_id: int
    dependency_ids: Tuple[int, ...]
    dimension_ids: Tuple[int, ...]


class SubsEntry(NamedTuple):
    """Sub-sample entry from a ``'subs'`` property."""
    subsample_size: int
    subsample_priority: int
    discardable: bool
    codec_specific_parameters: int


class Grid(NamedTuple):
    """Header of a `'grid'` derived item.

    `images` holds the constituent image-IDs in row-major,
    left-to-right order (i.e. `images[row * columns + col]`).
    """
    output_width: int
    output_height: int
    columns: int
    rows: int
    images: Tuple[int, ...]


class OverlayImage(NamedTuple):
    """One member of an `'iovl'` overlay derivation."""
    image_id: int
    horizontal_offset: int
    vertical_offset: int


class Overlay(NamedTuple):
    """Header of a `'iovl'` derived item."""
    fill_r: int
    fill_g: int
    fill_b: int
    fill_a: int
    output_width: int
    output_height: int
    images: Tuple[OverlayImage, ...]


class GainMapChannel(NamedTuple):
    """One channel of a `'tmap'` gain-map (Apple HDR).

    Every metadata field is a signed-rational `(num, den)` pair.
    Numerator types follow the spec: gain min/max and base offsets
    can be negative; gamma and denominators are always positive.
    """
    gain_map_min_numerator: int
    gain_map_min_denominator: int
    gain_map_max_numerator: int
    gain_map_max_denominator: int
    gamma_numerator: int
    gamma_denominator: int
    base_offset_numerator: int
    base_offset_denominator: int
    alternate_offset_numerator: int
    alternate_offset_denominator: int


class GainMap(NamedTuple):
    """Header + channels of a `'tmap'` derived item."""
    minimum_version: int
    writer_version: int
    is_multichannel: bool
    use_base_colour_space: bool
    channel_count: int
    base_hdr_headroom_numerator: int
    base_hdr_headroom_denominator: int
    alternate_hdr_headroom_numerator: int
    alternate_hdr_headroom_denominator: int
    channels: Tuple[GainMapChannel, ...]
    images: Tuple[int, ...]


class EntityGroup(NamedTuple):
    """One entity group from the `'grpl'` box.

    `type` is the FourCC group-type (e.g. `"altr"`, `"ster"`).
    `entities` is the tuple of entity IDs (image IDs or track IDs)
    in declaration order.
    """
    group_id: int
    type: str
    entities: Tuple[int, ...]


# track / sequence value types

class TrackInfo(NamedTuple):
    """Per-track scalar metadata from ``heif_track_info_t``."""
    track_id: int
    features: int
    alternate_group_id: int
    max_sample_size: int
    time_scale: int
    frame_rate_num: int
    frame_rate_den: int
    has_type_information: bool
    major_brand: str
    minor_version: int


class SampleInfo(NamedTuple):
    """Per-sample scalar metadata from ``heif_sample_info_t``."""
    sample_id: int
    sample_duration_ts: int
    size: int
    earliest_timestamp: int
    earliest_timestamp_ts: int
    sample_composition_offset_ts: int
    sample_description_index: int
    segment_id: int
    sample_flags: int
    sample_type: int
    has_clap: bool
    has_auxi: bool
    coding_constraints: int
    sample_entry_type: str


class EditUnit(NamedTuple):
    """Single edit-list entry from ``heif_edit_unit_t``."""
    edit_type: int
    media_time_in_track_ts: int
    duration_in_movie_ts: int
    media_rate_integer: int
    media_rate_fraction: int


class TrackReference(NamedTuple):
    """A ``(reference_type, referenced_track_id)`` pair."""
    reference_type: str
    referenced_track_id: int


class TrackGroup(NamedTuple):
    """A ``(group_type, member_track_id)`` pair."""
    group_type: str
    member_track_id: int


class SegmentInfo(NamedTuple):
    """Per-segment information from ``heif_segment_info_t``."""
    segment_id: int
    earliest_pts_in_ts: int
    start_data_offset: int
    reference_id: int
    timescale: int
    duration_in_ts: int
    data_size: int
    reference_type: int
    starts_with_sap: bool
    sap_type: int


# VVC sample-grouping value types

class SampleEqiv(NamedTuple):
    """``'eqiv'`` visual-equivalence sample grouping."""
    sample_group_description_index: int
    time_offset: int
    timescale_multiplier: int


class SampleStmi(NamedTuple):
    """``'stmi'`` still-image-to-metadata sample grouping.

    ``metadata_item_ids`` lists the item IDs of associated metadata.
    """
    sample_group_description_index: int
    metadata_item_ids: Tuple[int, ...]


class SampleRefs(NamedTuple):
    """``'refs'`` reference-item sample grouping.

    ``reference_item_ids`` lists the referenced item IDs.
    """
    sample_group_description_index: int
    sample_id: int
    reference_item_ids: Tuple[int, ...]


class SampleTrif(NamedTuple):
    """``'trif'`` rectangular-region sample grouping.

    ``dependency_group_ids`` lists dependent region group IDs.
    """
    sample_group_description_index: int
    group_id: int
    independent_idc: int
    horizontal_offset: int
    vertical_offset: int
    region_width: int
    region_height: int
    rect_region_flag: bool
    full_picture: bool
    filtering_disabled: bool
    dependency_group_ids: Tuple[int, ...]


class SampleSpor(NamedTuple):
    """``'spor'`` VVC sub-picture order sample grouping.

    ``subp_track_ref_indices`` lists sub-picture track reference indices.
    """
    sample_group_description_index: int
    subpic_id_bit_pos: int
    subpic_id_info_flag: bool
    subpicd_len_minus1: int
    start_code_emul_flag: bool
    pps_sps_subpic_id_flag: bool
    pps_id: int
    sps_id: int
    subp_track_ref_indices: Tuple[int, ...]


class SampleSulm(NamedTuple):
    """``'sulm'`` VVC sub-picture layout map sample grouping.

    ``group_ids`` lists the group IDs.
    """
    sample_group_description_index: int
    group_id_info_4cc: str
    group_ids: Tuple[int, ...]


class SampleMinp(NamedTuple):
    """``'minp'`` VVC mixed-NAL-unit-type sample grouping.

    ``mix_subp_track_idx1`` and ``mix_subp_track_idx2`` are the two
    variable-length index arrays.
    """
    sample_group_description_index: int
    pps_mix_nalu_types_in_pic_bit_pos: int
    pps_id: int
    mix_subp_track_idx1: Tuple[int, ...]
    mix_subp_track_idx2: Tuple[int, ...]


class HeifError(RuntimeError):
    """Raised on any non-OK return from the underlying C library.

    Attributes
    ----------
    code : int
        Numeric ``HEIF::ErrorCode`` value.
    code_name : str
        Stable human-readable name (e.g. ``"FILE_OPEN_ERROR"``).
    origin : int
        Numeric ``HEIF::ErrorOrigin`` value.
    origin_name : str
        Stable human-readable name (e.g. ``"Reader_FileOpen"``).
    file : str
        Source file inside the library where the failure was recorded.
    line : int
        Source line.
    message : str
        HEIF-level failure description.
    cause : str
        Upstream ``std::exception::what()`` string when available;
        empty when the failure was a plain guard return.

    These fields mirror the error-slot C surface 1:1.
    """

    __slots__ = ("code", "code_name", "origin", "origin_name",
                 "file", "line", "message", "cause")

    def __init__(self, info: dict):
        self.code        = info["code"]
        self.code_name   = info["code_name"]
        self.origin      = info["origin"]
        self.origin_name = info["origin_name"]
        self.file        = info["file"]
        self.line        = info["line"]
        self.message     = info["message"]
        self.cause       = info["cause"]
        text = f"{self.code_name} ({self.code}) from {self.origin_name}: {self.message}"
        if self.cause:
            text += f" [cause: {self.cause}]"
        super().__init__(text)


def _check(rc: int, where: str) -> None:
    """Raise :class:`HeifError` on a non-OK return code, then clear the slot."""
    if rc == _OK:
        return
    info = drain_last_error()
    # Augment the message with where-was-it-called from the binding.
    # The structured fields (code_name, file, line, ...) come straight
    # from the C library; the `where` is binding-side context only.
    info["message"] = f"{where}: {info['message']}" if info["message"] else where
    lib.heif_clear_last_error()
    raise HeifError(info)


def _pixi_channel_to_namedtuple(reader_handle, pid: int, idx: int,
                                rec) -> "PixiChannel":
    """Build a :class:`PixiChannel` from a populated
    ``heif_pixi_channel_t*``, fetching the variable-length label
    via the size-probe accessor.
    """
    label = ""
    if rec.channel_label_flag != 0:
        # Probe.  No-label channels return OK with size 0; that
        # branch is never reached because of the flag check above,
        # but we still handle "OK with len 0" defensively.
        size = ffi.new("uint64_t*")
        size[0] = 0
        rc = lib.heif_reader_get_property_pixi_channel_label(
            reader_handle, pid, idx, ffi.NULL, size
        )
        required = int(size[0])
        if rc == _OK:
            # No label after all.
            label = ""
        else:
            # Expect BUFFER_SIZE_TOO_SMALL with required > 0.
            info = drain_last_error()
            if info["code_name"] != "BUFFER_SIZE_TOO_SMALL" or required == 0:
                lib.heif_clear_last_error()
                info["message"] = (
                    f"get_property_pixi_channel_label probe "
                    f"(property_id={pid}, channel_index={idx}): {info['message']}"
                )
                raise HeifError(info)
            lib.heif_clear_last_error()
            buf = ffi.new("char[]", required)
            cap = ffi.new("uint64_t*")
            cap[0] = required
            _check(
                lib.heif_reader_get_property_pixi_channel_label(
                    reader_handle, pid, idx, buf, cap
                ),
                f"get_property_pixi_channel_label(property_id={pid}, channel_index={idx})",
            )
            # The C side NUL-terminates; ffi.string() stops at the NUL.
            label = ffi.string(buf).decode("utf-8", errors="replace")
    return PixiChannel(
        channel_idc=int(rec.channel_idc),
        component_format=int(rec.component_format),
        subsampling_flag=bool(rec.subsampling_flag),
        channel_label_flag=bool(rec.channel_label_flag),
        subsampling_type=int(rec.subsampling_type),
        subsampling_location=int(rec.subsampling_location),
        label=label,
    )


class Image:
    """A single master image item inside a HEIF/HEIC file.

    Created lazily by :attr:`Reader.master_images`; do not construct
    directly.  Lifetime is bounded by the parent ``Reader``: using
    an ``Image`` after its ``Reader`` has been closed raises
    :class:`HeifError` (``UNINITIALIZED``).
    """

    __slots__ = ("_reader", "image_id")

    def __init__(self, reader: "Reader", image_id: int):
        self._reader = reader
        #: The raw ``HEIF::ImageId`` payload (a 32-bit unsigned int).
        #: Pass this to any future API that takes an image id.
        self.image_id: int = image_id

    # per-image accessors

    @property
    def width(self) -> int:
        """Pixel width of the encoded item."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_width(self._reader._handle, self.image_id, out),
               f"get_width(image_id={self.image_id})")
        return int(out[0])

    @property
    def height(self) -> int:
        """Pixel height of the encoded item."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_height(self._reader._handle, self.image_id, out),
               f"get_height(image_id={self.image_id})")
        return int(out[0])

    @property
    def item_type(self) -> str:
        """FourCC item type (e.g. ``"hvc1"``, ``"avc1"``, ``"vvc1"``,
        ``"jpeg"``, ``"grid"``, ``"iovl"``, ``"iden"``, ``"tmap"``)."""
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_item_type(self._reader._handle, self.image_id, buf),
               f"get_item_type(image_id={self.image_id})")
        return fourcc_to_str(buf)

    def bitstream(self) -> bytes:
        """Return a decoder-ready encoded bitstream for this item.

        The returned ``bytes`` is the decoder configuration record(s)
        followed by the item data with bytestream NAL start-codes
        (``0x000001``) substituted in place of ``nal_length`` fields
        where applicable.  Feed it directly to any decoder that
        speaks the underlying codec -- ``pillow_heif.decode(...)``,
        ``av.open(BytesIO(...))``, ``vvdec`` Rust bindings, etc.

        This call uses the C library's size-probe convention
        internally: one probe call to learn the required size, one
        allocation in Python, one fetch call.  No buffers are
        allocated by the library.
        """
        return self._reader._fetch_bytes_via_probe(
            lib.heif_reader_get_item_data_with_decoder_parameters,
            self.image_id,
            f"get_item_data_with_decoder_parameters(image_id={self.image_id})",
        )

    def raw_item_bytes(self) -> bytes:
        """Return the item payload without the decoder configuration prefix.

        For non-image items (Exif, XMP, MIME) this is the natural
        accessor -- the result is the literal payload as written
        into the file.

        For coded image items this returns the item data **with
        Annex-B start codes already substituted** for the on-wire
        ``nal_length`` fields (the C library applies
        ``bytestreamHeaders=true`` unconditionally), but **without**
        the decoder configuration record that :meth:`bitstream`
        prepends.  In other words: same NAL framing as
        :meth:`bitstream`, just no ``hvcC`` / ``vvcC`` / ``avcC``
        prefix.  For decoding, :meth:`bitstream` is almost always
        what you want instead.

        Same size-probe convention as :meth:`bitstream`.
        """
        return self._reader._fetch_bytes_via_probe(
            lib.heif_reader_get_item_data,
            self.image_id,
            f"get_item_data(image_id={self.image_id})",
        )

    # image properties properties

    @property
    def properties(self) -> Tuple[ItemProperty, ...]:
        """All item properties associated with this image, in file order.

        Returns a tuple of :class:`ItemProperty` triples.  Use the
        higher-level accessors (:attr:`size`, :attr:`rotation`,
        :attr:`mirror`, :attr:`pixel_info`, :attr:`color_profile`,
        :attr:`pixel_aspect_ratio`) for the common cases -- this
        property is the escape hatch for bindings that need to walk
        every property (essential-flag inspection, unrecognised
        box reporting, debugging).
        """
        count = ffi.new("uint32_t*")
        _check(
            lib.heif_reader_get_item_property_count(self._reader._handle, self.image_id, count),
            f"get_item_property_count(image_id={self.image_id})",
        )
        out = []
        pid_buf = ffi.new("uint32_t*")
        type_buf = ffi.new("char[5]")
        ess_buf = ffi.new("int*")
        for i in range(int(count[0])):
            _check(
                lib.heif_reader_get_item_property_at(
                    self._reader._handle, self.image_id, i,
                    pid_buf, type_buf, ess_buf,
                ),
                f"get_item_property_at(image_id={self.image_id}, index={i})",
            )
            out.append(ItemProperty(
                property_id=int(pid_buf[0]),
                fourcc=fourcc_to_str(type_buf),
                essential=bool(ess_buf[0]),
            ))
        return tuple(out)

    def _find_property_id(self, fourcc: str) -> Optional[int]:
        """Return the property_id of the first property with matching
        FourCC, or ``None`` if absent.  Convenience helper for
        single-property lookups (irot/imir/ispe/pasp).  For
        properties that may appear multiple times (colr, pixi)
        callers should walk :attr:`properties` themselves."""
        for p in self.properties:
            if p.fourcc == fourcc:
                return p.property_id
        return None

    @property
    def size(self) -> Optional[Tuple[int, int]]:
        """``(width, height)`` from the ``ispe`` property, or ``None``
        if the item has no ``ispe`` (rare; coded image items always
        do).  Note: this is the *encoded* extent before
        irot/imir/iscl transforms -- see :attr:`width` / :attr:`height`
        for the decoder's view.

        Equivalent to ``(image.width, image.height)`` for plain
        coded items; differs for derived items (grid/iovl/iden/tmap)
        where the encoded extent is bigger or smaller than each
        constituent."""
        pid = self._find_property_id("ispe")
        if pid is None:
            return None
        w = ffi.new("uint32_t*")
        h = ffi.new("uint32_t*")
        _check(
            lib.heif_reader_get_property_ispe(self._reader._handle, pid, w, h),
            f"get_property_ispe(property_id={pid})",
        )
        return (int(w[0]), int(h[0]))

    @property
    def rotation(self) -> int:
        """Counter-clockwise rotation angle in degrees (0/90/180/270)
        from the ``irot`` property.  Returns 0 if no ``irot`` property
        is present (the assumed default per ISO/IEC 23008-12)."""
        pid = self._find_property_id("irot")
        if pid is None:
            return 0
        out = ffi.new("uint32_t*")
        _check(
            lib.heif_reader_get_property_irot(self._reader._handle, pid, out),
            f"get_property_irot(property_id={pid})",
        )
        return int(out[0])

    @property
    def mirror(self) -> Optional[str]:
        """Mirror axis from the ``imir`` property: ``"horizontal"``
        (top<->bottom flip), ``"vertical"`` (left<->right flip), or
        ``None`` if no ``imir`` property is present (the default)."""
        pid = self._find_property_id("imir")
        if pid is None:
            return None
        out = ffi.new("int*")
        _check(
            lib.heif_reader_get_property_imir(self._reader._handle, pid, out),
            f"get_property_imir(property_id={pid})",
        )
        return "horizontal" if out[0] != 0 else "vertical"

    @property
    def pixel_info(self) -> Optional[Tuple[int, ...]]:
        """Per-channel bit depths from the ``pixi`` property as a
        tuple of ``int``.  Length equals the channel count (typical
        values: 3 for YCbCr / RGB, 4 for RGBA, 1 for grayscale).
        Returns ``None`` if no ``pixi`` property is present.

        Example::

            >>> img.pixel_info
            (10, 10, 10)  # 10-bit YCbCr 4:2:0 HDR HEIC
        """
        pid = self._find_property_id("pixi")
        if pid is None:
            return None
        count = ffi.new("uint32_t*")
        _check(
            lib.heif_reader_get_property_pixi_channel_count(self._reader._handle, pid, count),
            f"get_property_pixi_channel_count(property_id={pid})",
        )
        bits = ffi.new("uint8_t*")
        out = []
        for i in range(int(count[0])):
            _check(
                lib.heif_reader_get_property_pixi_bits_at(self._reader._handle, pid, i, bits),
                f"get_property_pixi_bits_at(property_id={pid}, channel_index={i})",
            )
            out.append(int(bits[0]))
        return tuple(out)

    @property
    def pixel_channels(self) -> Optional[Tuple[PixiChannel, ...]]:
        """Per-channel descriptors from the v3 ``pixi`` property's
        ``ChannelData`` array, as a tuple of :class:`PixiChannel`.

        Surfaces the caller-allocated POD struct
        (``heif_pixi_channel_t``) and the size-probe label
        accessor through one Pythonic helper.

        Returns
        -------
        tuple[PixiChannel, ...] or None
            * ``None`` when the image has no ``pixi`` property at all.
            * Empty tuple when the file uses a v0 / v1 ``pixi`` box
              that has only ``bitsPerChannel`` and no v3
              ``ChannelData``.  In that case bindings must fall back
              to :attr:`pixel_info` for bit depths.
            * A non-empty tuple of length :attr:`pixel_info`-length
              when the file uses a v3 ``pixi`` with full
              ``ChannelData``.

        Example::

            >>> img.pixel_channels
            (PixiChannel(channel_idc=2, ..., label='Y'),
             PixiChannel(channel_idc=3, ..., label='Cb'),
             PixiChannel(channel_idc=4, ..., label='Cr'))
        """
        pid = self._find_property_id("pixi")
        if pid is None:
            return None
        # First the v3 numChannels byte.  On a v0/v1 pixi the
        # underlying parse leaves ChannelData empty regardless, so
        # we drive the walk off ChannelData.size by attempting the
        # first entry.
        num = ffi.new("uint8_t*")
        _check(
            lib.heif_reader_get_property_pixi_num_channels(self._reader._handle, pid, num),
            f"get_property_pixi_num_channels(property_id={pid})",
        )
        n = int(num[0])

        # Probe entry 0 to detect "no ChannelData" (v0/v1 pixi).
        # We must not raise on the expected miss; that's a normal
        # asset shape, not an error.
        first = ffi.new("heif_pixi_channel_t*")
        first.struct_size = ffi.sizeof("heif_pixi_channel_t")
        rc = lib.heif_reader_get_property_pixi_channel_at(
            self._reader._handle, pid, 0, first
        )
        if rc != _OK:
            # Drain the error slot left by the guard so the caller's
            # next call sees a clean slate.
            drain_last_error()
            return ()

        out = [_pixi_channel_to_namedtuple(self._reader._handle, pid, 0, first)]
        for i in range(1, n):
            rec = ffi.new("heif_pixi_channel_t*")
            rec.struct_size = ffi.sizeof("heif_pixi_channel_t")
            _check(
                lib.heif_reader_get_property_pixi_channel_at(
                    self._reader._handle, pid, i, rec
                ),
                f"get_property_pixi_channel_at(property_id={pid}, channel_index={i})",
            )
            out.append(_pixi_channel_to_namedtuple(self._reader._handle, pid, i, rec))
        return tuple(out)

    @property
    def color_profile(self) -> Optional[ColorProfile]:
        """The image's colour information from the ``colr`` property,
        or ``None`` if absent.  See :class:`ColorProfile` for the
        return shape.

        For colour-managed display, branch on
        ``profile.profile_type``:

          * ``"nclx"`` -> use the H.273 numeric triple to look up the
            colour space (BT.709, BT.2020, P3-D65, ...) and apply the
            transfer characteristic.
          * ``"rICC"`` / ``"prof"`` -> feed ``profile.icc_bytes`` to
            an ICC engine (LittleCMS, Apple ColorSync, ...)."""
        pid = self._find_property_id("colr")
        if pid is None:
            return None
        type_buf = ffi.new("char[5]")
        _check(
            lib.heif_reader_get_property_colr_type(self._reader._handle, pid, type_buf),
            f"get_property_colr_type(property_id={pid})",
        )
        ptype = fourcc_to_str(type_buf)

        prim = ffi.new("uint16_t*")
        trans = ffi.new("uint16_t*")
        mat = ffi.new("uint16_t*")
        full = ffi.new("int*")
        _check(
            lib.heif_reader_get_property_colr_nclx(
                self._reader._handle, pid, prim, trans, mat, full,
            ),
            f"get_property_colr_nclx(property_id={pid})",
        )

        # Probe ICC payload size, then fetch.  Mirrors
        # _fetch_bytes_via_probe but inlined because the entry-point
        # signature is (reader, prop_id, buf, *size) -- different
        # from the (reader, image_id, buf, *size) shape used there.
        size = ffi.new("uint64_t*")
        size[0] = 0
        rc = lib.heif_reader_get_property_colr_icc(self._reader._handle, pid, ffi.NULL, size)
        icc: bytes
        if rc == _OK:
            icc = b""  # nclx -- no ICC payload
        else:
            info = drain_last_error()
            if info["code_name"] != "BUFFER_SIZE_TOO_SMALL":
                lib.heif_clear_last_error()
                info["message"] = f"get_property_colr_icc (probe, property_id={pid}): {info['message']}"
                raise HeifError(info)
            lib.heif_clear_last_error()
            needed = int(size[0])
            if needed == 0:
                icc = b""
            else:
                buf = ffi.new("uint8_t[]", needed)
                size[0] = needed
                _check(
                    lib.heif_reader_get_property_colr_icc(self._reader._handle, pid, buf, size),
                    f"get_property_colr_icc(property_id={pid})",
                )
                icc = bytes(ffi.buffer(buf, int(size[0])))

        return ColorProfile(
            profile_type=ptype,
            primaries=int(prim[0]),
            transfer=int(trans[0]),
            matrix=int(mat[0]),
            full_range=bool(full[0]),
            icc_bytes=icc,
        )

    @property
    def pixel_aspect_ratio(self) -> Optional[Tuple[int, int]]:
        """``(relative_width, relative_height)`` from the ``pasp``
        property, or ``None`` if the item has no ``pasp`` (the
        common case -- implies square pixels, i.e. (1, 1))."""
        pid = self._find_property_id("pasp")
        if pid is None:
            return None
        w = ffi.new("uint32_t*")
        h = ffi.new("uint32_t*")
        _check(
            lib.heif_reader_get_property_pasp(self._reader._handle, pid, w, h),
            f"get_property_pasp(property_id={pid})",
        )
        return (int(w[0]), int(h[0]))

    # HDR static metadata -- HDR static metadata

    def _try_get_property(self, fourcc: str, c_fn, struct_typedef: str):
        """Generic helper for the size-prefix single-struct getters: find
        the first property of type `fourcc`, allocate a struct,
        set struct_size, dispatch.  Returns the cdata struct on
        success, None when the image has no such property."""
        pid = self._find_property_id(fourcc)
        if pid is None:
            return None
        out = ffi.new(f"{struct_typedef}*")
        out.struct_size = ffi.sizeof(struct_typedef)
        _check(c_fn(self._reader._handle, pid, out),
               f"get_property_{fourcc.strip()}(property_id={pid})")
        return out

    @property
    def content_light_level(self) -> Optional[ContentLightLevel]:
        """`'clli'` HDR content-light metadata, or ``None`` if the
        image has no `clli` property."""
        s = self._try_get_property("clli", lib.heif_reader_get_property_clli,
                                   "heif_clli_t")
        if s is None:
            return None
        return ContentLightLevel(
            max_content_light_level=int(s.max_content_light_level),
            max_pic_average_light_level=int(s.max_pic_average_light_level),
        )

    @property
    def mastering_display(self) -> Optional[MasteringDisplayColorVolume]:
        """`'mdcv'` mastering-display colour volume."""
        s = self._try_get_property("mdcv", lib.heif_reader_get_property_mdcv,
                                   "heif_mdcv_t")
        if s is None:
            return None
        prims = tuple(
            (int(s.display_primaries_x[i]), int(s.display_primaries_y[i]))
            for i in range(3)
        )
        return MasteringDisplayColorVolume(
            display_primaries=prims,
            white_point=(int(s.white_point_x), int(s.white_point_y)),
            max_display_mastering_luminance=int(s.max_display_mastering_luminance),
            min_display_mastering_luminance=int(s.min_display_mastering_luminance),
        )

    @property
    def content_color_volume(self) -> Optional[ContentColorVolume]:
        """`'cclv'` content colour volume."""
        s = self._try_get_property("cclv", lib.heif_reader_get_property_cclv,
                                   "heif_cclv_t")
        if s is None:
            return None
        prims = tuple(
            (int(s.primaries_x[i]), int(s.primaries_y[i])) for i in range(3)
        )
        return ContentColorVolume(
            primaries_present=bool(s.primaries_present),
            min_luminance_present=bool(s.min_luminance_present),
            max_luminance_present=bool(s.max_luminance_present),
            avg_luminance_present=bool(s.avg_luminance_present),
            primaries=prims,
            min_luminance_value=int(s.min_luminance_value),
            max_luminance_value=int(s.max_luminance_value),
            avg_luminance_value=int(s.avg_luminance_value),
        )

    @property
    def ambient_viewing(self) -> Optional[AmbientViewingEnvironment]:
        """`'amve'` ambient-viewing-environment metadata."""
        s = self._try_get_property("amve", lib.heif_reader_get_property_amve,
                                   "heif_amve_t")
        if s is None:
            return None
        return AmbientViewingEnvironment(
            ambient_illuminance=int(s.ambient_illuminance),
            ambient_light_x=int(s.ambient_light_x),
            ambient_light_y=int(s.ambient_light_y),
        )

    @property
    def reference_viewing(self) -> Optional[ReferenceViewingEnvironment]:
        """`'reve'` reference-viewing-environment metadata."""
        s = self._try_get_property("reve", lib.heif_reader_get_property_reve,
                                   "heif_reve_t")
        if s is None:
            return None
        return ReferenceViewingEnvironment(
            surround_luminance=int(s.surround_luminance),
            surround_light_x=int(s.surround_light_x),
            surround_light_y=int(s.surround_light_y),
            periphery_luminance=int(s.periphery_luminance),
            periphery_light_x=int(s.periphery_light_x),
            periphery_light_y=int(s.periphery_light_y),
        )

    @property
    def nominal_diffuse_white(self) -> Optional[NominalDiffuseWhite]:
        """`'ndwt'` nominal-diffuse-white metadata."""
        s = self._try_get_property("ndwt", lib.heif_reader_get_property_ndwt,
                                   "heif_ndwt_t")
        if s is None:
            return None
        return NominalDiffuseWhite(
            diffuse_white_luminance=int(s.diffuse_white_luminance),
        )

    @property
    def clean_aperture(self) -> Optional[CleanAperture]:
        """`'clap'` clean-aperture rectangle."""
        s = self._try_get_property("clap", lib.heif_reader_get_property_clap,
                                   "heif_clap_t")
        if s is None:
            return None
        return CleanAperture(
            width_n=int(s.width_n), width_d=int(s.width_d),
            height_n=int(s.height_n), height_d=int(s.height_d),
            horizontal_offset_n=int(s.horizontal_offset_n),
            horizontal_offset_d=int(s.horizontal_offset_d),
            vertical_offset_n=int(s.vertical_offset_n),
            vertical_offset_d=int(s.vertical_offset_d),
        )

    # long-tail descriptive properties

    @property
    def image_scale(self) -> Optional[Tuple[int, int, int, int]]:
        """``'iscl'`` image scaling as ``(w_n, w_d, h_n, h_d)``, or ``None``."""
        pid = self._find_property_id("iscl")
        if pid is None:
            return None
        wn = ffi.new("uint16_t*")
        wd = ffi.new("uint16_t*")
        hn = ffi.new("uint16_t*")
        hd = ffi.new("uint16_t*")
        _check(lib.heif_reader_get_property_iscl(
            self._reader._handle, pid, wn, wd, hn, hd),
            f"get_property_iscl(property_id={pid})")
        return (int(wn[0]), int(wd[0]), int(hn[0]), int(hd[0]))

    @property
    def relative_location(self) -> Optional[Tuple[int, int]]:
        """``'rloc'`` pixel offset as ``(horizontal, vertical)``, or ``None``."""
        pid = self._find_property_id("rloc")
        if pid is None:
            return None
        h_off = ffi.new("uint32_t*")
        v_off = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_property_rloc(
            self._reader._handle, pid, h_off, v_off),
            f"get_property_rloc(property_id={pid})")
        return (int(h_off[0]), int(v_off[0]))

    def _probe_string_property(self, fourcc: str, c_fn) -> Optional[str]:
        """Size-probe + fetch for a NUL-terminated string property."""
        pid = self._find_property_id(fourcc)
        if pid is None:
            return None
        h = self._reader._handle
        size = ffi.new("uint64_t*", 0)
        c_fn(h, pid, ffi.NULL, size)
        needed = int(size[0])
        if needed == 0:
            return ""
        buf = ffi.new("char[]", needed)
        size[0] = needed
        _check(c_fn(h, pid, buf, size),
               f"get_property_{fourcc}(property_id={pid})")
        return ffi.string(buf).decode("utf-8", errors="replace")

    def _probe_bytes_property(self, fourcc: str, c_fn) -> Optional[bytes]:
        """Size-probe + fetch for a raw-byte property."""
        pid = self._find_property_id(fourcc)
        if pid is None:
            return None
        h = self._reader._handle
        size = ffi.new("uint64_t*", 0)
        c_fn(h, pid, ffi.NULL, size)
        needed = int(size[0])
        if needed == 0:
            return b""
        buf = ffi.new("uint8_t[]", needed)
        size[0] = needed
        _check(c_fn(h, pid, buf, size),
               f"get_property_{fourcc}(property_id={pid})")
        return bytes(ffi.buffer(buf, int(size[0])))

    @property
    def auxiliary_type(self) -> Optional[str]:
        """``'auxC'`` auxiliary type string, or ``None``."""
        return self._probe_string_property("auxC", lib.heif_reader_get_property_auxc_type)

    @property
    def auxiliary_subtype(self) -> Optional[bytes]:
        """``'auxC'`` auxiliary subtype raw bytes, or ``None``."""
        return self._probe_bytes_property("auxC", lib.heif_reader_get_property_auxc_subtype)

    @property
    def required_reference_types(self) -> Optional[Tuple[str, ...]]:
        """``'rref'`` required reference FourCC list, or ``None``."""
        pid = self._find_property_id("rref")
        if pid is None:
            return None
        h = self._reader._handle
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_property_rref_count(h, pid, cnt),
               f"get_property_rref_count(property_id={pid})")
        result = []
        buf = ffi.new("char[5]")
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_property_rref_at(h, pid, i, buf),
                   f"get_property_rref_at(property_id={pid}, index={i})")
            result.append(fourcc_to_str(buf))
        return tuple(result)

    def user_description(self, field_index: int) -> Optional[str]:
        """``'udes'`` user-description field at the given index.

        Field indices: 0 = lang, 1 = name, 2 = description, 3 = tags.
        Returns ``None`` if the item has no ``udes`` property.
        """
        pid = self._find_property_id("udes")
        if pid is None:
            return None
        h = self._reader._handle
        size = ffi.new("uint64_t*", 0)
        lib.heif_reader_get_property_udes_field(h, pid, field_index, ffi.NULL, size)
        needed = int(size[0])
        if needed == 0:
            return ""
        buf = ffi.new("char[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_property_udes_field(h, pid, field_index, buf, size),
               f"get_property_udes_field(property_id={pid}, field={field_index})")
        return ffi.string(buf).decode("utf-8", errors="replace")

    def alternate_text(self, field_index: int) -> Optional[str]:
        """``'altt'`` alternate-text field at the given index.

        Returns ``None`` if the item has no ``altt`` property.
        """
        pid = self._find_property_id("altt")
        if pid is None:
            return None
        h = self._reader._handle
        size = ffi.new("uint64_t*", 0)
        lib.heif_reader_get_property_altt_field(h, pid, field_index, ffi.NULL, size)
        needed = int(size[0])
        if needed == 0:
            return ""
        buf = ffi.new("char[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_property_altt_field(h, pid, field_index, buf, size),
               f"get_property_altt_field(property_id={pid}, field={field_index})")
        return ffi.string(buf).decode("utf-8", errors="replace")

    @property
    def creation_time(self) -> Optional[int]:
        """``'crtt'`` creation timestamp in microseconds, or ``None``."""
        pid = self._find_property_id("crtt")
        if pid is None:
            return None
        out = ffi.new("uint64_t*")
        _check(lib.heif_reader_get_property_crtt(self._reader._handle, pid, out),
               f"get_property_crtt(property_id={pid})")
        return int(out[0])

    @property
    def modification_time(self) -> Optional[int]:
        """``'mdft'`` modification timestamp in microseconds, or ``None``."""
        pid = self._find_property_id("mdft")
        if pid is None:
            return None
        out = ffi.new("uint64_t*")
        _check(lib.heif_reader_get_property_mdft(self._reader._handle, pid, out),
               f"get_property_mdft(property_id={pid})")
        return int(out[0])

    def raw_property_type(self, property_id: int) -> str:
        """FourCC type of any property by its raw ID."""
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_property_raw_type(
            self._reader._handle, property_id, buf),
            f"get_property_raw_type(property_id={property_id})")
        return fourcc_to_str(buf)

    def raw_property_data(self, property_id: int) -> bytes:
        """Raw bytes of any property by its raw ID."""
        h = self._reader._handle
        size = ffi.new("uint64_t*", 0)
        lib.heif_reader_get_property_raw_data(h, property_id, ffi.NULL, size)
        needed = int(size[0])
        if needed == 0:
            return b""
        buf = ffi.new("uint8_t[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_property_raw_data(h, property_id, buf, size),
               f"get_property_raw_data(property_id={property_id})")
        return bytes(ffi.buffer(buf, int(size[0])))

    # multilayer properties (lsel, tols, oinf, subs)

    @property
    def layer_selector(self) -> Optional[int]:
        """``'lsel'`` layer-id, or ``None`` if absent."""
        pid = self._find_property_id("lsel")
        if pid is None:
            return None
        out = ffi.new("uint16_t*")
        _check(lib.heif_reader_get_property_lsel(
            self._reader._handle, pid, out),
            f"get_property_lsel(property_id={pid})")
        return int(out[0])

    @property
    def target_ols_index(self) -> Optional[int]:
        """``'tols'`` target output-layer-set index, or ``None``."""
        pid = self._find_property_id("tols")
        if pid is None:
            return None
        out = ffi.new("uint16_t*")
        _check(lib.heif_reader_get_property_tols(
            self._reader._handle, pid, out),
            f"get_property_tols(property_id={pid})")
        return int(out[0])

    @property
    def operating_points_info(self) -> Optional[Tuple[int, Tuple["OinfPtl", ...], Tuple["OinfOp", ...], Tuple["OinfLayer", ...]]]:
        """``'oinf'`` operating-points-info table.

        Returns ``(scalability_mask, ptl_entries, operating_points,
        layers)`` or ``None`` if absent.
        """
        pid = self._find_property_id("oinf")
        if pid is None:
            return None
        h = self._reader._handle

        mask = ffi.new("uint16_t*")
        _check(lib.heif_reader_get_property_oinf(h, pid, mask),
               f"get_property_oinf(property_id={pid})")

        # PTL array
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_property_oinf_ptl_count(h, pid, cnt),
               f"get_property_oinf_ptl_count(property_id={pid})")
        ptls = []
        for i in range(int(cnt[0])):
            p = ffi.new("heif_oinf_ptl_t*")
            p.struct_size = ffi.sizeof("heif_oinf_ptl_t")
            _check(lib.heif_reader_get_property_oinf_ptl_at(h, pid, i, p),
                   f"get_property_oinf_ptl_at(property_id={pid}, index={i})")
            ptls.append(OinfPtl(
                general_profile_space=int(p.general_profile_space),
                general_tier_flag=int(p.general_tier_flag),
                general_profile_idc=int(p.general_profile_idc),
                general_level_idc=int(p.general_level_idc),
                general_profile_compatibility_flags=int(p.general_profile_compatibility_flags),
                general_constraint_indicator_flags=int(p.general_constraint_indicator_flags),
            ))

        # Operating points + nested layers
        _check(lib.heif_reader_get_property_oinf_op_count(h, pid, cnt),
               f"get_property_oinf_op_count(property_id={pid})")
        ops = []
        for i in range(int(cnt[0])):
            op = ffi.new("heif_oinf_op_t*")
            op.struct_size = ffi.sizeof("heif_oinf_op_t")
            _check(lib.heif_reader_get_property_oinf_op_at(h, pid, i, op),
                   f"get_property_oinf_op_at(property_id={pid}, index={i})")
            lcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_property_oinf_op_layer_count(h, pid, i, lcnt),
                   f"get_property_oinf_op_layer_count(property_id={pid}, op_index={i})")
            layers = []
            for j in range(int(lcnt[0])):
                lyr = ffi.new("heif_oinf_op_layer_t*")
                lyr.struct_size = ffi.sizeof("heif_oinf_op_layer_t")
                _check(lib.heif_reader_get_property_oinf_op_layer_at(h, pid, i, j, lyr),
                       f"get_property_oinf_op_layer_at(property_id={pid}, op={i}, layer={j})")
                layers.append(OinfOpLayer(
                    ptl_idx=int(lyr.ptl_idx),
                    layer_id=int(lyr.layer_id),
                    is_output_layer=bool(lyr.is_output_layer),
                    is_alternate_output_layer=bool(lyr.is_alternate_output_layer),
                ))
            ops.append(OinfOp(
                output_layer_set_idx=int(op.output_layer_set_idx),
                max_temporal_id=int(op.max_temporal_id),
                max_chroma_format=int(op.max_chroma_format),
                max_bit_depth_minus8=int(op.max_bit_depth_minus8),
                max_pic_width=int(op.max_pic_width),
                max_pic_height=int(op.max_pic_height),
                min_pic_width=int(op.min_pic_width),
                min_pic_height=int(op.min_pic_height),
                avg_frame_rate=int(op.avg_frame_rate),
                max_bit_rate=int(op.max_bit_rate),
                avg_bit_rate=int(op.avg_bit_rate),
                constant_frame_rate=int(op.constant_frame_rate),
                frame_rate_info_flag=bool(op.frame_rate_info_flag),
                bit_rate_info_flag=bool(op.bit_rate_info_flag),
                layers=tuple(layers),
            ))

        # Layer-info array
        _check(lib.heif_reader_get_property_oinf_layer_count(h, pid, cnt),
               f"get_property_oinf_layer_count(property_id={pid})")
        layer_infos = []
        for i in range(int(cnt[0])):
            dep_id = ffi.new("uint8_t*")
            _check(lib.heif_reader_get_property_oinf_layer_at(h, pid, i, dep_id),
                   f"get_property_oinf_layer_at(property_id={pid}, index={i})")
            dcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_property_oinf_layer_dep_count(h, pid, i, dcnt),
                   f"get_property_oinf_layer_dep_count(property_id={pid}, layer={i})")
            deps = []
            dep_out = ffi.new("uint8_t*")
            for j in range(int(dcnt[0])):
                _check(lib.heif_reader_get_property_oinf_layer_dep_at(h, pid, i, j, dep_out),
                       f"get_property_oinf_layer_dep_at(property_id={pid}, layer={i}, dep={j})")
                deps.append(int(dep_out[0]))
            _check(lib.heif_reader_get_property_oinf_layer_dim_count(h, pid, i, dcnt),
                   f"get_property_oinf_layer_dim_count(property_id={pid}, layer={i})")
            dims = []
            dim_out = ffi.new("uint8_t*")
            for j in range(int(dcnt[0])):
                _check(lib.heif_reader_get_property_oinf_layer_dim_at(h, pid, i, j, dim_out),
                       f"get_property_oinf_layer_dim_at(property_id={pid}, layer={i}, dim={j})")
                dims.append(int(dim_out[0]))
            layer_infos.append(OinfLayer(
                dependent_layer_id=int(dep_id[0]),
                dependency_ids=tuple(deps),
                dimension_ids=tuple(dims),
            ))

        return (int(mask[0]), tuple(ptls), tuple(ops), tuple(layer_infos))

    @property
    def subsamples(self) -> Optional[Tuple["SubsEntry", ...]]:
        """``'subs'`` sub-sample entries, or ``None`` if absent."""
        pid = self._find_property_id("subs")
        if pid is None:
            return None
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_property_subs_count(
            self._reader._handle, pid, cnt),
            f"get_property_subs_count(property_id={pid})")
        entries = []
        for i in range(int(cnt[0])):
            e = ffi.new("heif_subs_entry_t*")
            e.struct_size = ffi.sizeof("heif_subs_entry_t")
            _check(lib.heif_reader_get_property_subs_at(
                self._reader._handle, pid, i, e),
                f"get_property_subs_at(property_id={pid}, index={i})")
            entries.append(SubsEntry(
                subsample_size=int(e.subsample_size),
                subsample_priority=int(e.subsample_priority),
                discardable=bool(e.discardable),
                codec_specific_parameters=int(e.codec_specific_parameters),
            ))
        return tuple(entries)

    # per-item discoverability

    @property
    def features(self) -> int:
        """Per-item feature bitmask (`HEIF::ItemFeatureEnum::Feature`).

        Bit values (mirror of `srcs/api/reader/heifreaderdatatypes.h`,
        in source order; names are reproduced verbatim from the C
        header, including the historical `Unrecognzed` typo):

          * 0x00000001 -- IsMasterImage
          * 0x00000002 -- IsThumbnailImage
          * 0x00000004 -- IsAuxiliaryImage
          * 0x00000008 -- IsPrimaryImage
          * 0x00000010 -- IsDerivedImage
          * 0x00000020 -- IsPreComputedDerivedImage
          * 0x00000040 -- IsHiddenImage
          * 0x00000080 -- IsCoverImage
          * 0x00000100 -- IsProtected
          * 0x00000200 -- HasLinkedThumbnails
          * 0x00000400 -- HasLinkedAuxiliaryImage
          * 0x00000800 -- HasLinkedDerivedImage
          * 0x00001000 -- HasLinkedPreComputedDerivedImage
          * 0x00002000 -- HasLinkedTiles
          * 0x00004000 -- HasLinkedMetadata
          * 0x00008000 -- IsTileImageItem
          * 0x00010000 -- IsMetadataItem
          * 0x00020000 -- IsExifItem
          * 0x00040000 -- IsXMPItem
          * 0x00080000 -- IsMPEG7Item
          * 0x00100000 -- HasUnrecognzedRequiredReferences
          * 0x00200000 -- IsPredictivelyCodedImage
          * 0x00400000 -- IsGainmapImage

        The C side caches `FileInformation` on first call so this is
        O(1) per image.
        """
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_item_features(self._reader._handle,
                                                 self.image_id, out),
               f"get_item_features(image_id={self.image_id})")
        return int(out[0])

    @property
    def decoder_code_type(self) -> str:
        """Decoder code-type FourCC (e.g. ``'hvc1'``, ``'hev1'``,
        ``'vvc1'``, ``'vvi1'``, ``'avc1'``, ``'jpeg'``).

        Distinguishes the parameter-set placement convention for
        the underlying coded item: ``'hvc1'`` / ``'vvc1'`` /
        ``'avc1'`` keep parameter sets out-of-band (in the
        decoder configuration record), while ``'hev1'`` /
        ``'vvi1'`` permit parameter sets in-band in the slice
        stream.  This is independent of NAL prefix format --
        :meth:`bitstream` already substitutes Annex-B start codes
        regardless of the code type, so consumers feeding a
        bytestream-mode decoder do *not* need to branch on this.

        Bindings that need the decoder configuration record
        separately (rather than as a prefix on :meth:`bitstream`)
        can pair this property with :meth:`raw_item_bytes` to get
        the item payload alone.
        """
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_decoder_code_type(self._reader._handle,
                                                     self.image_id, buf),
               f"get_decoder_code_type(image_id={self.image_id})")
        return fourcc_to_str(buf)

    @property
    def decode_dependencies(self) -> Tuple[int, ...]:
        """Image-IDs whose decoded samples must be available before
        this item can be decoded.  Empty tuple for typical
        independently-coded still-image items."""
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_decode_dependency_count(
                   self._reader._handle, self.image_id, count),
               f"get_decode_dependency_count(image_id={self.image_id})")
        out_id = ffi.new("uint32_t*")
        deps = []
        for i in range(int(count[0])):
            _check(lib.heif_reader_get_decode_dependency_at(
                       self._reader._handle, self.image_id, i, out_id),
                   f"get_decode_dependency_at(image_id={self.image_id}, "
                   f"index={i})")
            deps.append(int(out_id[0]))
        return tuple(deps)

    @property
    def decoder_param_sets(self) -> Tuple[Tuple[int, bytes], ...]:
        """Decoder parameter sets as ``(type, payload)`` pairs.

        ``type`` is the ``HEIF::DecoderSpecInfoType`` numeric value.
        """
        h = self._reader._handle
        iid = self.image_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_decoder_param_set_count(h, iid, cnt),
               f"get_decoder_param_set_count(image_id={iid})")
        result = []
        for i in range(int(cnt[0])):
            t = ffi.new("uint8_t*")
            _check(lib.heif_reader_get_decoder_param_set_type_at(h, iid, i, t),
                   f"get_decoder_param_set_type_at(image_id={iid}, index={i})")
            size = ffi.new("uint64_t*", 0)
            lib.heif_reader_get_decoder_param_set_data_at(h, iid, i, ffi.NULL, size)
            buf = ffi.new("uint8_t[]", int(size[0]) or 1)
            _check(lib.heif_reader_get_decoder_param_set_data_at(h, iid, i, buf, size),
                   f"get_decoder_param_set_data_at(image_id={iid}, index={i})")
            result.append((int(t[0]), bytes(ffi.buffer(buf, int(size[0])))))
        return tuple(result)

    @property
    def decoder_param_sets_all_layers(self) -> Tuple[Tuple[int, bytes], ...]:
        """All-layers decoder parameter sets (VVC multi-layer).

        For non-VVC or single-layer items the result is identical to
        :attr:`decoder_param_sets`.
        """
        h = self._reader._handle
        iid = self.image_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_decoder_param_set_all_layers_count(h, iid, cnt),
               f"get_decoder_param_set_all_layers_count(image_id={iid})")
        result = []
        for i in range(int(cnt[0])):
            t = ffi.new("uint8_t*")
            _check(lib.heif_reader_get_decoder_param_set_all_layers_type_at(h, iid, i, t),
                   f"get_decoder_param_set_all_layers_type_at(image_id={iid}, index={i})")
            size = ffi.new("uint64_t*", 0)
            lib.heif_reader_get_decoder_param_set_all_layers_data_at(h, iid, i, ffi.NULL, size)
            buf = ffi.new("uint8_t[]", int(size[0]) or 1)
            _check(lib.heif_reader_get_decoder_param_set_all_layers_data_at(h, iid, i, buf, size),
                   f"get_decoder_param_set_all_layers_data_at(image_id={iid}, index={i})")
            result.append((int(t[0]), bytes(ffi.buffer(buf, int(size[0])))))
        return tuple(result)

    @property
    def protection_scheme(self) -> Optional[bytes]:
        """Raw ``'sinf'`` protection-scheme bytes, or ``None`` if unprotected."""
        h = self._reader._handle
        iid = self.image_id
        size = ffi.new("uint64_t*", 0)
        rc = lib.heif_reader_get_item_protection_scheme(h, iid, ffi.NULL, size)
        if rc != _OK:
            return None
        needed = int(size[0])
        if needed == 0:
            return b""
        buf = ffi.new("uint8_t[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_item_protection_scheme(h, iid, buf, size),
               f"get_item_protection_scheme(image_id={iid})")
        return bytes(ffi.buffer(buf, int(size[0])))


    def referenced_from(self, reference_type: str) -> Tuple["Image", ...]:
        """Outgoing references: items that THIS item points at.

        Item references are directional -- the FROM side declares
        the relationship and names the TO side.  The meaning of a
        given ``reference_type`` therefore depends on which end
        you're standing at.  This method returns the TO side of
        references that originate at ``self``.

        Common ``reference_type`` values, interpreted from the
        outgoing side:

          * ``"thmb"`` -- on a thumbnail item, the master(s) it
            describes (a master image itself has no outgoing
            ``"thmb"`` references; use :meth:`referenced_to` to
            walk from a master to its thumbnails).
          * ``"cdsc"`` -- on an Exif / XMP / MIME metadata item,
            the image item(s) it describes.
          * ``"auxl"`` -- on an auxiliary item (alpha, depth, ...),
            the image item(s) it augments.
          * ``"dimg"`` -- on a derived item (grid, overlay, iden,
            tmap), the constituent image(s) it composes.
          * ``"base"`` -- on a predictively-coded item, its base
            layer(s).

        Returns a tuple of :class:`Image` views over the referenced
        items, in file order.  Empty tuple if no such references
        originate at this item.
        """
        return self._reader._collect_refs(
            lib.heif_reader_get_referenced_from_item_list_by_type_count,
            lib.heif_reader_get_referenced_from_item_list_by_type_at,
            self.image_id,
            reference_type,
            f"referenced_from(image_id={self.image_id}, type={reference_type!r})",
        )

    def referenced_to(self, reference_type: str) -> Tuple["Image", ...]:
        """Incoming references: items that point AT this item.

        Inverse of :meth:`referenced_from`.  Useful for walking
        from a master image out to its satellites -- given a master,
        :meth:`referenced_to` with ``"thmb"`` returns its
        thumbnails, with ``"auxl"`` returns its auxiliary layers,
        with ``"cdsc"`` returns its Exif / XMP / MIME metadata
        items::

            thumbs = primary.referenced_to("thmb")  # thumbnails of primary
            metas  = primary.referenced_to("cdsc")  # Exif/XMP siblings
            assert thumbs[0].referenced_from("thmb")[0].image_id == primary.image_id
        """
        return self._reader._collect_refs(
            lib.heif_reader_get_referenced_to_item_list_by_type_count,
            lib.heif_reader_get_referenced_to_item_list_by_type_at,
            self.image_id,
            reference_type,
            f"referenced_to(image_id={self.image_id}, type={reference_type!r})",
        )

    def __repr__(self) -> str:
        return (f"<heif.Image image_id={self.image_id} "
                f"type={self.item_type!r} {self.width}x{self.height}>")


class Sample:
    """A single sample inside a track.

    Created by :attr:`Track.samples`; do not construct directly.
    Lifetime is bounded by the parent ``Reader``.
    """

    __slots__ = ("_reader", "_track_id", "sample_id")

    def __init__(self, reader: "Reader", track_id: int, sample_id: int):
        self._reader = reader
        self._track_id = track_id
        self.sample_id: int = sample_id

    @property
    def width(self) -> int:
        """Coded frame width of this sample."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_width(
            self._reader._handle, self._track_id, self.sample_id, out),
            f"get_track_sample_width(track={self._track_id}, sample={self.sample_id})")
        return int(out[0])

    @property
    def height(self) -> int:
        """Coded frame height of this sample."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_height(
            self._reader._handle, self._track_id, self.sample_id, out),
            f"get_track_sample_height(track={self._track_id}, sample={self.sample_id})")
        return int(out[0])

    @property
    def decoder_code_type(self) -> str:
        """Decoder code-type FourCC (e.g. ``'hvc1'``, ``'hev1'``)."""
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_track_sample_decoder_code_type(
            self._reader._handle, self._track_id, self.sample_id, buf),
            f"get_track_sample_decoder_code_type(track={self._track_id}, sample={self.sample_id})")
        return fourcc_to_str(buf)

    @property
    def item_type(self) -> str:
        """Sample-description-entry FourCC."""
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_track_sample_item_type(
            self._reader._handle, self._track_id, self.sample_id, buf),
            f"get_track_sample_item_type(track={self._track_id}, sample={self.sample_id})")
        return fourcc_to_str(buf)

    def bitstream(self) -> bytes:
        """Decoder-ready bitstream (config records + NAL payload).

        Uses the standard size-probe convention.
        """
        h = self._reader._handle
        tid = self._track_id
        sid = self.sample_id
        size = ffi.new("uint64_t*", 0)
        lib.heif_reader_get_track_sample_data_with_decoder_parameters(
            h, tid, sid, ffi.NULL, size)
        buf = ffi.new("uint8_t[]", int(size[0]))
        _check(lib.heif_reader_get_track_sample_data_with_decoder_parameters(
            h, tid, sid, buf, size),
            f"get_track_sample_data_with_decoder_parameters(track={tid}, sample={sid})")
        return bytes(ffi.buffer(buf, int(size[0])))

    def raw_data(self) -> bytes:
        """Raw sample bytes without decoder configuration prefix."""
        h = self._reader._handle
        tid = self._track_id
        sid = self.sample_id
        size = ffi.new("uint64_t*", 0)
        lib.heif_reader_get_track_sample_data(h, tid, sid, ffi.NULL, size)
        buf = ffi.new("uint8_t[]", int(size[0]))
        _check(lib.heif_reader_get_track_sample_data(h, tid, sid, buf, size),
            f"get_track_sample_data(track={tid}, sample={sid})")
        return bytes(ffi.buffer(buf, int(size[0])))

    @property
    def decode_dependencies(self) -> Tuple[int, ...]:
        """Sample IDs that must be decoded before this sample."""
        h = self._reader._handle
        tid = self._track_id
        sid = self.sample_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_decode_dependency_count(
            h, tid, sid, cnt),
            f"get_track_sample_decode_dependency_count(track={tid}, sample={sid})")
        out = ffi.new("uint32_t*")
        deps = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_sample_decode_dependency_at(
                h, tid, sid, i, out),
                f"get_track_sample_decode_dependency_at(track={tid}, sample={sid}, index={i})")
            deps.append(int(out[0]))
        return tuple(deps)

    @property
    def decoder_param_sets(self) -> Tuple[Tuple[int, bytes], ...]:
        """Decoder parameter sets as ``(type, payload)`` pairs.

        ``type`` is the ``HEIF::DecoderSpecInfoType`` numeric value.
        """
        h = self._reader._handle
        tid = self._track_id
        sid = self.sample_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_decoder_param_set_count(
            h, tid, sid, cnt),
            f"get_track_sample_decoder_param_set_count(track={tid}, sample={sid})")
        result = []
        for i in range(int(cnt[0])):
            t = ffi.new("uint8_t*")
            _check(lib.heif_reader_get_track_sample_decoder_param_set_type_at(
                h, tid, sid, i, t),
                f"get_track_sample_decoder_param_set_type_at(track={tid}, sample={sid}, index={i})")
            size = ffi.new("uint64_t*", 0)
            lib.heif_reader_get_track_sample_decoder_param_set_data_at(
                h, tid, sid, i, ffi.NULL, size)
            buf = ffi.new("uint8_t[]", int(size[0]))
            _check(lib.heif_reader_get_track_sample_decoder_param_set_data_at(
                h, tid, sid, i, buf, size),
                f"get_track_sample_decoder_param_set_data_at(track={tid}, sample={sid}, index={i})")
            result.append((int(t[0]), bytes(ffi.buffer(buf, int(size[0])))))
        return tuple(result)

    @property
    def clean_aperture(self) -> Optional["CleanAperture"]:
        """``'clap'`` from the sample-description entry, if present."""
        # heif_sample_info_t.has_clap tells whether clap is present,
        # but the caller may not have the SampleInfo at hand. We try
        # the call and return None on NOT_APPLICABLE / similar.
        s = ffi.new("heif_clap_t*")
        s.struct_size = ffi.sizeof("heif_clap_t")
        # sample_description_index is 1-based; we use 1 as default
        # (most tracks have a single sample entry).
        # The caller who needs a specific SDI should use the raw lib call.
        rc = lib.heif_reader_get_track_sample_clap(
            self._reader._handle, self._track_id, 1, s)
        if rc != _OK:
            return None
        return CleanAperture(
            width_n=int(s.width_n), width_d=int(s.width_d),
            height_n=int(s.height_n), height_d=int(s.height_d),
            horizontal_offset_n=int(s.horizontal_offset_n),
            horizontal_offset_d=int(s.horizontal_offset_d),
            vertical_offset_n=int(s.vertical_offset_n),
            vertical_offset_d=int(s.vertical_offset_d),
        )

    def __repr__(self) -> str:
        return (f"<heif.Sample track={self._track_id} "
                f"sample_id={self.sample_id}>")


class Track:
    """A single track (image sequence) inside a HEIF/HEIC file.

    Created by :attr:`Reader.tracks`; do not construct directly.
    Lifetime is bounded by the parent ``Reader``.
    """

    __slots__ = ("_reader", "track_id")

    def __init__(self, reader: "Reader", track_id: int):
        self._reader = reader
        self.track_id: int = track_id

    @property
    def info(self) -> "TrackInfo":
        """Per-track scalar metadata."""
        out = ffi.new("heif_track_info_t*")
        out.struct_size = ffi.sizeof("heif_track_info_t")
        _check(lib.heif_reader_get_track_info(
            self._reader._handle, self.track_id, out),
            f"get_track_info(track_id={self.track_id})")
        return TrackInfo(
            track_id=int(out.track_id),
            features=int(out.features),
            alternate_group_id=int(out.alternate_group_id),
            max_sample_size=int(out.max_sample_size),
            time_scale=int(out.time_scale),
            frame_rate_num=int(out.frame_rate_num),
            frame_rate_den=int(out.frame_rate_den),
            has_type_information=bool(out.has_type_information),
            major_brand=fourcc_to_str(out.major_brand),
            minor_version=int(out.minor_version),
        )

    @property
    def display_width(self) -> int:
        """Track-header display width."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_display_width(
            self._reader._handle, self.track_id, out),
            f"get_track_display_width(track_id={self.track_id})")
        return int(out[0])

    @property
    def display_height(self) -> int:
        """Track-header display height."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_display_height(
            self._reader._handle, self.track_id, out),
            f"get_track_display_height(track_id={self.track_id})")
        return int(out[0])

    @property
    def playback_duration_secs(self) -> float:
        """Playback duration in seconds, honouring the edit list."""
        out = ffi.new("double*")
        _check(lib.heif_reader_get_track_playback_duration_secs(
            self._reader._handle, self.track_id, out),
            f"get_track_playback_duration_secs(track_id={self.track_id})")
        return float(out[0])

    @property
    def matrix(self) -> Tuple[int, ...]:
        """9-entry ``tkhd`` display matrix (a, b, u, c, d, v, x, y, w)."""
        h = self._reader._handle
        tid = self.track_id
        out = ffi.new("int32_t*")
        vals = []
        for i in range(9):
            _check(lib.heif_reader_get_track_matrix_at(h, tid, i, out),
                   f"get_track_matrix_at(track_id={tid}, index={i})")
            vals.append(int(out[0]))
        return tuple(vals)

    @property
    def samples(self) -> Tuple["SampleInfo", ...]:
        """All samples in sample-table order as ``SampleInfo`` tuples."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_count(h, tid, cnt),
               f"get_track_sample_count(track_id={tid})")
        out = ffi.new("heif_sample_info_t*")
        result = []
        for i in range(int(cnt[0])):
            out.struct_size = ffi.sizeof("heif_sample_info_t")
            _check(lib.heif_reader_get_track_sample_at(h, tid, i, out),
                   f"get_track_sample_at(track_id={tid}, index={i})")
            result.append(SampleInfo(
                sample_id=int(out.sample_id),
                sample_duration_ts=int(out.sample_duration_ts),
                size=int(out.size),
                earliest_timestamp=int(out.earliest_timestamp),
                earliest_timestamp_ts=int(out.earliest_timestamp_ts),
                sample_composition_offset_ts=int(out.sample_composition_offset_ts),
                sample_description_index=int(out.sample_description_index),
                segment_id=int(out.segment_id),
                sample_flags=int(out.sample_flags),
                sample_type=int(out.sample_type),
                has_clap=bool(out.has_clap),
                has_auxi=bool(out.has_auxi),
                coding_constraints=int(out.coding_constraints),
                sample_entry_type=fourcc_to_str(out.sample_entry_type),
            ))
        return tuple(result)

    def sample(self, sample_id: int) -> "Sample":
        """Return a ``Sample`` view for the given sample ID."""
        return Sample(self._reader, self.track_id, sample_id)

    @property
    def master_images(self) -> Tuple[int, ...]:
        """Sample IDs of sync / random-access-point samples."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_master_image_count(h, tid, cnt),
               f"get_track_master_image_count(track_id={tid})")
        out = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_master_image_at(h, tid, i, out),
                   f"get_track_master_image_at(track_id={tid}, index={i})")
            result.append(int(out[0]))
        return tuple(result)

    @property
    def alternate_tracks(self) -> Tuple[int, ...]:
        """Track IDs sharing this track's ``alternateGroupId``."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_alternate_track_count(h, tid, cnt),
               f"get_track_alternate_track_count(track_id={tid})")
        out = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_alternate_track_at(h, tid, i, out),
                   f"get_track_alternate_track_at(track_id={tid}, index={i})")
            result.append(int(out[0]))
        return tuple(result)

    @property
    def references(self) -> Tuple["TrackReference", ...]:
        """``(reference_type, referenced_track_id)`` pairs."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_reference_count(h, tid, cnt),
               f"get_track_reference_count(track_id={tid})")
        ref_type = ffi.new("char[5]")
        ref_id = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_reference_at(
                h, tid, i, ref_type, ref_id),
                f"get_track_reference_at(track_id={tid}, index={i})")
            result.append(TrackReference(
                reference_type=fourcc_to_str(ref_type),
                referenced_track_id=int(ref_id[0]),
            ))
        return tuple(result)

    @property
    def groups(self) -> Tuple["TrackGroup", ...]:
        """``(group_type, member_track_id)`` pairs."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_group_count(h, tid, cnt),
               f"get_track_group_count(track_id={tid})")
        grp_type = ffi.new("char[5]")
        grp_id = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_group_at(
                h, tid, i, grp_type, grp_id),
                f"get_track_group_at(track_id={tid}, index={i})")
            result.append(TrackGroup(
                group_type=fourcc_to_str(grp_type),
                member_track_id=int(grp_id[0]),
            ))
        return tuple(result)

    @property
    def compatible_brands(self) -> Tuple[str, ...]:
        """Per-track compatible brands from ``TrackTypeInformation``."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_compatible_brand_count(h, tid, cnt),
               f"get_track_compatible_brand_count(track_id={tid})")
        buf = ffi.new("char[5]")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_compatible_brand_at(h, tid, i, buf),
                   f"get_track_compatible_brand_at(track_id={tid}, index={i})")
            result.append(fourcc_to_str(buf))
        return tuple(result)

    @property
    def decoding_order(self) -> Tuple[Tuple[int, int], ...]:
        """``(timestamp_ms, sample_id)`` pairs in decoding order."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_decoding_order_count(h, tid, cnt),
               f"get_track_decoding_order_count(track_id={tid})")
        ts = ffi.new("int64_t*")
        sid = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_decoding_order_at(
                h, tid, i, ts, sid),
                f"get_track_decoding_order_at(track_id={tid}, index={i})")
            result.append((int(ts[0]), int(sid[0])))
        return tuple(result)

    @property
    def timestamps(self) -> Tuple[Tuple[int, int], ...]:
        """``(timestamp_ms, sample_id)`` pairs in presentation order."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_item_timestamp_count(h, tid, cnt),
               f"get_track_item_timestamp_count(track_id={tid})")
        ts = ffi.new("int64_t*")
        sid = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_item_timestamp_at(
                h, tid, i, ts, sid),
                f"get_track_item_timestamp_at(track_id={tid}, index={i})")
            result.append((int(ts[0]), int(sid[0])))
        return tuple(result)

    def sample_timestamps(self, sample_id: int) -> Tuple[int, ...]:
        """Timestamps (ms) at which a sample is displayed (edit-list)."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_item_timestamp_count(h, tid, sample_id, cnt),
               f"get_item_timestamp_count(track={tid}, sample={sample_id})")
        ts = ffi.new("int64_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_item_timestamp_at(
                h, tid, sample_id, i, ts),
                f"get_item_timestamp_at(track={tid}, sample={sample_id}, index={i})")
            result.append(int(ts[0]))
        return tuple(result)

    @property
    def edit_list_meta(self) -> Optional[Tuple[bool, float]]:
        """``(looping, repetitions)`` or ``None`` if no edit list."""
        h = self._reader._handle
        tid = self.track_id
        looping = ffi.new("int*")
        reps = ffi.new("double*")
        rc = lib.heif_reader_get_track_edit_list_meta(h, tid, looping, reps)
        if rc != _OK:
            return None
        return (bool(looping[0]), float(reps[0]))

    @property
    def edit_units(self) -> Tuple["EditUnit", ...]:
        """Edit-list entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_edit_unit_count(h, tid, cnt),
               f"get_track_edit_unit_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_edit_unit_t*")
            out.struct_size = ffi.sizeof("heif_edit_unit_t")
            _check(lib.heif_reader_get_track_edit_unit_at(h, tid, i, out),
                   f"get_track_edit_unit_at(track_id={tid}, index={i})")
            result.append(EditUnit(
                edit_type=int(out.edit_type),
                media_time_in_track_ts=int(out.media_time_in_track_ts),
                duration_in_movie_ts=int(out.duration_in_movie_ts),
                media_rate_integer=int(out.media_rate_integer),
                media_rate_fraction=int(out.media_rate_fraction),
            ))
        return tuple(result)

    def samples_by_type(self, sample_type: int) -> Tuple[int, ...]:
        """Sample IDs filtered by ``TrackSampleType`` classification."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_list_by_type_count(
            h, tid, sample_type, cnt),
            f"get_track_sample_list_by_type_count(track_id={tid}, type={sample_type})")
        out = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_sample_list_by_type_at(
                h, tid, sample_type, i, out),
                f"get_track_sample_list_by_type_at(track_id={tid}, type={sample_type}, index={i})")
            result.append(int(out[0]))
        return tuple(result)

    # VVC sample groupings

    @property
    def sample_eqiv(self) -> Tuple["SampleEqiv", ...]:
        """``'eqiv'`` visual-equivalence sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_eqiv_count(h, tid, cnt),
               f"get_track_sample_eqiv_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_eqiv_t*")
            out.struct_size = ffi.sizeof("heif_sample_eqiv_t")
            _check(lib.heif_reader_get_track_sample_eqiv_at(h, tid, i, out),
                   f"get_track_sample_eqiv_at(track_id={tid}, index={i})")
            result.append(SampleEqiv(
                sample_group_description_index=int(out.sample_group_description_index),
                time_offset=int(out.time_offset),
                timescale_multiplier=int(out.timescale_multiplier),
            ))
        return tuple(result)

    @property
    def sample_stmi(self) -> Tuple["SampleStmi", ...]:
        """``'stmi'`` still-image-to-metadata sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_stmi_count(h, tid, cnt),
               f"get_track_sample_stmi_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_stmi_t*")
            out.struct_size = ffi.sizeof("heif_sample_stmi_t")
            _check(lib.heif_reader_get_track_sample_stmi_at(h, tid, i, out),
                   f"get_track_sample_stmi_at(track_id={tid}, index={i})")
            mcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_stmi_metadata_item_count(h, tid, i, mcnt),
                   f"get_track_sample_stmi_metadata_item_count(track_id={tid}, stmi_index={i})")
            mids = []
            mid_out = ffi.new("uint32_t*")
            for j in range(int(mcnt[0])):
                _check(lib.heif_reader_get_track_sample_stmi_metadata_item_at(h, tid, i, j, mid_out),
                       f"get_track_sample_stmi_metadata_item_at(track_id={tid}, stmi={i}, item={j})")
                mids.append(int(mid_out[0]))
            result.append(SampleStmi(
                sample_group_description_index=int(out.sample_group_description_index),
                metadata_item_ids=tuple(mids),
            ))
        return tuple(result)

    @property
    def sample_refs(self) -> Tuple["SampleRefs", ...]:
        """``'refs'`` reference-item sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_refs_count(h, tid, cnt),
               f"get_track_sample_refs_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_refs_t*")
            out.struct_size = ffi.sizeof("heif_sample_refs_t")
            _check(lib.heif_reader_get_track_sample_refs_at(h, tid, i, out),
                   f"get_track_sample_refs_at(track_id={tid}, index={i})")
            rcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_refs_reference_item_count(h, tid, i, rcnt),
                   f"get_track_sample_refs_reference_item_count(track_id={tid}, refs_index={i})")
            rids = []
            rid_out = ffi.new("uint32_t*")
            for j in range(int(rcnt[0])):
                _check(lib.heif_reader_get_track_sample_refs_reference_item_at(h, tid, i, j, rid_out),
                       f"get_track_sample_refs_reference_item_at(track_id={tid}, refs={i}, item={j})")
                rids.append(int(rid_out[0]))
            result.append(SampleRefs(
                sample_group_description_index=int(out.sample_group_description_index),
                sample_id=int(out.sample_id),
                reference_item_ids=tuple(rids),
            ))
        return tuple(result)

    @property
    def sample_trif(self) -> Tuple["SampleTrif", ...]:
        """``'trif'`` rectangular-region sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_trif_count(h, tid, cnt),
               f"get_track_sample_trif_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_trif_t*")
            out.struct_size = ffi.sizeof("heif_sample_trif_t")
            _check(lib.heif_reader_get_track_sample_trif_at(h, tid, i, out),
                   f"get_track_sample_trif_at(track_id={tid}, index={i})")
            dcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_trif_dependency_count(h, tid, i, dcnt),
                   f"get_track_sample_trif_dependency_count(track_id={tid}, trif_index={i})")
            deps = []
            dep_out = ffi.new("uint16_t*")
            for j in range(int(dcnt[0])):
                _check(lib.heif_reader_get_track_sample_trif_dependency_at(h, tid, i, j, dep_out),
                       f"get_track_sample_trif_dependency_at(track_id={tid}, trif={i}, dep={j})")
                deps.append(int(dep_out[0]))
            result.append(SampleTrif(
                sample_group_description_index=int(out.sample_group_description_index),
                group_id=int(out.group_id),
                independent_idc=int(out.independent_idc),
                horizontal_offset=int(out.horizontal_offset),
                vertical_offset=int(out.vertical_offset),
                region_width=int(out.region_width),
                region_height=int(out.region_height),
                rect_region_flag=bool(out.rect_region_flag),
                full_picture=bool(out.full_picture),
                filtering_disabled=bool(out.filtering_disabled),
                dependency_group_ids=tuple(deps),
            ))
        return tuple(result)

    @property
    def sample_spor(self) -> Tuple["SampleSpor", ...]:
        """``'spor'`` VVC sub-picture order sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_spor_count(h, tid, cnt),
               f"get_track_sample_spor_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_spor_t*")
            out.struct_size = ffi.sizeof("heif_sample_spor_t")
            _check(lib.heif_reader_get_track_sample_spor_at(h, tid, i, out),
                   f"get_track_sample_spor_at(track_id={tid}, index={i})")
            rcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_spor_subp_track_ref_idx_count(h, tid, i, rcnt),
                   f"get_track_sample_spor_subp_track_ref_idx_count(track_id={tid}, spor_index={i})")
            refs = []
            ref_out = ffi.new("uint16_t*")
            for j in range(int(rcnt[0])):
                _check(lib.heif_reader_get_track_sample_spor_subp_track_ref_idx_at(h, tid, i, j, ref_out),
                       f"get_track_sample_spor_subp_track_ref_idx_at(track_id={tid}, spor={i}, ref={j})")
                refs.append(int(ref_out[0]))
            result.append(SampleSpor(
                sample_group_description_index=int(out.sample_group_description_index),
                subpic_id_bit_pos=int(out.subpic_id_bit_pos),
                subpic_id_info_flag=bool(out.subpic_id_info_flag),
                subpicd_len_minus1=int(out.subpicd_len_minus1),
                start_code_emul_flag=bool(out.start_code_emul_flag),
                pps_sps_subpic_id_flag=bool(out.pps_sps_subpic_id_flag),
                pps_id=int(out.pps_id),
                sps_id=int(out.sps_id),
                subp_track_ref_indices=tuple(refs),
            ))
        return tuple(result)

    @property
    def sample_sulm(self) -> Tuple["SampleSulm", ...]:
        """``'sulm'`` VVC sub-picture layout map sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_sulm_count(h, tid, cnt),
               f"get_track_sample_sulm_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_sulm_t*")
            out.struct_size = ffi.sizeof("heif_sample_sulm_t")
            _check(lib.heif_reader_get_track_sample_sulm_at(h, tid, i, out),
                   f"get_track_sample_sulm_at(track_id={tid}, index={i})")
            gcnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_sulm_group_id_count(h, tid, i, gcnt),
                   f"get_track_sample_sulm_group_id_count(track_id={tid}, sulm_index={i})")
            gids = []
            gid_out = ffi.new("uint16_t*")
            for j in range(int(gcnt[0])):
                _check(lib.heif_reader_get_track_sample_sulm_group_id_at(h, tid, i, j, gid_out),
                       f"get_track_sample_sulm_group_id_at(track_id={tid}, sulm={i}, group={j})")
                gids.append(int(gid_out[0]))
            result.append(SampleSulm(
                sample_group_description_index=int(out.sample_group_description_index),
                group_id_info_4cc=fourcc_to_str(out.group_id_info_4cc),
                group_ids=tuple(gids),
            ))
        return tuple(result)

    @property
    def sample_minp(self) -> Tuple["SampleMinp", ...]:
        """``'minp'`` VVC mixed-NAL-unit-type sample-grouping entries."""
        h = self._reader._handle
        tid = self.track_id
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_sample_minp_count(h, tid, cnt),
               f"get_track_sample_minp_count(track_id={tid})")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_sample_minp_t*")
            out.struct_size = ffi.sizeof("heif_sample_minp_t")
            _check(lib.heif_reader_get_track_sample_minp_at(h, tid, i, out),
                   f"get_track_sample_minp_at(track_id={tid}, index={i})")
            # idx1 array
            i1cnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_minp_mix_subp_track_idx1_count(h, tid, i, i1cnt),
                   f"get_track_sample_minp_mix_subp_track_idx1_count(track_id={tid}, minp={i})")
            idx1 = []
            i1_out = ffi.new("uint16_t*")
            for j in range(int(i1cnt[0])):
                _check(lib.heif_reader_get_track_sample_minp_mix_subp_track_idx1_at(h, tid, i, j, i1_out),
                       f"get_track_sample_minp_mix_subp_track_idx1_at(track_id={tid}, minp={i}, idx1={j})")
                idx1.append(int(i1_out[0]))
            # idx2 array
            i2cnt = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_track_sample_minp_mix_subp_track_idx2_count(h, tid, i, i2cnt),
                   f"get_track_sample_minp_mix_subp_track_idx2_count(track_id={tid}, minp={i})")
            idx2 = []
            i2_out = ffi.new("uint16_t*")
            for j in range(int(i2cnt[0])):
                _check(lib.heif_reader_get_track_sample_minp_mix_subp_track_idx2_at(h, tid, i, j, i2_out),
                       f"get_track_sample_minp_mix_subp_track_idx2_at(track_id={tid}, minp={i}, idx2={j})")
                idx2.append(int(i2_out[0]))
            result.append(SampleMinp(
                sample_group_description_index=int(out.sample_group_description_index),
                pps_mix_nalu_types_in_pic_bit_pos=int(out.pps_mix_nalu_types_in_pic_bit_pos),
                pps_id=int(out.pps_id),
                mix_subp_track_idx1=tuple(idx1),
                mix_subp_track_idx2=tuple(idx2),
            ))
        return tuple(result)

    # auxiliary type info

    def auxi_type(self, sample_description_index: int = 1) -> Optional[str]:
        """Auxiliary type URN for the given sample description index, or ``None``."""
        h = self._reader._handle
        tid = self.track_id
        size = ffi.new("uint64_t*", 0)
        rc = lib.heif_reader_get_track_sample_auxi_type(
            h, tid, sample_description_index, ffi.NULL, size)
        if rc != _OK:
            return None
        needed = int(size[0])
        if needed == 0:
            return ""
        buf = ffi.new("char[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_track_sample_auxi_type(
            h, tid, sample_description_index, buf, size),
            f"get_track_sample_auxi_type(track_id={tid}, sdi={sample_description_index})")
        return ffi.string(buf).decode("utf-8", errors="replace")

    def auxi_subtype(self, sample_description_index: int = 1) -> Optional[bytes]:
        """Auxiliary subtype bytes for the given sample description index, or ``None``."""
        h = self._reader._handle
        tid = self.track_id
        size = ffi.new("uint64_t*", 0)
        rc = lib.heif_reader_get_track_sample_auxi_subtype(
            h, tid, sample_description_index, ffi.NULL, size)
        if rc != _OK:
            return None
        needed = int(size[0])
        if needed == 0:
            return b""
        buf = ffi.new("uint8_t[]", needed)
        size[0] = needed
        _check(lib.heif_reader_get_track_sample_auxi_subtype(
            h, tid, sample_description_index, buf, size),
            f"get_track_sample_auxi_subtype(track_id={tid}, sdi={sample_description_index})")
        return bytes(ffi.buffer(buf, int(size[0])))

    def __repr__(self) -> str:
        return f"<heif.Track track_id={self.track_id}>"


class _SegmentStream:
    """Internal wrapper around a `heif_stream_t` that pins the four
    cffi callback objects (read / absolute_seek / tell / size) plus
    the user-side ``BinaryIO`` for as long as the C library may call
    back into Python.

    The library copies the callback table by value but holds the
    four function pointers and ``user_data`` by reference, so each
    Python-side closure object MUST stay alive until the matching
    invalidate / destroy call.  Releasing the cffi.callback objects
    earlier frees the underlying JIT trampolines and turns any
    later library callback into a use-after-free.

    The instance is owned by :class:`Reader` (via
    ``Reader._init_stream`` for the initialization segment, or
    ``Reader._segment_streams[segment_id]`` for media segments).
    Direct construction by user code is not part of the public API.
    """

    __slots__ = (
        "_handle", "_src",
        "_cb_read", "_cb_seek", "_cb_tell", "_cb_size",
        "_cb_table",
    )

    def __init__(self, src):
        self._handle = ffi.NULL
        self._src = src

        # Define the four callbacks as bound closures over `src`.
        # cffi.callback() returns a CData object that owns a JIT
        # trampoline; releasing the Python object frees the
        # trampoline, so we keep strong refs as instance attrs.
        @ffi.callback("int64_t(void*, char*, int64_t)")
        def _read(_user_data, buffer, size):
            try:
                data = src.read(int(size))
            except Exception:
                return -1
            if not data:
                return 0
            n = len(data)
            ffi.memmove(buffer, data, n)
            return n

        @ffi.callback("int(void*, int64_t)")
        def _seek(_user_data, offset):
            try:
                src.seek(int(offset))
                return 1  # truthy = success per StreamInterface contract
            except Exception:
                return 0

        @ffi.callback("int64_t(void*)")
        def _tell(_user_data):
            try:
                return int(src.tell())
            except Exception:
                return -1

        @ffi.callback("int64_t(void*)")
        def _size(_user_data):
            try:
                here = src.tell()
                src.seek(0, 2)  # SEEK_END
                end = src.tell()
                src.seek(here)
                return int(end)
            except Exception:
                return -1

        self._cb_read = _read
        self._cb_seek = _seek
        self._cb_tell = _tell
        self._cb_size = _size

        # Build the callback table.  Pin it as an instance attr so
        # cffi does not garbage-collect it while heif_stream_create
        # is copying it.  The library copies by value (per the C
        # contract) but the conservative pin matches the wiki guide.
        table = ffi.new("heif_stream_callbacks_t*")
        table.struct_size   = ffi.sizeof("heif_stream_callbacks_t")
        table.read          = _read
        table.absolute_seek = _seek
        table.tell          = _tell
        table.size          = _size
        table.user_data     = ffi.NULL
        self._cb_table = table

        h = lib.heif_stream_create(table)
        if h == ffi.NULL:
            raise MemoryError("heif_stream_create returned NULL "
                              "(allocation failed or callback table invalid)")
        self._handle = h

    @property
    def handle(self):
        """The raw `heif_stream_t*` for passing to `lib.heif_reader_*`."""
        return self._handle

    def close(self) -> None:
        """Destroy the C handle.  Idempotent.  MUST NOT be called while
        a reader still references the underlying stream."""
        h = getattr(self, "_handle", ffi.NULL)
        if h != ffi.NULL:
            lib.heif_stream_destroy(h)
            self._handle = ffi.NULL

    def __del__(self) -> None:
        try:
            self.close()
        except Exception:
            pass


class Reader:
    """Context-managed HEIF/HEIC reader.

    Typical usage::

        with heif.Reader("photo.heic") as r:
            print(r.major_brand)
            for img in r.master_images:
                payload = img.bitstream()

    The C library handle is allocated in :meth:`__init__` and
    released by :meth:`close` (called automatically on ``__exit__``
    or garbage collection).  After ``close()`` the reader is
    unusable; create a new one to re-open the file.
    """

    __slots__ = ("_handle", "_path", "_master_images_cache",
                 "_init_stream", "_segment_streams")

    def __init__(self, path: str):
        self._handle = lib.heif_reader_create()
        if self._handle == ffi.NULL:
            raise MemoryError("heif_reader_create returned NULL "
                              "(library allocation failed)")
        self._path = path
        self._master_images_cache: Optional[Tuple[Image, ...]] = None
        self._init_stream: Optional["_SegmentStream"] = None
        self._segment_streams: dict = {}

        try:
            encoded = path.encode("utf-8")
        except UnicodeEncodeError as exc:
            self.close()
            raise ValueError(f"path is not encodable as UTF-8: {path!r}") from exc

        try:
            _check(lib.heif_reader_initialize_file(self._handle, encoded),
                   f"initialize_file({path!r})")
        except Exception:
            self.close()
            raise

    @classmethod
    def from_stream(cls, src) -> "Reader":
        """Open a reader from a file-like ``BinaryIO`` object.

        The stream must support ``read``, ``seek``, ``tell``, and be
        seekable.  The reader takes ownership of the stream callbacks
        for its lifetime; the caller retains ownership of *src* and
        must keep it open until the reader is closed.
        """
        self = object.__new__(cls)
        self._handle = lib.heif_reader_create()
        if self._handle == ffi.NULL:
            raise MemoryError("heif_reader_create returned NULL "
                              "(library allocation failed)")
        self._path = "<stream>"
        self._master_images_cache = None
        self._init_stream = None
        self._segment_streams = {}

        stream = _SegmentStream(src)
        try:
            _check(lib.heif_reader_initialize_stream(
                       self._handle, stream._cb_table),
                   "initialize_stream")
        except Exception:
            stream.close()
            self.close()
            raise
        self._init_stream = stream
        return self

    # file-level accessors

    @property
    def major_brand(self) -> str:
        """Major brand of the file (e.g. ``"heic"``, ``"mif1"``, ``"mif3"``,
        ``"msf1"``)."""
        buf = ffi.new("char[5]")
        _check(lib.heif_reader_get_major_brand(self._handle, buf),
               "get_major_brand")
        return fourcc_to_str(buf)

    @property
    def minor_version(self) -> int:
        """Minor version field of the ``ftyp`` box (vendor-defined integer)."""
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_minor_version(self._handle, out),
               "get_minor_version")
        return int(out[0])

    @property
    def compatible_brands(self) -> Tuple[str, ...]:
        """All compatible brands declared in the ``ftyp`` box, in file order.

        Returned as a tuple of 4-character strings.  Computed once
        per access via the C side's lazy cache, so calling this in
        a hot loop is cheap.
        """
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_compatible_brand_count(self._handle, count),
               "get_compatible_brand_count")
        buf = ffi.new("char[5]")
        out = []
        for i in range(int(count[0])):
            _check(lib.heif_reader_get_compatible_brand_at(self._handle, i, buf),
                   f"get_compatible_brand_at(index={i})")
            out.append(fourcc_to_str(buf))
        return tuple(out)

    @property
    def primary_image(self) -> Optional["Image"]:
        """The file's declared primary item, or ``None`` if no primary is set.

        Bindings that present a single image (rather than a gallery)
        should select this and decode its :meth:`Image.bitstream`.
        Falls through to ``None`` (rather than raising) when the
        file declares no primary, since "no primary set" is a
        legitimate file shape and the caller usually wants to fall
        back to ``master_images[0]``.
        """
        out_id = ffi.new("uint32_t*")
        rc = lib.heif_reader_get_primary_item(self._handle, out_id)
        if rc == _OK:
            return Image(self, int(out_id[0]))
        # PRIMARY_ITEM_NOT_SET is the expected non-error path -- we
        # match by code_name (resilient to numeric renumbering).
        info = drain_last_error()
        lib.heif_clear_last_error()
        if info["code_name"] == "PRIMARY_ITEM_NOT_SET":
            return None
        raise HeifError(info)

    # type-filtered enumeration

    def items_by_type(self, item_type: str) -> Tuple["Image", ...]:
        """All items in the file whose item-type FourCC equals *item_type*.

        ``item_type`` is a 4-character string; common values:
        ``"hvc1"``, ``"avc1"``, ``"vvc1"``, ``"jpeg"``, ``"grid"``,
        ``"iovl"``, ``"iden"``, ``"tmap"``, ``"Exif"``, ``"mime"``.

        Note that for non-image item types (``"Exif"``, ``"mime"``,
        ...) the returned ``Image`` objects are still useful for
        ``raw_item_bytes()``; the ``width`` / ``height`` /
        ``bitstream`` properties are only meaningful for coded
        image items.
        """
        if not isinstance(item_type, str):
            raise TypeError(f"item_type must be str, got {type(item_type).__name__}")
        type_bytes = item_type.encode("ascii")
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_item_list_by_type_count(self._handle, type_bytes, count),
               f"get_item_list_by_type_count(type={item_type!r})")
        out_id = ffi.new("uint32_t*")
        items = []
        for i in range(int(count[0])):
            _check(lib.heif_reader_get_item_list_by_type_at(self._handle, type_bytes, i, out_id),
                   f"get_item_list_by_type_at(type={item_type!r}, index={i})")
            items.append(Image(self, int(out_id[0])))
        return tuple(items)

    # internal helpers (used by Image)

    def _fetch_bytes_via_probe(self, c_fn, image_id: int, where: str) -> bytes:
        """Run the standard size-probe / allocate / fetch dance.

        Shared between :meth:`Image.bitstream` and
        :meth:`Image.raw_item_bytes`; both have identical
        ``(reader, image_id, buffer, size_inout)`` signatures and
        identical contract.
        """
        size = ffi.new("uint64_t*")
        size[0] = 0
        rc = c_fn(self._handle, image_id, ffi.NULL, size)
        if rc == _OK:
            # Zero-byte item -- return empty bytes per the C contract.
            return b""
        info = drain_last_error()
        if info["code_name"] != "BUFFER_SIZE_TOO_SMALL":
            lib.heif_clear_last_error()
            # Augment with binding-side context so the traceback is
            # actionable without inspecting `info` directly.
            info["message"] = f"{where} (probe): {info['message']}"
            raise HeifError(info)
        lib.heif_clear_last_error()

        needed = int(size[0])
        if needed == 0:
            return b""
        buf = ffi.new("uint8_t[]", needed)
        size[0] = needed
        _check(c_fn(self._handle, image_id, buf, size), where)
        actual = int(size[0])
        return bytes(ffi.buffer(buf, actual))

    def _collect_refs(self, count_fn, at_fn,
                      image_id: int, reference_type: str,
                      where: str) -> Tuple["Image", ...]:
        """Run the standard ``(_count, _at[0..n))`` collection loop.

        Used by both :meth:`Image.referenced_from` and
        :meth:`Image.referenced_to`; the C-side single-entry cache
        keyed on ``(image_id, reference_type)`` makes this O(n)
        in entry-point calls.
        """
        if not isinstance(reference_type, str):
            raise TypeError(f"reference_type must be str, got {type(reference_type).__name__}")
        ref_bytes = reference_type.encode("ascii")
        count = ffi.new("uint32_t*")
        _check(count_fn(self._handle, image_id, ref_bytes, count), where)
        out_id = ffi.new("uint32_t*")
        refs = []
        for i in range(int(count[0])):
            _check(at_fn(self._handle, image_id, ref_bytes, i, out_id),
                   f"{where}[index={i}]")
            refs.append(Image(self, int(out_id[0])))
        return tuple(refs)

    @property
    def master_images(self) -> Tuple[Image, ...]:
        """All master image items in the file, in file order.

        Computed once on first access and cached.  Returned as a
        tuple so it supports ``len()``, indexing, slicing, and
        iteration -- the natural Python idioms for "a sequence of
        things you got from a file".
        """
        if self._master_images_cache is not None:
            return self._master_images_cache

        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_master_image_count(self._handle, count),
               "get_master_image_count")

        out_id = ffi.new("uint32_t*")
        images = []
        for i in range(int(count[0])):
            _check(lib.heif_reader_get_master_image_at(self._handle, i, out_id),
                   f"get_master_image_at(index={i})")
            images.append(Image(self, int(out_id[0])))

        self._master_images_cache = tuple(images)
        return self._master_images_cache

    @property
    def tracks(self) -> Tuple["Track", ...]:
        """All tracks in the file, in file order."""
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_track_count(self._handle, cnt),
               "get_track_count")
        out_id = ffi.new("uint32_t*")
        result = []
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_track_id_at(self._handle, i, out_id),
                   f"get_track_id_at(index={i})")
            result.append(Track(self, int(out_id[0])))
        return tuple(result)

    @property
    def segment_index(self) -> Tuple["SegmentInfo", ...]:
        """Segment index entries from ``sidx`` boxes, if present."""
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_segment_index_count(self._handle, cnt),
               "get_segment_index_count")
        result = []
        for i in range(int(cnt[0])):
            out = ffi.new("heif_segment_info_t*")
            out.struct_size = ffi.sizeof("heif_segment_info_t")
            _check(lib.heif_reader_get_segment_index_at(self._handle, i, out),
                   f"get_segment_index_at(index={i})")
            result.append(SegmentInfo(
                segment_id=int(out.segment_id),
                earliest_pts_in_ts=int(out.earliest_pts_in_ts),
                start_data_offset=int(out.start_data_offset),
                reference_id=int(out.reference_id),
                timescale=int(out.timescale),
                duration_in_ts=int(out.duration_in_ts),
                data_size=int(out.data_size),
                reference_type=int(out.reference_type),
                starts_with_sap=bool(out.starts_with_sap),
                sap_type=int(out.sap_type),
            ))
        return tuple(result)

    @property
    def compatible_brand_combos(self) -> Tuple[Tuple[str, ...], ...]:
        """Compatible brand combinations from ``ftyp`` / ``styp`` boxes.

        Returns a tuple-of-tuples; each inner tuple is one brand combo.
        """
        combo_cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_compatible_brand_combo_count(
            self._handle, combo_cnt), "get_compatible_brand_combo_count")
        result = []
        brand_cnt = ffi.new("uint32_t*")
        buf = ffi.new("char[5]")
        for ci in range(int(combo_cnt[0])):
            _check(lib.heif_reader_get_compatible_brand_combo_brand_count(
                self._handle, ci, brand_cnt),
                f"get_compatible_brand_combo_brand_count(combo={ci})")
            brands = []
            for bi in range(int(brand_cnt[0])):
                _check(lib.heif_reader_get_compatible_brand_combo_brand_at(
                    self._handle, ci, bi, buf),
                    f"get_compatible_brand_combo_brand_at(combo={ci}, brand={bi})")
                brands.append(fourcc_to_str(buf))
            result.append(tuple(brands))
        return tuple(result)

    @property
    def matrix(self) -> Tuple[int, ...]:
        """File-level (``mvhd``) 3x3 transformation matrix (9 ints)."""
        vals = []
        out = ffi.new("int32_t*")
        for i in range(9):
            _check(lib.heif_reader_get_matrix_at(self._handle, i, out),
                   f"get_matrix_at(index={i})")
            vals.append(int(out[0]))
        return tuple(vals)

    def group_properties(self, group_id: int) -> Tuple[Tuple[int, str, bool], ...]:
        """Properties associated with an entity group.

        Returns tuples of ``(property_id, fourcc_type, is_essential)``.
        """
        cnt = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_group_property_count(
            self._handle, group_id, cnt),
            f"get_group_property_count(group_id={group_id})")
        result = []
        out_pid = ffi.new("uint32_t*")
        out_type = ffi.new("char[5]")
        out_ess = ffi.new("uint8_t*")
        for i in range(int(cnt[0])):
            _check(lib.heif_reader_get_group_property_at(
                self._handle, group_id, i, out_pid, out_type, out_ess),
                f"get_group_property_at(group_id={group_id}, index={i})")
            result.append((int(out_pid[0]), fourcc_to_str(out_type), bool(out_ess[0])))
        return tuple(result)

    # file-level discoverability

    @property
    def file_features(self) -> int:
        """File-level feature bitmask (`HEIF::FileFeatureEnum::Feature`).

        Bit values (mirror of `srcs/api/reader/heifreaderdatatypes.h`,
        in source order):

          * 0x01 -- HasSingleImage
          * 0x02 -- HasImageCollection
          * 0x04 -- HasImageSequence
          * 0x08 -- HasRootLevelMetaBox
          * 0x10 -- HasAlternateTracks
        """
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_file_features(self._handle, out),
               "get_file_features")
        return int(out[0])

    @property
    def metabox_features(self) -> int:
        """Root-metabox feature bitmask
        (`HEIF::MetaBoxFeatureEnum::Feature`).

        Bit values (mirror of `srcs/api/reader/heifreaderdatatypes.h`,
        in source order):

          * 0x001 -- IsSingleImage
          * 0x002 -- IsImageCollection
          * 0x004 -- HasMasterImages
          * 0x008 -- HasThumbnails
          * 0x010 -- HasAuxiliaryImages
          * 0x020 -- HasDerivedImages
          * 0x040 -- HasPreComputedDerivedImages
          * 0x080 -- HasHiddenImages
          * 0x100 -- HasGroupLists
        """
        out = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_metabox_features(self._handle, out),
               "get_metabox_features")
        return int(out[0])

    # grid + overlay -- derived items: grid + overlay

    def grid(self, image_id: int) -> "Grid":
        """Decode a `'grid'` derived item into its header + image list.

        Raises :class:`HeifError` (with code `INVALID_ITEM_ID` or
        similar) when *image_id* is not a `grid` item.  Use
        :meth:`Image.item_type` to filter first if you are not sure.
        """
        out = ffi.new("heif_grid_t*")
        out.struct_size = ffi.sizeof("heif_grid_t")
        _check(lib.heif_reader_get_grid(self._handle, image_id, out),
               f"get_grid(image_id={image_id})")
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_grid_image_count(self._handle, image_id, count),
               f"get_grid_image_count(image_id={image_id})")
        ids = ffi.new("uint32_t*")
        images = []
        for i in range(int(count[0])):
            _check(lib.heif_reader_get_grid_image_at(self._handle, image_id, i, ids),
                   f"get_grid_image_at(image_id={image_id}, index={i})")
            images.append(int(ids[0]))
        return Grid(
            output_width=int(out.output_width),
            output_height=int(out.output_height),
            columns=int(out.columns),
            rows=int(out.rows),
            images=tuple(images),
        )

    def overlay(self, image_id: int) -> "Overlay":
        """Decode a `'iovl'` derived item into its header + image list."""
        out = ffi.new("heif_overlay_t*")
        out.struct_size = ffi.sizeof("heif_overlay_t")
        _check(lib.heif_reader_get_overlay(self._handle, image_id, out),
               f"get_overlay(image_id={image_id})")
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_overlay_image_count(self._handle, image_id, count),
               f"get_overlay_image_count(image_id={image_id})")
        oimg = ffi.new("heif_overlay_image_t*")
        oimg.struct_size = ffi.sizeof("heif_overlay_image_t")
        images = []
        for i in range(int(count[0])):
            oimg.struct_size = ffi.sizeof("heif_overlay_image_t")
            _check(lib.heif_reader_get_overlay_image_at(self._handle, image_id, i, oimg),
                   f"get_overlay_image_at(image_id={image_id}, index={i})")
            images.append(OverlayImage(
                image_id=int(oimg.image_id),
                horizontal_offset=int(oimg.horizontal_offset),
                vertical_offset=int(oimg.vertical_offset),
            ))
        return Overlay(
            fill_r=int(out.fill_r), fill_g=int(out.fill_g),
            fill_b=int(out.fill_b), fill_a=int(out.fill_a),
            output_width=int(out.output_width),
            output_height=int(out.output_height),
            images=tuple(images),
        )

    # gain-map / tonemap -- gain-map / Tonemap derived items

    def gainmap(self, image_id: int) -> "GainMap":
        """Decode a `'tmap'` (Apple HDR gain-map) derived item.

        Returns the GainMapData header + per-channel records + the
        list of referenced images (typically `(base_sdr, gain_map)`).
        """
        out = ffi.new("heif_gainmap_t*")
        out.struct_size = ffi.sizeof("heif_gainmap_t")
        _check(lib.heif_reader_get_tonemap_gainmap(self._handle, image_id, out),
               f"get_tonemap_gainmap(image_id={image_id})")

        chcount = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_tonemap_gainmap_channel_count(
                   self._handle, image_id, chcount),
               f"get_tonemap_gainmap_channel_count(image_id={image_id})")
        ch = ffi.new("heif_gainmap_channel_t*")
        channels = []
        for i in range(int(chcount[0])):
            ch.struct_size = ffi.sizeof("heif_gainmap_channel_t")
            _check(lib.heif_reader_get_tonemap_gainmap_channel_at(
                       self._handle, image_id, i, ch),
                   f"get_tonemap_gainmap_channel_at(image_id={image_id}, "
                   f"index={i})")
            channels.append(GainMapChannel(
                gain_map_min_numerator=int(ch.gain_map_min_numerator),
                gain_map_min_denominator=int(ch.gain_map_min_denominator),
                gain_map_max_numerator=int(ch.gain_map_max_numerator),
                gain_map_max_denominator=int(ch.gain_map_max_denominator),
                gamma_numerator=int(ch.gamma_numerator),
                gamma_denominator=int(ch.gamma_denominator),
                base_offset_numerator=int(ch.base_offset_numerator),
                base_offset_denominator=int(ch.base_offset_denominator),
                alternate_offset_numerator=int(ch.alternate_offset_numerator),
                alternate_offset_denominator=int(ch.alternate_offset_denominator),
            ))

        imcount = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_tonemap_image_count(self._handle, image_id, imcount),
               f"get_tonemap_image_count(image_id={image_id})")
        ids = ffi.new("uint32_t*")
        imgs = []
        for i in range(int(imcount[0])):
            _check(lib.heif_reader_get_tonemap_image_at(self._handle, image_id, i, ids),
                   f"get_tonemap_image_at(image_id={image_id}, index={i})")
            imgs.append(int(ids[0]))

        return GainMap(
            minimum_version=int(out.minimum_version),
            writer_version=int(out.writer_version),
            is_multichannel=bool(out.is_multichannel),
            use_base_colour_space=bool(out.use_base_colour_space),
            channel_count=int(out.channel_count),
            base_hdr_headroom_numerator=int(out.base_hdr_headroom_numerator),
            base_hdr_headroom_denominator=int(out.base_hdr_headroom_denominator),
            alternate_hdr_headroom_numerator=int(out.alternate_hdr_headroom_numerator),
            alternate_hdr_headroom_denominator=int(out.alternate_hdr_headroom_denominator),
            channels=tuple(channels),
            images=tuple(imgs),
        )

    # entity groups -- entity groups

    @property
    def entity_groups(self) -> Tuple["EntityGroup", ...]:
        """All entity groups declared in `'grpl'`, in file order.

        Common group types: ``"altr"`` (alternates),
        ``"ster"`` (stereo pair), ``"eqiv"`` (equivalents).
        """
        count = ffi.new("uint32_t*")
        _check(lib.heif_reader_get_entity_group_count(self._handle, count),
               "get_entity_group_count")
        groups = []
        g = ffi.new("heif_entity_group_t*")
        for i in range(int(count[0])):
            g.struct_size = ffi.sizeof("heif_entity_group_t")
            _check(lib.heif_reader_get_entity_group_at(self._handle, i, g),
                   f"get_entity_group_at(index={i})")
            type_str = ffi.string(g.type, 4).decode(
                "ascii", errors="backslashreplace")

            ecount = ffi.new("uint32_t*")
            _check(lib.heif_reader_get_entity_group_entity_count(
                       self._handle, i, ecount),
                   f"get_entity_group_entity_count(index={i})")
            eid = ffi.new("uint32_t*")
            entities = []
            for j in range(int(ecount[0])):
                _check(lib.heif_reader_get_entity_group_entity_at(
                           self._handle, i, j, eid),
                       f"get_entity_group_entity_at(index={i}, "
                       f"entity_index={j})")
                entities.append(int(eid[0]))
            groups.append(EntityGroup(
                group_id=int(g.group_id),
                type=type_str,
                entities=tuple(entities),
            ))
        return tuple(groups)

    # segmented playback (DASH-style streaming)
    #
    # The four methods below mirror `Reader::parseInitializationSegment`,
    # `parseSegment`, `invalidateSegment`, `parseSegmentIndex` from the
    # C++ surface.  Lifetime contract: each `_SegmentStream` wrapper is
    # pinned for at least the window the C library needs to call back
    # into Python (init stream until reader close; media stream until
    # invalidate_segment; sidx stream only for the parse_segment_index
    # call duration).  After that window closes the strong ref is
    # dropped and the cffi callbacks become eligible for collection.

    def parse_initialization_segment(self, src) -> None:
        """Parse the initialization segment from *src*.

        *src* MUST be a binary file-like object supporting
        ``read(n) -> bytes``, ``seek(offset)``, ``tell() -> int``
        (and ``seek(0, 2)`` for end-positioning so the binding can
        report the stream's total size to the library).

        The library retains the underlying stream for the entire
        reader lifetime; the Reader holds the matching
        :class:`_SegmentStream` strong-ref until :meth:`close`.
        Calling this method twice replaces the previous init
        stream (the old one is destroyed once :meth:`close` runs).
        """
        stream = _SegmentStream(src)
        try:
            _check(lib.heif_reader_parse_initialization_segment(
                       self._handle, stream.handle),
                   "parse_initialization_segment")
        except Exception:
            stream.close()
            raise
        # Drop any previous init stream only after the new one is
        # successfully installed; the C side has already replaced
        # its internal reference.
        old = self._init_stream
        self._init_stream = stream
        if old is not None:
            old.close()

    def parse_segment(self, segment_id: int, src,
                      earliest_pts_in_ts: Optional[int] = None) -> None:
        """Parse a media segment from *src* under *segment_id*.

        *segment_id* MUST be globally unique across this reader and
        MUST differ from any init segment ID.  Pass
        *earliest_pts_in_ts* when feeding a partial segment without
        an `sidx` box; ``None`` (the default) maps to the C
        sentinel ``UINT64_MAX`` meaning "unset".

        The library retains the underlying stream until the
        matching :meth:`invalidate_segment(segment_id)` call; the
        Reader holds the :class:`_SegmentStream` in
        ``self._segment_streams[segment_id]`` for the same window.
        """
        if segment_id in self._segment_streams:
            raise ValueError(f"segment_id {segment_id} already parsed; "
                             "call invalidate_segment(...) first")
        pts = (1 << 64) - 1 if earliest_pts_in_ts is None else int(earliest_pts_in_ts)
        stream = _SegmentStream(src)
        try:
            _check(lib.heif_reader_parse_segment(
                       self._handle, stream.handle,
                       int(segment_id), pts),
                   f"parse_segment(segment_id={segment_id})")
        except Exception:
            stream.close()
            raise
        self._segment_streams[int(segment_id)] = stream

    def invalidate_segment(self, segment_id: int) -> None:
        """Invalidate a previously-parsed media segment.

        After this returns the matching :class:`_SegmentStream` is
        destroyed and the underlying ``BinaryIO`` is no longer held
        by this Reader.  Raises :class:`KeyError` if *segment_id*
        was not previously parsed via :meth:`parse_segment`.
        """
        sid = int(segment_id)
        if sid not in self._segment_streams:
            raise KeyError(f"segment_id {sid} not parsed")
        _check(lib.heif_reader_invalidate_segment(self._handle, sid),
               f"invalidate_segment(segment_id={sid})")
        # Only drop the strong ref after the C side has confirmed
        # the stream is no longer referenced.
        stream = self._segment_streams.pop(sid)
        stream.close()

    def parse_segment_index(self, src) -> Tuple[dict, ...]:
        """Parse a DASH On-Demand `sidx` box from *src*.

        Returns a tuple of dicts, one per segment, each carrying
        the `heif_segment_info_t` fields:
        ``segment_id``, ``earliest_pts_in_ts``, ``start_data_offset``,
        ``reference_id``, ``timescale``, ``duration_in_ts``,
        ``data_size``, ``reference_type``, ``starts_with_sap``,
        ``sap_type``.

        The stream is used only for the duration of this call; the
        binding releases its :class:`_SegmentStream` wrapper before
        returning so *src* may be closed by the caller immediately.
        """
        stream = _SegmentStream(src)
        try:
            count = ffi.new("uint32_t*")
            count[0] = 0
            _check(lib.heif_reader_parse_segment_index(
                       self._handle, stream.handle, ffi.NULL, count),
                   "parse_segment_index (probe)")
            n = int(count[0])
            if n == 0:
                return ()
            arr = ffi.new("heif_segment_info_t[]", n)
            sz  = ffi.sizeof("heif_segment_info_t")
            for i in range(n):
                arr[i].struct_size = sz
            _check(lib.heif_reader_parse_segment_index(
                       self._handle, stream.handle, arr, count),
                   "parse_segment_index (fetch)")
            out = []
            for i in range(int(count[0])):
                rec = arr[i]
                out.append({
                    "segment_id":         int(rec.segment_id),
                    "earliest_pts_in_ts": int(rec.earliest_pts_in_ts),
                    "start_data_offset":  int(rec.start_data_offset),
                    "reference_id":       int(rec.reference_id),
                    "timescale":          int(rec.timescale),
                    "duration_in_ts":     int(rec.duration_in_ts),
                    "data_size":          int(rec.data_size),
                    "reference_type":     int(rec.reference_type),
                    "starts_with_sap":    int(rec.starts_with_sap),
                    "sap_type":           int(rec.sap_type),
                })
            return tuple(out)
        finally:
            stream.close()

    # lifecycle

    def close(self) -> None:
        """Release the underlying C handle.  Idempotent; safe to call
        multiple times and from ``__del__``."""
        # ``getattr`` guards the case where __init__ raised before
        # _handle was assigned (defensive but cheap).
        h = getattr(self, "_handle", None)
        if h is not None and h != ffi.NULL:
            # Tear down in the order required by the C lifetime
            # contract: destroy the reader BEFORE dropping the
            # segment streams it still references, so the library
            # never observes a freed StreamInterface.
            lib.heif_reader_destroy(h)
            self._handle = ffi.NULL
        # Drop segment-stream strong refs after the reader is gone.
        # _SegmentStream.close() then runs heif_stream_destroy on
        # each cffi-callback-pinned wrapper.
        seg = getattr(self, "_segment_streams", None)
        if seg:
            for stream in list(seg.values()):
                stream.close()
            seg.clear()
        init = getattr(self, "_init_stream", None)
        if init is not None:
            init.close()
            self._init_stream = None

    def __enter__(self) -> "Reader":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def __del__(self) -> None:
        # Best-effort fallback; embedders SHOULD use a `with` block
        # for deterministic cleanup.  Suppress all exceptions because
        # __del__ is allowed to be called with the interpreter in any
        # state including a partially-torn-down library handle.
        try:
            self.close()
        except Exception:
            pass

    def __repr__(self) -> str:
        return f"<heif.Reader path={self._path!r}>"
