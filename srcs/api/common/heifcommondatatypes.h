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

#ifndef HEIFCOMMONDATATYPES_H
#define HEIFCOMMONDATATYPES_H

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include "heifexport.h"
#include "heifid.h"


#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

namespace HEIF
{
    /** @brief HEIF file reader and writer ErrorCode enumeration. */
    enum class ErrorCode
    {
        OK = 0,
        ALLOCATOR_ALREADY_SET,
        ALREADY_INITIALIZED,
        BRANDS_NOT_SET,
        BUFFER_SIZE_TOO_SMALL,
        DECODER_CONFIGURATION_ERROR,
        FILE_HEADER_ERROR,
        FILE_OPEN_ERROR,
        FILE_READ_ERROR,
        FTYP_ALREADY_WRITTEN,
        HIDDEN_PRIMARY_ITEM,
        INVALID_CONTEXT_ID,
        INVALID_FUNCTION_PARAMETER,
        INVALID_GROUP_ID,
        INVALID_ITEM_ID,
        INVALID_MEDIADATA_ID,
        INVALID_METADATAITEM_ID,
        INVALID_MEDIA_FORMAT,
        INVALID_PROPERTY_INDEX,
        INVALID_REFERENCE_COUNT,
        INVALID_SAMPLE_DESCRIPTION_INDEX,
        INVALID_SEQUENCE_ID,
        INVALID_DECODER_CONFIG_ID,
        INVALID_SEQUENCE_IMAGE_ID,
        MEDIA_PARSING_ERROR,
        NOT_APPLICABLE,
        PRIMARY_ITEM_NOT_SET,
        PROTECTED_ITEM,
        UNINITIALIZED,
        UNPROTECTED_ITEM,
        UNSUPPORTED_CODE_TYPE
    };

    struct HEIF_DLL_PUBLIC FourCC
    {
        char value[5];
        inline FourCC()
            : value{}
        {
        }
        inline FourCC(uint32_t v)
        {
            value[0] = char((v >> 24) & 0xff);
            value[1] = char((v >> 16) & 0xff);
            value[2] = char((v >> 8) & 0xff);
            value[3] = char((v >> 0) & 0xff);
            value[4] = '\0';
        }
        inline FourCC(const char* str)
        {
            value[0] = str[0];
            value[1] = str[1];
            value[2] = str[2];
            value[3] = str[3];
            value[4] = '\0';
        }
        inline FourCC(const FourCC& fourcc)
        {
            value[0] = fourcc.value[0];
            value[1] = fourcc.value[1];
            value[2] = fourcc.value[2];
            value[3] = fourcc.value[3];
            value[4] = '\0';
        }
        inline FourCC& operator=(const FourCC& other)
        {
            value[0] = other.value[0];
            value[1] = other.value[1];
            value[2] = other.value[2];
            value[3] = other.value[3];
            value[4] = '\0';
            return *this;
        }
        inline bool operator==(const FourCC& other) const
        {
            return (value[0] == other.value[0]) && (value[1] == other.value[1]) && (value[2] == other.value[2]) &&
                   (value[3] == other.value[3]);
        }
        inline bool operator!=(const FourCC& other) const
        {
            return (value[0] != other.value[0]) || (value[1] != other.value[1]) || (value[2] != other.value[2]) ||
                   (value[3] != other.value[3]);
        }
        inline bool operator<(const FourCC& other) const
        {
            return (value[0] < other.value[0])
                       ? true
                       : (value[0] > other.value[0])
                             ? false
                             : (value[1] < other.value[1])
                                   ? true
                                   : (value[1] > other.value[1])
                                         ? false
                                         : (value[2] < other.value[2])
                                               ? true
                                               : (value[2] > other.value[2])
                                                     ? false
                                                     : (value[3] < other.value[3])
                                                           ? true
                                                           : (value[3] > other.value[3]) ? false : false;
        }
        inline bool operator<=(const FourCC& other) const
        {
            return *this == other || *this < other;
        }
        inline bool operator>=(const FourCC& other) const
        {
            return !(*this < other);
        }
        inline bool operator>(const FourCC& other) const
        {
            return !(*this <= other);
        }
    };

    template <typename T>
    struct HEIF_DLL_PUBLIC Array
    {
        typedef T value_type;
        typedef T* iterator;
        typedef const T* const_iterator;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        size_t size;
        T* elements;
        Array();
        Array(size_t n);
        Array(const Array& other);
        virtual Array& operator=(const Array& other);
        inline T& operator[](size_t index)
        {
            return elements[index];
        }
        inline const T& operator[](size_t index) const
        {
            return elements[index];
        }
        inline T* begin()
        {
            return elements;
        }
        inline T* end()
        {
            return elements + size;
        }
        inline const T* begin() const
        {
            return elements;
        }
        inline const T* end() const
        {
            return elements + size;
        }
        template <typename U>
        Array(U begin, U end);
        Array(std::initializer_list<T> aInit);
        virtual ~Array();
    };

    IdType(std::uint32_t, ImageId);
    IdType(std::uint32_t, SequenceImageId);
    IdType(std::uint32_t, SequenceId);
    IdType(std::uint32_t, DecoderConfigId);
    IdType(std::uint32_t, PropertyId);
    IdType(std::uint32_t, MetadataItemId);

    struct HEIF_DLL_PUBLIC Rational
    {
        uint64_t num;
        uint64_t den;
    };

    enum DecoderSpecInfoType
    {
        AVC_SPS = 7,  ///< H.264/AVC Sequence Parameter Set (SPS) nal unit, bytestream header (0001) if any is stripped
                      ///< internally.
        AVC_PPS = 8,  ///< H.264/AVC Picture Parameter Set (PPS) nal unit, bytestream header (0001) if any is stripped
                      ///< internally.

        HEVC_VPS = 32,  ///< H.265/HEVC Video Parameter Set (VPS) nal unit, bytestream header (0001) if any is stripped
                        ///< internally.
        HEVC_SPS = 33,  ///< H.265/HEVC Sequence Parameter Set (SPS) nal unit, bytestream header (0001) if any is
                        ///< stripped internally.
        HEVC_PPS = 34,  ///< H.265/HEVC Picture Parameter Set (PPS) nal unit, bytestream header (0001) if any is
                        ///< stripped internally.

        AudioSpecificConfig  ///< As defined in 1.6.2.1 AudioSpecificConfig of ISO/IEC 14496-3:200X(E)
    };

    struct HEIF_DLL_PUBLIC DecoderSpecificInfo
    {
        DecoderSpecInfoType decSpecInfoType;  // can be "SPS" and "PPS" for AVC, additional "VPS" for HEVC
        Array<uint8_t> decSpecInfoData;
    };

    // HEIF item Properties

    /// Data of transformative item property Image mirroring 'imir'.
    struct HEIF_DLL_PUBLIC Mirror
    {
        bool horizontalAxis;  ///< Mirror axis (\c true = horizontal, \c false = vertical)
    };

    /// Data of transformative item property Image rotation 'irot'.
    struct HEIF_DLL_PUBLIC Rotate
    {
        uint32_t angle;  ///< Angle of rotation in anti-clockwise direction. Valid values are 0, 90, 180 and 270.
    };

    /// Data of descriptive item property Relative location 'rloc'.
    struct HEIF_DLL_PUBLIC RelativeLocation
    {
        uint32_t horizontalOffset;  ///< Horizontal offset in pixels of the left-most pixel column
        uint32_t verticalOffset;    ///< Vertical offset in pixels of the top-most pixel row
    };

    /// Data of descriptive item property Pixel aspect ratio 'pasp'.
    struct HEIF_DLL_PUBLIC PixelAspectRatio
    {
        uint32_t relativeWidth;   ///< Defines aspect ratio of a pixel, together with relativeHeight.
        uint32_t relativeHeight;  ///< Defines aspect ratio of a pixel, together with relativeWidth.
    };

    /// Data of descriptive item property Pixel information 'pixi'.
    struct HEIF_DLL_PUBLIC PixelInformation
    {
        Array<uint8_t> bitsPerChannel;  ///< Bits per channel for the pixels of the image.
    };

    /// Data of descriptive item property Colour information 'colr'.
    struct HEIF_DLL_PUBLIC ColourInformation
    {
        FourCC colourType;
        // These are used with colour type "nclx".
        uint16_t colourPrimaries;
        uint16_t transferCharacteristics;
        uint16_t matrixCoefficients;
        bool fullRangeFlag;
        // ICC profile is used with colour types "rICC" and "prof".
        Array<uint8_t> iccProfile;
    };

    /// Data of transformative item property Clean aperture 'clap.'
    struct HEIF_DLL_PUBLIC CleanAperture
    {
        uint32_t widthN;             ///< Numerator of clean aperture width fractional value, in counted pixels
        uint32_t widthD;             ///< Denominator of clean aperture width fractional value, in counted pixels
        uint32_t heightN;            ///< Numerator of clean aperture height fractional value, in counted pixels
        uint32_t heightD;            ///< Denominator of clean aperture height fractional value, in counted pixels
        uint32_t horizontalOffsetN;  ///< Numerator of fractional number to define the horizontal offset of clean
                                     ///< aperture center minus (width-1)/2.
        uint32_t horizontalOffsetD;  ///< Denominator of fractional number to define the horizontal offset of clean
                                     ///< aperture center minus (width-1)/2.
        uint32_t verticalOffsetN;    ///< Numerator of fractional number to define the vertical offset of clean aperture
                                     ///< center minus (height-1)/2.
        uint32_t verticalOffsetD;  ///< Denominator of fractional number to define the vertical offset of clean aperture
                                   ///< center minus (height-1)/2.
    };

    /// Data of descriptive item property for auxiliary images 'auxC'.
    /// AuxProperty.auxType : "urn:mpeg:hevc:2015:auxid:1" for alpha plane
    ///                       "urn:mpeg:hevc:2015:auxid:2" for depth map
    /// AuxProperty.subType : HEVC auxiliary images depth case see "B.2.4.2 Syntax" of ISO/IEC 23008-12:2017(E) for
    /// further details See "6.5.8 Image properties for auxiliary images" of ISO/IEC 23008-12:2017(E) for more details
    struct HEIF_DLL_PUBLIC AuxiliaryType
    {
        Array<char> auxType;     ///< Type of the associated auxiliary image item. This is not null-terminated.
        Array<uint8_t> subType;  ///< Aux subtype, semantics depends on the auxType value
    };

    /// Data of raw/custom item property.
    struct HEIF_DLL_PUBLIC RawProperty
    {
        FourCC type;  ///< Box type of the property, for convenient reader API access. Type is included also in data
                      ///< array, as a part of box header. Writer ignores this field.
        Array<uint8_t> data;  ///< Complete data of the property. This includes box or possible full box header (size,
                              ///< type, version, flags).
    };

    // Non-image item type definitions
    /**
     * Image overlay derivation */
    struct HEIF_DLL_PUBLIC Overlay
    {
        std::uint16_t r;  ///< Fill color red value for empty canvas. The RGB values are in the sRGB color space as
                          ///< defined in IEC 61966-2-1.
        std::uint16_t g;  ///< Fill color green value for empty canvas. The RGB values are in the sRGB color space as
                          ///< defined in IEC 61966-2-1.
        std::uint16_t b;  ///< Fill color blue value for empty canvas. The RGB values are in the sRGB color space as
                          ///< defined in IEC 61966-2-1.
        std::uint16_t
            a;  ///< Fill color for empty canvas. Opacity range is from 0 (transparent) to 65535 (fully opaque).
        std::uint32_t outputWidth;   ///< Width of the reconstructed overlay image
        std::uint32_t outputHeight;  ///< Height of the reconstructed overlay image

        /** Offsets of an image as a part of overlay derivation */
        struct Offset
        {
            std::int32_t horizontal;  ///< Horizontal offset of the image top-left corner, from the top-left corner of
                                      ///< the canvas.
            std::int32_t
                vertical;  ///< Vertical offset of the image top-left corner, from the top-left corner of the canvas.
        };
        Array<Offset> offsets;  ///< Image offsets. There must be one for each item referenced. Array size must match
                                ///< number of itemIds given.
        Array<ImageId>
            imageIds;  ///< Ids of the images to display in the overlay. Array size must match number of offsets.
    };

    /**
     * Data of Image grid type derived item */
    struct HEIF_DLL_PUBLIC Grid
    {
        std::uint32_t outputWidth;   ///< Width of the reconstructed grid image
        std::uint32_t outputHeight;  ///< Height of the reconstructed grid image
        std::uint32_t columns;       ///< Number of columns in the grid, must be 1 or more
        std::uint32_t rows;          ///< Number of rows in the grid, must be 1 or more
        Array<ImageId> imageIds;     ///< Ids of the images to display in the grid. Array size must match columns*rows.
    };

    struct HEIF_DLL_PUBLIC CodingConstraints
    {
        bool allRefPicsIntra;  ///< This flag when set to one indicates the restriction that samples that are not sync
                               ///< samples, if any, are predicted only from sync samples.
        bool intraPredUsed;  ///< False indicates that intra prediction is not used in the inter predicted images. True
                             ///< indicates that intra prediction may or may not be used in the inter predicted images.
        uint8_t maxRefPerPic;  ///< Maximum number of reference images that may be used for decoding any single image
                               ///< within an image sequence. (value 15 = any number)
    };

    enum class EditType
    {
        EMPTY,
        DWELL,
        SHIFT
    };

    /**
     * @brief A single edit EditList unit */
    struct HEIF_DLL_PUBLIC EditUnit
    {
        EditType editType;                ///< Edit unit type (empty, dwell, shift)
        std::int64_t mediaTimeInTrackTS;  ///< Edit time in media in timescale units of track
        std::uint64_t durationInMovieTS;  ///< Edit unit length in timescale units of movie (writer uses value 1000 for
                                          ///< movie timescale)
    };

    /**
     * @brief Edit list for tracks */
    struct HEIF_DLL_PUBLIC EditList
    {
        bool looping       = false;  ///< True if the edit list should be repeated indefinitely.
        double repetitions = 0.0;    ///< Edit list repetitions, a non-negative value. Only affects when looping is set
                                     ///< true. Zero means infinite looping.
        Array<EditUnit> editUnits;   ///< Edit units in the order they should be applied.
    };
}  // namespace HEIF

#endif /* HEIFCOMMONDATATYPES_H */
