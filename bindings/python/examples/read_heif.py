#!/usr/bin/env python3
"""
Example: reading a HEIF / HEIC file with the Nokia HEIF Python binding.

Usage:
    python read_heif.py <file.heic>

This script walks through the main Reader and Image APIs with
explanatory comments.  It is meant as a reference for binding
users -- no assertions, just prints.

Prerequisites:
    pip install nokia-heif          # from sdist / PyPI
    # or: pip install -e bindings/python   (in-tree dev install)
"""

from __future__ import annotations

import os
import sys

import heif


def main() -> None:
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <file.heic>", file=sys.stderr)
        sys.exit(1)

    path = sys.argv[1]
    if not os.path.isfile(path):
        print(f"error: file not found: {path}", file=sys.stderr)
        sys.exit(1)

    # -- 1. Open the file -----------------------------------------
    #
    # Reader is a context manager.  The underlying C handle is
    # allocated on entry and released on exit.  One Reader per file;
    # open a new Reader for each file you want to inspect.
    with heif.Reader(path) as reader:
        print(f"Opened: {path}")
        print(f"  Reader repr : {reader!r}")

        # -- 2. File-level metadata -------------------------------
        #
        # major_brand is the FourCC from the 'ftyp' box, e.g.
        # "heic", "mif1", "mif3", "msf1".
        print(f"  Major brand : {reader.major_brand}")

        # Compatible brand combos come from ftyp / styp boxes.
        # Each combo is a tuple of FourCC strings.
        for i, combo in enumerate(reader.compatible_brand_combos):
            print(f"  Brand combo {i}: {', '.join(combo)}")

        # File-level feature bitmask.
        # Bit 0x01 = single image, 0x02 = image collection,
        # 0x04 = image sequence, 0x08 = root meta box,
        # 0x10 = alternate tracks.
        ff = reader.file_features
        labels = []
        if ff & 0x01: labels.append("SingleImage")
        if ff & 0x02: labels.append("ImageCollection")
        if ff & 0x04: labels.append("ImageSequence")
        if ff & 0x08: labels.append("RootMetaBox")
        if ff & 0x10: labels.append("AlternateTracks")
        print(f"  File features: 0x{ff:02x} ({', '.join(labels) or 'none'})")

        # -- 3. Master images -------------------------------------
        #
        # master_images returns a cached tuple of Image objects --
        # the primary displayable items in the file.  For a
        # single-photo .heic this is usually one item.
        images = reader.master_images
        print(f"\n  Master images: {len(images)}")

        for img in images:
            print(f"\n  -- Image id={img.image_id} --")
            print(f"     Repr      : {img!r}")

            # item_type is the FourCC codec tag: "hvc1" (HEVC),
            # "vvc1" (VVC), "avc1" (AVC/H.264), "jpeg", "av01",
            # or a derived-item type like "grid", "iovl", "iden",
            # "tmap".
            print(f"     Item type : {img.item_type}")
            print(f"     Dimensions: {img.width} x {img.height}")

            # -- 4. Image properties ------------------------------
            #
            # properties returns a tuple of ItemProperty named-
            # tuples listing every property box associated with
            # the image (ispe, pixi, colr, irot, imir, clap, ...).
            props = img.properties
            print(f"     Properties: {len(props)} total")
            for p in props:
                print(f"       id={p.property_id} fourcc={p.fourcc!r} "
                      f"essential={p.essential}")

            # Convenience accessors for common properties.
            # Each returns None when the property is absent.

            # Pixel information: bits per channel.
            pi = img.pixel_info
            if pi is not None:
                print(f"     Pixel info (bpc per channel): {pi}")

            # Detailed per-channel pixel info (pixi v2 with
            # labels, interleave type, etc.).
            pch = img.pixel_channels
            if pch is not None:
                for ch in pch:
                    print(f"     Pixel channel: {ch}")

            # Display rotation in degrees (0, 90, 180, 270).
            rot = img.rotation
            if rot:
                print(f"     Rotation  : {rot} degrees")

            # Mirror axis ("horizontal" or "vertical"), or None.
            mir = img.mirror
            if mir is not None:
                print(f"     Mirror    : {mir}")

            # Clean aperture (crop rectangle).
            clap = img.clean_aperture
            if clap is not None:
                print(f"     CleanAp   : {clap}")

            # Pixel aspect ratio (hSpacing, vSpacing).
            par = img.pixel_aspect_ratio
            if par is not None:
                print(f"     PAR       : {par[0]}:{par[1]}")

            # ICC / nclx colour profile.
            cp = img.color_profile
            if cp is not None:
                print(f"     Color prof: profile_type={cp.profile_type!r}, "
                      f"{len(cp.icc_bytes)} ICC bytes")

            # HDR metadata properties.
            cll = img.content_light_level
            if cll is not None:
                print(f"     CLL       : {cll}")
            mdcv = img.mastering_display
            if mdcv is not None:
                print(f"     MDCV      : {mdcv}")

            # -- 5. Item references -------------------------------
            #
            # referenced_to("thmb") -- thumbnails that point at this
            # image.  referenced_to("cdsc") -- Exif/XMP metadata
            # items that describe this image.
            thumbs = img.referenced_to("thmb")
            if thumbs:
                print(f"     Thumbnails: {[t.image_id for t in thumbs]}")

            meta_items = img.referenced_to("cdsc")
            if meta_items:
                print(f"     Metadata  : {[m.image_id for m in meta_items]}")

            aux = img.referenced_to("auxl")
            if aux:
                print(f"     Auxiliary  : {[a.image_id for a in aux]}")

            # For derived items, referenced_from("dimg") gives the
            # constituent images.
            if img.item_type in ("grid", "iovl", "iden", "tmap"):
                constituents = img.referenced_from("dimg")
                print(f"     Derived from: "
                      f"{[c.image_id for c in constituents]}")

            # -- 6. Derived-item details --------------------------
            #
            # If the image is a grid, we can inspect the layout.
            if img.item_type == "grid":
                g = reader.grid(img.image_id)
                print(f"     Grid      : {g.columns}x{g.rows}, "
                      f"output {g.output_width}x{g.output_height}, "
                      f"tiles {g.images}")

            # If the image is an overlay, we can inspect offsets.
            if img.item_type == "iovl":
                ov = reader.overlay(img.image_id)
                print(f"     Overlay   : {ov.output_width}x"
                      f"{ov.output_height}, "
                      f"{len(ov.images)} layers")

            # -- 7. Encoded bitstream -----------------------------
            #
            # bitstream() returns the decoder-ready payload: codec
            # parameter sets prepended, NAL start codes substituted.
            # Feed it directly to a codec decoder.
            bs = img.bitstream()
            print(f"     Bitstream : {len(bs)} bytes, "
                  f"head={bs[:8].hex() if len(bs) >= 8 else bs.hex()}")

            # raw_item_bytes() returns the raw item data without
            # decoder configuration -- useful for Exif / XMP / MIME
            # payloads on non-image items.

        # -- 8. Entity groups -------------------------------------
        #
        # Entity groups from the 'grpl' box: alternates ("altr"),
        # stereo pairs ("ster"), equivalences ("eqiv"), etc.
        groups = reader.entity_groups
        if groups:
            print(f"\n  Entity groups: {len(groups)}")
            for eg in groups:
                print(f"    group_id={eg.group_id} type={eg.type!r} "
                      f"entities={eg.entities}")

        # -- 9. Tracks (image sequences) --------------------------
        #
        # Tracks are present in files with brand "msf1" or the
        # ImageSequence feature flag.  Each track contains samples
        # (video frames or burst photos).
        tracks = reader.tracks
        if tracks:
            print(f"\n  Tracks: {len(tracks)}")
            for trk in tracks:
                ti = trk.info
                print(f"\n  -- Track id={trk.track_id} --")
                print(f"     Display size: {trk.display_width}x"
                      f"{trk.display_height}")
                print(f"     Duration    : {trk.playback_duration_secs:.3f}s")
                print(f"     Timescale   : {ti.time_scale}")
                print(f"     Frame rate  : {ti.frame_rate_num}/"
                      f"{ti.frame_rate_den}")
                print(f"     Brand       : {ti.major_brand}")

                # List samples (frames).
                samples = trk.samples
                print(f"     Samples     : {len(samples)}")
                for si in samples[:5]:  # show first 5
                    print(f"       sample_id={si.sample_id} "
                          f"type={si.sample_entry_type!r} "
                          f"size={si.size} "
                          f"pts={si.earliest_timestamp_ts}")
                if len(samples) > 5:
                    print(f"       ... and {len(samples) - 5} more")

                # Fetch a bitstream for the first sample.
                if samples:
                    s = trk.sample(samples[0].sample_id)
                    bs = s.bitstream()
                    print(f"     First sample bitstream: {len(bs)} bytes")

        # -- 10. Error handling -----------------------------------
        #
        # All C-level errors surface as heif.HeifError, a subclass
        # of RuntimeError.  It carries structured fields:
        #   err.code, err.code_name, err.origin, err.origin_name,
        #   err.file, err.line, err.message, err.cause
        print("\n  -- Error handling demo --")
        try:
            # Requesting a non-existent image ID triggers an error.
            bad_img = heif.Image(reader, 999999)
            _ = bad_img.width
        except heif.HeifError as err:
            print(f"     Caught HeifError: {err.code_name}")
            print(f"     Message : {err.message}")

    # After the `with` block, reader.close() has been called
    # automatically.  The C handle is released and the reader is
    # no longer usable.
    print("\nDone -- reader closed.")


if __name__ == "__main__":
    main()
