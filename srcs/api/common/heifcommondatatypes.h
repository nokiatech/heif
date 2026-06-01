/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include <iterator>

#include <string>
#include <map>

#include "heifexport.h"
#include "heifid.h"


#ifndef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif

namespace HEIF
{
    /** @brief HEIF file reader and writer ErrorCode enumeration.
     *          These might need updating with more informational comment
     *           descriptions based on actual usage in code.
     *
     * Underlying type pinned to std::uint8_t. */
    enum class ErrorCode : std::uint8_t
    {
        OK = 0,                          ///< Operation completed successfully.
        ALLOCATOR_ALREADY_SET,           ///< Custom allocator was already configured.
        ALREADY_INITIALIZED,             ///< Reader or writer instance already initialised.
        BRANDS_NOT_SET,                  ///< File brands have not been set before writing.
        BUFFER_SIZE_TOO_SMALL,           ///< Caller-supplied buffer is too small for the result.
        CONTENT_TYPE_NOT_SET,            ///< Content type not configured for the item.
        DECODER_CONFIGURATION_ERROR,     ///< Decoder configuration record is invalid or missing.
        FILE_HEADER_ERROR,               ///< File-level header (ftyp/meta) could not be parsed.
        FILE_OPEN_ERROR,                 ///< File could not be opened for reading.
        FILE_READ_ERROR,                 ///< I/O error while reading file data.
        FTYP_ALREADY_WRITTEN,            ///< FileTypeBox has already been emitted.
        HIDDEN_PRIMARY_ITEM,             ///< Primary item is marked as hidden (spec violation).
        INVALID_FUNCTION_PARAMETER,      ///< A function argument is out of range or null.
        INVALID_GROUP_ID,                ///< Entity group ID does not exist in this file.
        INVALID_ITEM_ID,                 ///< Item ID does not exist in this file.
        INVALID_MEDIADATA_ID,            ///< Media data ID does not exist in this file.
        INVALID_METADATAITEM_ID,         ///< Metadata item ID does not exist in this file.
        INVALID_MEDIA_FORMAT,            ///< Media format is not recognised or supported.
        INVALID_PROPERTY_INDEX,          ///< Property index is out of range.
        INVALID_REFERENCE_COUNT,         ///< Reference count does not match expectations.
        INVALID_SAMPLE_DESCRIPTION_INDEX,///< Sample description index is out of range.
        INVALID_SEGMENT,                 ///< Segment index or data is invalid.
        INVALID_SEQUENCE_ID,             ///< Sequence (track) ID does not exist in this file.
        INVALID_DECODER_CONFIG_ID,       ///< Decoder configuration ID does not exist.
        INVALID_SEQUENCE_IMAGE_ID,       ///< Sequence image (sample) ID does not exist.
        MEDIA_PARSING_ERROR,             ///< Media-level bitstream parsing failed.
        MEMORY_TOO_SMALL_BUFFER,         ///< Internal buffer allocation was too small.
        NOT_APPLICABLE,                  ///< Operation is not applicable to this item/track.
        NO_MEDIA,                        ///< No media data found in the file.
        PRIMARY_ITEM_NOT_SET,            ///< Primary item has not been designated.
        PROTECTED_ITEM,                  ///< Item is protected; access requires decryption.
        UNINITIALIZED,                   ///< Reader or writer has not been initialised.
        UNPROTECTED_ITEM,                ///< Item is not protected when protection was expected.
        UNSUPPORTED_CODE_TYPE,           ///< Codec type is not supported by this build.

        // MiniBox-specific error codes.
        TMI,                             ///< Too many items for a MiniBox-constrained file.
        MISSING_PROPERTIES,              ///< MinBox requires ispe, colr, pixi, and a codec config.
        MULTIPLE_GAINMAPS,               ///< More than one gain map is not supported in MiniBox.
        INVALID_EXIF_ORIENTATION_VALUE,  ///< Exif orientation value is outside the 0-7 range.
        BYTESTREAM_READ_ERROR,           ///< Low-level bitstream read failed.
        VVC_DISABLED,                    ///< VVC support is disabled in this build.
        UNKNOWN_ERROR                    ///< Unclassified internal error.
    };

    struct HEIF_DLL_PUBLIC FourCC
    {
        char value[5];
        inline FourCC()
            : value{}
        {
        }
        // NOLINTNEXTLINE(google-explicit-constructor): public API: integrators write FourCC fcc = 0x68656963; ("heic").
        inline FourCC(uint32_t v) noexcept
        {
            value[0] = static_cast<char>((v >> 24) & 0xff);
            value[1] = static_cast<char>((v >> 16) & 0xff);
            value[2] = static_cast<char>((v >> 8) & 0xff);
            value[3] = static_cast<char>((v >> 0) & 0xff);
            value[4] = '\0';
        }
        // NOLINTNEXTLINE(google-explicit-constructor): public API: integrators write FourCC fcc = "heic";
        inline FourCC(const char* str) noexcept
        {
            // Copy up to 4 characters, stopping at the null terminator
            // to avoid reading past the end of short strings (e.g. "").
            for (int i = 0; i < 4; ++i)
            {
                value[i] = str[i];
                if (str[i] == '\0')
                {
                    for (int j = i + 1; j < 4; ++j)
                    {
                        value[j] = '\0';
                    }
                    break;
                }
            }
            value[4] = '\0';
        }
        inline FourCC(const FourCC& fourcc) noexcept = default;
        inline FourCC& operator=(const FourCC& other) = default;
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
            return (value[0] < other.value[0])   ? true
                   : (value[0] > other.value[0]) ? false
                   : (value[1] < other.value[1]) ? true
                   : (value[1] > other.value[1]) ? false
                   : (value[2] < other.value[2]) ? true
                   : (value[2] > other.value[2]) ? false
                   : (value[3] < other.value[3]) ? true
                                                 : false;
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
        explicit Array(size_t n);
        Array(const Array& other);
        Array& operator=(const Array& other);
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
        Array(U begin, U end)
            : Array(static_cast<size_t>(std::distance(begin, end)))
        {
            auto it      = begin;
            size_t index = 0;
            while (it != end)
            {
                elements[index] = *it;
                ++it;
                ++index;
            }
        }
        Array(std::initializer_list<T> aInit);
        virtual ~Array();
    };

    // NOLINTBEGIN(google-explicit-constructor) -- IdType wraps a uint with intentional implicit
    // conversion from the underlying integer; that's the public API contract for these handles.
    IdType(std::uint32_t, GroupId);
    IdType(std::uint32_t, ImageId);
    IdType(std::uint32_t, SegmentId);
    IdType(std::uint32_t, SequenceImageId);
    IdType(std::uint32_t, SequenceId);
    IdType(std::uint32_t, DecoderConfigId);
    IdType(std::uint32_t, PropertyId);
    IdType(std::uint32_t, MetadataItemId);
    IdType(std::uint32_t, TrackGroupId);
    IdType(std::uint32_t, MixedNalUnitTypeId);
    // NOLINTEND(google-explicit-constructor)

    struct HEIF_DLL_PUBLIC Rational
    {
        uint64_t num;
        uint64_t den;
    };

    enum class DecoderSpecInfoType : uint8_t
    {
        AVC_SPS = 7,  ///< H.264/AVC Sequence Parameter Set (SPS) nal unit, bytestream header (0001) if any is stripped
                      ///< internally.
        AVC_PPS = 8,  ///< H.264/AVC Picture Parameter Set (PPS) nal unit, bytestream header (0001) if any is stripped
                      ///< internally.

        VVC_VPS = 14,
        VVC_SPS = 15,
        VVC_PPS = 16,
        VVC_APS_PREFIX = 17,
        VVC_APS_SUFFIX = 18,

        HEVC_VPS = 32,  ///< H.265/HEVC Video Parameter Set (VPS) nal unit, bytestream header (0001) if any is stripped
                        ///< internally.
        HEVC_SPS = 33,  ///< H.265/HEVC Sequence Parameter Set (SPS) nal unit, bytestream header (0001) if any is
                        ///< stripped internally.
        HEVC_PPS = 34,  ///< H.265/HEVC Picture Parameter Set (PPS) nal unit, bytestream header (0001) if any is
                        ///< stripped internally.

        PREFIX_SEI_NUT = 39,
        SUFFIX_SEI_NUT = 40,

        AudioSpecificConfig = 63,  ///< As defined in 1.6.2.1 AudioSpecificConfig of ISO/IEC 14496-3:200X(E)

        JPEG = 108,  ///< This number probably comes from ISO/IEC 10918-1

        //Placeholder, use with caution
        RAW = 255   ///< RAW typing, used to passing unrecognized data
    };

    struct HEIF_DLL_PUBLIC DecoderSpecificInfo
    {
        DecoderSpecInfoType decSpecInfoType;  // can be "SPS" and "PPS" for AVC, additional "VPS" for HEVC
        Array<uint8_t> decSpecInfoData;
                int id;
    };

    struct HEIF_DLL_PUBLIC ItemDescription
    {
        Array<char> name;             ///< string in UTF-8 characters containing a symbolic name of the item
        Array<char> contentType;      ///< string in UTF-8 characters with the MIME type of the item
        Array<char> contentEncoding;  ///< an optional string in UTF-8 characters used to indicate that
                                      ///< the binary file is encoded and needs to be decoded before interpreted
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

    /// Data of descriptive item property Image Spatial Extents 'ispe'
    struct HEIF_DLL_PUBLIC SpatialExtents
    {
        std::uint32_t mImageWidth;   ///< Width of the reconstructed image in pixels.
        std::uint32_t mImageHeight;  ///< Height of the reconstructed image in pixels.
    };

    /// Data of Image scaling transformative item property 'iscl'.
    struct HEIF_DLL_PUBLIC Scale
    {
        uint16_t targetWidthN;  ///< Numerator of the scaling ratio in the horizontal dimension. Value 0 is not allowed.
        uint16_t
            targetWidthD;  ///< Denominator of the scaling ratio in the horizontal dimension. Value 0 is not allowed.
        uint16_t targetHeightN;  ///< Numerator of the scaling ratio in the vertical dimension. Value 0 is not allowed.
        uint16_t
            targetHeightD;  ///< Denominator of the scaling ratio in the vertical dimension. Value 0 is not allowed.
    };

    /// Data of descriptive item property Relative location 'rloc'.
    struct HEIF_DLL_PUBLIC RelativeLocation
    {
        uint32_t horizontalOffset;  ///< Horizontal offset in pixels of the left-most pixel column
        uint32_t verticalOffset;    ///< Vertical offset in pixels of the top-most pixel row
    };

    /// Data of descriptive item property Layer selector 'lsel'.
    /// Identifies the layer of a multi-layer (e.g. L-HEVC) item that the
    /// associated decoder configuration produces. Spec: ISO/IEC 23008-12.
    struct HEIF_DLL_PUBLIC LayerSelector
    {
        uint16_t layerId;  ///< Layer identifier value of the selected layer.
    };

    /// Data of descriptive item property Target Output Layer Set 'tols'.
    /// Identifies the target Output Layer Set within an associated 'oinf'
    /// operating-points table for a multi-layer (e.g. L-HEVC) image item.
    /// Spec: ISO/IEC 23008-12.
    struct HEIF_DLL_PUBLIC TargetOls
    {
        uint16_t targetOlsIndex;  ///< Index of the selected output layer set in the 'oinf' table.
    };

    /// Data of descriptive item property Operating Points Information 'oinf'.
    /// Enumerates the operating points (output layer sets), the
    /// profile/tier/level descriptors they reference, and the inter-layer
    /// dependency graph for a multi-layer (e.g. L-HEVC) image item.
    /// Spec: ISO/IEC 14496-15 Section 11.6.2 (record body),
    /// ISO/IEC 23008-12 (HEIF item-property wrapper).
    struct HEIF_DLL_PUBLIC OperatingPointsInfo
    {
        struct ProfileTierLevel
        {
            uint8_t  generalProfileSpace;               ///< 2-bit field
            uint8_t  generalTierFlag;                   ///< 1-bit field
            uint8_t  generalProfileIdc;                 ///< 5-bit field
            uint32_t generalProfileCompatibilityFlags;
            uint64_t generalConstraintIndicatorFlags;   ///< 48-bit field, upper 16 bits zero
            uint8_t  generalLevelIdc;
        };

        struct OperatingPointLayer
        {
            uint8_t ptlIdx;                  ///< 1-based index into profileTierLevels
            uint8_t layerId;                 ///< 6-bit layer id within the OLS
            uint8_t isOutputLayer;           ///< 1-bit flag, 0 or 1
            uint8_t isAlternateOutputLayer;  ///< 1-bit flag, 0 or 1
        };

        struct OperatingPoint
        {
            uint16_t                   outputLayerSetIdx;  ///< Matches the value in a 'tols' property.
            uint8_t                    maxTemporalId;
            Array<OperatingPointLayer> layers;
            uint16_t                   minPicWidth;
            uint16_t                   minPicHeight;
            uint16_t                   maxPicWidth;
            uint16_t                   maxPicHeight;
            uint8_t                    maxChromaFormat;     ///< 2-bit field
            uint8_t                    maxBitDepthMinus8;   ///< 3-bit field
            uint8_t                    frameRateInfoFlag;   ///< 1 if avgFrameRate / constantFrameRate are valid
            uint8_t                    bitRateInfoFlag;     ///< 1 if maxBitRate / avgBitRate are valid
            uint16_t                   avgFrameRate;        ///< Valid only if frameRateInfoFlag != 0
            uint8_t                    constantFrameRate;   ///< 2-bit field, valid only if frameRateInfoFlag != 0
            uint32_t                   maxBitRate;          ///< Valid only if bitRateInfoFlag != 0
            uint32_t                   avgBitRate;          ///< Valid only if bitRateInfoFlag != 0
        };

        struct LayerInfo
        {
            uint8_t          dependentLayerId;
            Array<uint8_t>   dependsOnLayerIds;
            Array<uint8_t>   dimensionIdentifiers;  ///< One entry per set bit in scalabilityMask, lowest-bit first.
        };

        uint16_t                 scalabilityMask;
        Array<ProfileTierLevel>  profileTierLevels;
        Array<OperatingPoint>    operatingPoints;
        Array<LayerInfo>         layers;
    };

    /// Data of descriptive item property Sub-sample information 'subs'.
    /// Each entry describes one sub-sample (byte range) within the coded
    /// image item's data, together with a priority and codec-specific role.
    /// Spec: ISO/IEC 14496-12 Section 8.7.7.
    struct HEIF_DLL_PUBLIC SubSampleInfo
    {
        struct SubSampleEntry
        {
            uint32_t subsampleSize;            ///< Byte count of this sub-sample.
            uint8_t  subsamplePriority;        ///< Lower values are more important.
            uint8_t  discardable;              ///< Non-zero if this sub-sample may be discarded.
            uint32_t codecSpecificParameters;  ///< Codec-defined semantics.
        };

        Array<SubSampleEntry> subSamples;
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
        struct ChannelInfo{
            uint8_t ComponentFormat;
            bool SubSamplingFlag;
            bool ChannelLabelFlag;
            uint8_t SubSamplingType;
            uint8_t SubSamplingLocation;
            Array<char> ChannelLabel;  ///< UTF-8 channel label string
        };
        /// A single channel entry: channel IDC key + channel info value.
        /// Channel IDC values: 0=Unused, 1=Unspecified, 2=(Y/R),
        /// 3=Second color (U/G), 4=Third color (V/B), 5=Alpha, 6=Depth, 7=Fourth color (K)
        struct ChannelEntry{
            uint8_t channelIdc;  ///< Channel identifier code (0-7)
            ChannelInfo info;    ///< Channel properties
        };
        uint8_t numChannels;
        Array<std::uint8_t> bitsPerChannel;  ///< Bits per each channel.
        Array<ChannelEntry> ChannelData;     ///< Channel specific values
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
        int32_t horizontalOffsetN;  ///< Numerator of fractional number to define the horizontal offset of clean
                                     ///< aperture center minus (width-1)/2.
        uint32_t horizontalOffsetD;  ///< Denominator of fractional number to define the horizontal offset of clean
                                     ///< aperture center minus (width-1)/2.
        int32_t verticalOffsetN;    ///< Numerator of fractional number to define the vertical offset of clean aperture
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

    /// Data of Required reference types descriptive item property.
    struct HEIF_DLL_PUBLIC RequiredReferenceTypes
    {
        Array<FourCC>
            referenceTypes;  ///< Reference types required to understand and process the associated image item.
    };

    /// Data of User description descriptive item property.
    struct HEIF_DLL_PUBLIC UserDescription
    {
        Array<char> lang;         ///< An RFC 5646 compliant language tag string (e.g. "en-US", "de")
        Array<char> name;         ///< Name for the item or group of entities.
        Array<char> description;  ///< Description of the item or group of entities.
        Array<char> tags;         ///< Comma-separated user-defined tags related to the item(s).
    };

    /// Data of Accessibility text descriptive item property.
    struct HEIF_DLL_PUBLIC AccessibilityText
    {
        Array<char>
            text;  ///< An alternate text for image, if the image can not be display. String in UTF-8 characters.
        Array<char> lang;  ///< An RFC 5646 compliant language tag string (e.g. "en-US", "de")
    };

    /// Data of Creation time information descriptive item property.
    struct HEIF_DLL_PUBLIC CreationTimeInformation
    {
        uint64_t time;  ///< Microseconds since midnight, January 1st, 1904, in UTC time.
    };

    /// Data of Modification time information descriptive item property.
    struct HEIF_DLL_PUBLIC ModificationTimeInformation
    {
        uint64_t time;  ///< Microseconds since midnight, January 1st, 1904, in UTC time.
    };

    struct HEIF_DLL_PUBLIC ContentLightLevel
    {
        uint16_t maxContentLightLevel;      ///< Maximum Content Light Level (MaxCLL) in candelas per square meter
        uint16_t maxPicAverageLightLevel;   ///< Maximum Frame-Average Light Level (MaxFALL) in candelas per square
                                            ///< meter
    };

    struct HEIF_DLL_PUBLIC MasteringDisplayColorVolume
    {
        Array<uint16_t> displayPrimariesX;
        Array<uint16_t> displayPrimariesY;
        uint16_t whitePointX;
        uint16_t whitePointY;
        uint32_t maxDisplayMasteringLuminance;
        uint32_t minDisplayMasteringLuminance;
    };

    struct HEIF_DLL_PUBLIC ContentColorVolume
    {
        bool ccvPrimariesPresent;
        bool ccvMinLuminanceValuePresent;
        bool ccvMaxLuminanceValuePresent;
        bool ccvAvgLuminanceValuePresent;

        Array<uint32_t> ccvPrimariesX;
        Array<uint32_t> ccvPrimariesY;

        uint32_t ccvMinLuminanceValue;
        uint32_t ccvMaxLuminanceValue;
        uint32_t ccvAvgLuminanceValue;
    };

    struct HEIF_DLL_PUBLIC AmbientViewingEnvironment
    {
        uint32_t ambientIlluminance;
        uint16_t ambientLightX;
        uint16_t ambientLightY;
    };

    struct HEIF_DLL_PUBLIC ReferenceViewingEnvironment
    {
        uint32_t surroundLuminance;
        uint16_t surroundLightX;
        uint16_t surroundLightY;
        uint32_t peripheryLuminance;
        uint16_t peripheryLightX;
        uint16_t peripheryLightY;
    };

    struct HEIF_DLL_PUBLIC NominalDiffuseWhite
    {
        uint32_t diffuseWhiteLuminance;
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

    struct HEIF_DLL_PUBLIC GainMapData{
        uint16_t minimumVersion;
        uint16_t writerVersion;

        bool isMultichannel;
        bool useBaseColourSpace;

        int channelCount;

        uint32_t baseHdrHeadroomNumerator;
        uint32_t baseHdrHeadroomDenominator;
        uint32_t alternateHdrHeadroomNumerator;
        uint32_t alternateHdrHeadroomDenominator;

        struct Channel{
            int32_t gainMapMinNumerator;
            uint32_t gainMapMinDenominator;
            int32_t gainMapMaxNumerator;
            uint32_t gainMapMaxDenominator;
            uint32_t gammaNumerator;
            uint32_t gammaDenominator;
            int32_t baseOffsetNumerator;
            uint32_t baseOffsetDenominator;
            int32_t alternateOffsetNumerator;
            uint32_t alternateOffsetDenominator;
        };

        Array<Channel> channels;
    };

    struct HEIF_DLL_PUBLIC Tonemap{
        Array<ImageId> imageIds; ///< Ids of the images to which tonemapping is applied. Array size must be 1 or more.
        GainMapData gain_map_metadata;
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

    /** @brief Edit list edit unit type. Underlying type pinned to std::uint8_t. */
    enum class EditType : std::uint8_t
    {
        EMPTY,
        DWELL,
        SHIFT,
        RAW
    };

    /**
     * @brief A single edit EditList unit */
    struct HEIF_DLL_PUBLIC EditUnit
    {
        EditType editType;                ///< Edit unit type (empty, dwell, shift)
        std::int64_t mediaTimeInTrackTS;  ///< Edit time in media in timescale units of track
        std::uint64_t durationInMovieTS;  ///< Edit unit length in timescale units of movie (writer uses value 1000 for
                                          ///< movie timescale)
        std::int16_t mediaRateInteger;    ///< Integer part of the media_rate (fixed-point 16.16)
        std::int16_t mediaRateFraction;   ///< Fraction part of the media_rate (fixed-point 16.16)
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

    /**
     * @brief VVC RectangularRegionGroupEntry 'trif' */
    struct HEIF_DLL_PUBLIC Trif
    {
       uint16_t groupId;
       bool rectRegionFlag;
       uint16_t independentIdc;
       bool fullPicture;
       bool filteringDisabled;
       uint16_t horizontalOffset;
       uint16_t verticalOffset;
       uint16_t regionWidth;
       uint16_t regionHeight;
       Array<std::uint16_t> dependencyRectRegionGroupIds;
    };

    /**
     * @brief VVC VvcSubpictureLayoutMapEntry 'sulm' */
    struct HEIF_DLL_PUBLIC Sulm
    {
       FourCC groupIdInfo;
       Array<std::uint16_t> groupIds;
    };

}  // namespace HEIF

#endif /* HEIFCOMMONDATATYPES_H */
