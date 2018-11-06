/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef WRITERAPIDATATYPES_HPP
#define WRITERAPIDATATYPES_HPP

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include "heifcommondatatypes.h"
#include "heifexport.h"
#include "heifid.h"
#include "OutputStreamInterface.h"

namespace HEIF
{
    IdType(std::uint32_t, GroupId);
    IdType(std::uint32_t, MediaDataId);

    struct HEIF_DLL_PUBLIC OutputConfig
    {
        /**
         * Output filename */
        const char* fileName;

        /**
          * Output stream interface 
          * If set all writes will be directed here*/
        OutputStreamInterface* outputStream = nullptr;

        /**
         * If true: then all file data is kept in memory until finalize() is called.
         * Order of boxes ('ftyp', 'meta' and possible 'moov' boxes are before MediaDataBox ('mdat'),
         * which make progressive download and partial parsing of file possible.
         *
         * If false: then 'ftyp' is written in initialize(), e.g. brands written to file are final.
         * MediaDataBox ('mdat') content is written as it is fed using feedMediaData().
         * Both 'meta' and possible 'moov' box are written at the end of the file after MediaDataBox.
         * When parsing generated file whole file needs to be available for parsing to be possible. */
        bool progressiveFile = true;

        /**
         * Brand four character code information stored to 'ftyp' box at the start of the file indicating content of the
         * file. If progressiveFile = false, then this information needs to be available when initialize() is called. If
         * progressiveFile = true, then further brands can be added between initialize() and finalize() with calls to
         *                            setMajorBrand() and addCompatibleBrand(). */
        FourCC majorBrand;
        Array<FourCC> compatibleBrands;
    };

    enum class MediaFormat
    {
        INVALID = 0,  ///< Undefined MediaForamt, invalid value used internal error checking.

        AVC,   ///< Image(s) encoded with AVC/H.264 encoder (Annex E in HEIF ISO/IEC 23008-12:2017(E) spec for details).
        HEVC,  ///< Image(s) encoded with HEVC/H.265 encoder (Annex B in HEIF ISO/IEC 23008-12:2017(E) spec for
               ///< details).
        JPEG,  ///< Image(s) encoded with JPEG encoder (Annex H in HEIF ISO/IEC 23008-12:2017(E) spec for details).
        EXIF,  ///< EXIF Metadata binary data (ExifDataBlock() as defined in Annex A.2.1 in HEIF ISO/IEC
               ///< 23008-12:2017(E)).
        XMP,   ///< XMP Metadata binary data (Annex A.3 in HEIF ISO/IEC 23008-12:2017(E) spec for details).
        MPEG7,  ///< MPEG-7 Metadata binary data (Annex A.4 in HEIF ISO/IEC 23008-12:2017(E) spec for details).
        AAC     ///< AAC-LC audio track data.
    };

    struct HEIF_DLL_PUBLIC Data
    {
        MediaFormat mediaFormat = MediaFormat::INVALID;

        uint8_t* data = nullptr;
        uint64_t size = 0;

        DecoderConfigId decoderConfigId =
            0;  // required for MediaFormat values: AVC, HEVC, JPEG and AAC. Not needed for EXIF,XMP or MPEG7 metadata.
    };

    struct HEIF_DLL_PUBLIC SampleInfo
    {
        uint64_t duration;          ///< duration of sample in ImageSequence timeBase units.
        int64_t compositionOffset;  ///< compositionOffset of sample in ImageSequence timeBase units.
        bool isSyncSample;          ///< whether sample is sync sample

        /** A sample that is predicted from other samples requires all its reference samples to be decoded prior to its
         * decoding. Direct reference samples list 'refs' is a sample group that identifies all the direct reference
         * samples for a sample. **/
        Array<SequenceImageId> referenceSamples;  ///< list of samples that this sample has direct decode dependency on.
    };

    struct HEIF_DLL_PUBLIC EquivalenceTimeOffset
    {
        /**
         * Specifies the difference of the time related to the image item(s) in the associated 'eqiv' entity group
         * and the composition time of the sample associated with the sample group description entry, as specified
         * in HEIF spec ("6.8 Relating an untimed item to a timed sequence"). */
        std::int16_t timeOffset;

        /**
         * Specifies the timescale, as a multiplier to the media timescale of the track. */
        std::uint16_t timescaleMultiplier;  //  8.8 fixed-point value, Recommended value: (1.0 presented as 1 << 8)
    };

    struct AudioParams
    {
        std::uint16_t channelCount;
        std::uint32_t sampleRate;
        std::uint32_t averageBitrate;
        std::uint32_t maxBitrate;
    };
}  // namespace HEIF


#endif  // WRITERAPIDATATYPES_HPP
