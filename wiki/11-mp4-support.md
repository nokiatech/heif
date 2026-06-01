# 11 - MP4 / ISOBMFF Support

This library is a **HEIF reader/writer** that shares ISOBMFF box
machinery with MP4. It is not a general-purpose MP4 mux/demux library.
This page documents what MP4 functionality is available, what is not,
and common questions.

---

## At a glance

**Can do**

* Read plain `.mp4` / `.m4a` containing H.264 / H.265 / H.266 video
  and/or AAC audio.
* Read fragmented MP4 / DASH / CMAF segments (init + media segments).
* Read HEIF / HEIC files (the library's primary job).
* Write plain `.mp4` with one video track (H.264 / H.265 / H.266).
* Write plain `.m4a` with one audio track (AAC-LC).
* Write video + audio in the same `.mp4`.

**Cannot do**

* Write fragmented MP4 / DASH / CMAF segments.
* Encrypt or decrypt (no DRM / Common Encryption).
* Write audio other than AAC-LC (no Opus, FLAC, AC-3, MPEG-H).
* Write video other than AVC / HEVC / VVC (no AV1, VP9).
* Read or write subtitle, hint, or timed-metadata tracks.
* Validate that `OutputConfig::compatibleBrands` matches the
  produced structure.

---

## Capability matrix

| Capability | Read | Write |
|---|---|---|
| Plain progressive MP4 | ✅ | ✅ |
| `moov`-before-`mdat` (`progressiveFile=true`) | ✅ | ✅ |
| `moov`-after-`mdat` (default) | ✅ | ✅ |
| Fragmented MP4 (`moof`/`mdat`, `mvex`/`trex`) | ✅ | ❌ |
| Multi-segment (DASH/CMAF init+media) | ✅ | ❌ |
| Edit list (`elst`) | ✅ | ✅ |
| Track references (`tref`) | ✅ | ✅ |
| Track groups (`trgr`) | ✅ | ✅ |
| Alternate groups | ✅ | ✅ |
| Common Encryption (`sinf`/`tenc`/`pssh`/…) | ❌ | ❌ |
| User data (`udta`, `cprt`) | ❌ | ❌ |
| `mfra` random-access tables | ❌ | ❌ |

---

## Handler types

The reader accepts **four** handler types:

| FourCC | Meaning | Status |
|---|---|---|
| `pict` | HEIF image sequence | Read + write |
| `auxv` | Auxiliary video | Read + write |
| `vide` | Video | Read + write |
| `soun` | Audio | Read + write |

**Tracks with any other handler type are silently dropped.** This
includes subtitles (`subt`/`sbtl`/`text`), timed metadata (`meta`),
hint tracks (`hint`), and closed captions (`clcp`).

---

## Sample entry types

| FourCC | Codec | Read | Write |
|---|---|---|---|
| `hvc1`, `hev1` | HEVC / H.265 | ✅ | ✅ |
| `avc1`, `avc3` | AVC / H.264 | ✅ | ✅ |
| `vvc1` | VVC / H.266 | ✅ | ✅ |
| `vvs1` | VVC subpicture | ✅ | ✅ |
| `mp4a` | AAC (MPEG-4 Audio, OTI 0x40) | ✅ | ✅ |

Unknown sample entries are logged and skipped. The track stays in
the catalog but data access on its samples will fail.

**Not supported:** AV1 (`av01`), VP8/VP9 (`vp08`/`vp09`),
Dolby Vision (`dvh1`/`dvhe`), Opus, FLAC, AC-3, MPEG-H 3D Audio,
raw PCM.

---

## Video tracks (`vide`)

`Writer::addVideoTrack` produces a compliant ISOBMFF video track
with `tkhd`, `mdhd`, `hdlr=vide`, `vmhd`, and a full `stbl`
(sample table with `stsd`, `stts`, `stsc`, `stsz`, `stco`/`co64`,
optional `ctts`, `cslg`, `stss`).

Track references (`tref`) and edit lists (`edts`/`elst`) are
supported via the corresponding API calls.

### Sample groupings

Supported: `refs`, `eqiv`, `stmi`, `trif`, `spor`, `sulm`, `minp`.

Standard MP4 groupings (`rap `, `roll`, `prol`, etc.) round-trip
as opaque entries but are not surfaced via the public API.

---

## Audio tracks (`soun`)

### Codec: AAC-LC only

The writer requires a `DecoderSpecInfoType::AudioSpecificConfig`
blob and hard-codes MPEG-4 Audio (OTI 0x40). HE-AAC and xHE-AAC
bitstreams will serialise correctly (the distinction is inside
AudioSpecificConfig, not in the OTI), but player compatibility
for those profiles is your responsibility.

### What the writer produces

* `tkhd` (volume = 1.0, dimensions = 0×0)
* `mdhd` + `hdlr=soun` + `smhd`
* `stsd` with one `mp4a` entry containing an `esds`
  (`ES_Descriptor` → `DecoderConfigDescriptor` →
  `DecoderSpecificInfo`)

### `AudioParams`

```cpp
struct AudioParams {
    uint16_t channelCount;
    uint32_t sampleRate;
    uint32_t averageBitrate;  // 0 = unknown
    uint32_t maxBitrate;      // 0 = unknown
};
```

The HEIFPP wrapper computes bitrates empirically from sample sizes.

---

## Brand handling (`ftyp`)

The major brand is **not** auto-selected. You must call
`Writer::setMajorBrand(...)` before `finalize()`, otherwise you
get `BRANDS_NOT_SET`.

Common brand sets:

| File type | `majorBrand` | `compatibleBrands` |
|---|---|---|
| Plain MP4 video | `mp42` | `mp42`, `isom`, `hevc` |
| Plain `.m4a` audio | `M4A ` | `mp42`, `isom` |
| HEIF still | `mif1` | `mif1`, `heic`, `hevc` |

The writer auto-adds `msf1` and `iso8` for image-sequence tracks
when `progressiveFile=false`. These signal HEIF Media Segment
Format compliance.

**No cross-validation.** The writer trusts your brand list - only
declare brands whose contract the produced file actually fulfils.

---

## Fragmented MP4 reader

The reader parses `styp`, `sidx`, `moof`/`traf`/`tfhd`/`trun`/`tfdt`,
and `mvex`/`trex`/`mehd` from initialization and media segments.

Use `Reader::parseInitializationSegment` for the init segment and
`Reader::parseSegment` for each media segment.

**Not supported on read:** `mfra`/`tfra` random-access, `prft`,
`emsg`, CENC encryption metadata (`senc`/`saio`/`saiz`).

**No fragmented writer.** To produce DASH/CMAF segments, write a
plain progressive file and post-process with a fragmenting tool
(`MP4Box -dash`, `ffmpeg -movflags frag_keyframe`, Shaka Packager).

---

## FAQ

**Q. I have an `.mp4` from ffmpeg / OBS / a phone camera. Will the
reader open it?**

* If the only tracks are H.264/H.265/H.266 video and/or AAC audio,
  with no DRM: yes.
* Subtitle / hint / timed-metadata tracks are silently dropped.
  Other tracks still work.
* DRM-protected items return `PROTECTED_ITEM` on data access.
* Unsupported video codecs (AV1, VP9, etc.): the track appears in
  `getTrackInformations()` but data access fails.

**Q. Can I write video + audio in the same file?**

Yes. Call `addVideoTrack(...)` and `addAudioTrack(...)` before adding
samples, then interleave `addVideo` / `addAudio` calls. Set
`majorBrand = "mp42"`.

**Q. Can I produce DASH/CMAF segments?**

No. Write a plain MP4 and fragment it with an external tool.

**Q. Can I write Opus / FLAC / AC-3?**

No. AAC-LC is the only writable audio codec.

**Q. Can I read a DASH segment from a CDN?**

Yes, if it's unencrypted and uses a supported codec. Use
`parseInitializationSegment` + `parseSegment`.

**Q. The brands `msf1` / `iso8` appeared in my file but I didn't
add them - why?**

Auto-added for image-sequence tracks when `progressiveFile=false`.
They signal HEIF Media Segment Format compliance. If they cause
problems downstream, post-process `ftyp` to strip them.

**Q. VVC support - is it VVC-in-HEIF, VVC-in-MP4, or both?**

Both. The `vvc1` and `vvs1` sample entries work identically in
`pict`, `auxv`, and `vide` tracks. VVC-specific sample groupings
(`spor`, `sulm`, `minp`, `trif`) are supported for either container.

---

## See also

* [00 - Architecture Overview](00-architecture-overview.md)
* [05 - CMake Options & Targets](05-cmake-options-and-targets.md)
* [10 - ABI Policy](10-abi-policy.md)
