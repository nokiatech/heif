"""
Nokia HEIF reader binding for Python -- pure-Python ``cffi``-based wrapper.

This package is a thin Python wrapper over the pure-C reader facade exposed by ``libheif_shared`` (`srcs/api/reader/heifreader_c.h`)
plus the error-inspection slot
(`srcs/api/common/heif_error_c.h`).

Capabilities
------------------
* Open a HEIF/HEIC file.
* Enumerate its master image items.
* For each image, read pixel dimensions, item type, and pull a
  decoder-ready encoded bitstream as a ``bytes`` object.

Not yet supported
-----------------------------
* Decode pixels -- the returned ``bytes`` is an encoded HEVC / AVC /
  VVC / JPEG payload; feed it to whichever decoder the embedder
  prefers (``pillow_heif``, ``imageio``, ``av`` / PyAV, ``vvdec``...).
* Write HEIF -- not yet implemented.
* Read Exif / XMP / item properties -- not yet implemented.
* Read image sequences / video tracks -- not yet implemented.

Usage
-----
::

    import heif

    with heif.Reader("photo.heic") as r:
        print(r.major_brand)              # "mif3"
        for img in r.master_images:
            print(img.item_type, img.width, img.height)
            payload = img.bitstream()     # bytes
            with open(f"item-{img.image_id}.bin", "wb") as f:
                f.write(payload)

Why ``cffi`` ABI mode (and not ``cffi.set_source`` API mode)
------------------------------------------------------------
ABI mode means we do NOT compile any C glue at install time -- we
just declare the shape of the C functions in a ``cdef()`` string
and ``cffi`` calls them by symbol lookup against the installed
``libheif_shared.so``.  This means a Python user does NOT need a
C compiler installed; ``pip install`` is pure-Python.  The
trade-off is slightly higher per-call overhead (negligible for typical use) and that the C declarations have to be kept in sync
with the installed headers by hand.

Loading the shared library
--------------------------
``Reader.__init__`` resolves the library by, in order:

    1. The ``HEIF_LIBRARY_PATH`` environment variable (full path to
       the .so / .dll / .dylib).  Set this when running against a
       custom build directory.
    2. The platform's default search path (``ld.so`` / ``LoadLibrary`` /
       ``dyld``) for ``libheif_shared.so.4`` (Linux), ``heif_shared.dll``
       (Windows), ``libheif_shared.4.dylib`` (macOS).

Errors
------
Every C entry point returns ``HEIF::ErrorCode`` as ``int32_t``.  This
binding translates non-zero returns into a :class:`HeifError`
exception, populated from the thread-local error slot
(``heif_get_last_error_*``).  Embedders never see raw error codes.
"""

from ._reader import (
    AmbientViewingEnvironment,
    CleanAperture,
    ColorProfile,
    ContentColorVolume,
    ContentLightLevel,
    EditUnit,
    EntityGroup,
    GainMap,
    GainMapChannel,
    Grid,
    HeifError,
    Image,
    ItemProperty,
    MasteringDisplayColorVolume,
    NominalDiffuseWhite,
    OinfLayer,
    OinfOp,
    OinfOpLayer,
    OinfPtl,
    Overlay,
    OverlayImage,
    PixiChannel,
    Reader,
    ReferenceViewingEnvironment,
    Sample,
    SampleEqiv,
    SampleInfo,
    SampleMinp,
    SampleRefs,
    SampleSpor,
    SampleStmi,
    SampleSulm,
    SampleTrif,
    SegmentInfo,
    SubsEntry,
    Track,
    TrackGroup,
    TrackInfo,
    TrackReference,
)

__all__ = (
    "AmbientViewingEnvironment",
    "CleanAperture",
    "ColorProfile",
    "ContentColorVolume",
    "ContentLightLevel",
    "EditUnit",
    "EntityGroup",
    "GainMap",
    "GainMapChannel",
    "Grid",
    "HeifError",
    "Image",
    "ItemProperty",
    "MasteringDisplayColorVolume",
    "NominalDiffuseWhite",
    "OinfLayer",
    "OinfOp",
    "OinfOpLayer",
    "OinfPtl",
    "Overlay",
    "OverlayImage",
    "PixiChannel",
    "Reader",
    "ReferenceViewingEnvironment",
    "Sample",
    "SampleEqiv",
    "SampleInfo",
    "SampleMinp",
    "SampleRefs",
    "SampleSpor",
    "SampleStmi",
    "SampleSulm",
    "SampleTrif",
    "SegmentInfo",
    "SubsEntry",
    "Track",
    "TrackGroup",
    "TrackInfo",
    "TrackReference",
)
__version__ = "1.0.0"
