/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#ifndef ISOMEDIAFILE_HPP
#define ISOMEDIAFILE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

/**
 * @class IsoMediaFile
 * @brief Declare an interface for writing an ISOBMFF based file.
 * @todo contextId must not be exposed outside the interface. */
class IsoMediaFile
{
public:
    struct Configuration; // forward declaration

    /** @brief The instance creator. */
    static std::unique_ptr<IsoMediaFile> newInstance();

    /**
     * @brief Method to generate the output file.
     * @param configuration File writer input configuration. */
    virtual void makeFile(const Configuration& configuration) = 0;

    /** @brief Virtual default destructor. */
    virtual ~IsoMediaFile() = default;

    typedef std::vector<std::vector<std::uint32_t>> IndexList; ///< Convenience type for reference indexes
    typedef std::vector<std::uint32_t> ReferenceList; ///< Convenience type for reference contexts

    /**
     * @brief Brands configuration */
    struct Brands
    {
        std::string major;              ///< Major brand
        std::vector<std::string> other; ///< Other brands
    };

    /**
     * @brief General configuration */
    struct General
    {
        std::string output_file; ///< Output file name
        Brands brands;           ///< Brand configuration
        std::uint32_t prim_refr; ///< Primary item context uniq_bsid
        std::uint32_t prim_indx; ///< Primary item index (1-based)
    };

    /**
     * @brief A single edit EditList unit */
    struct EditUnit
    {
        std::string edit_type;   ///< Edit unit type ('empty', 'dwell', 'shift')
        std::uint32_t mdia_time; ///< Edit time in media in milliseconds
        std::uint32_t time_span; ///< Edit unit length in milliseconds
    };

    /**
     * @brief Edit list for tracks */
    struct EditList
    {
        std::int32_t numb_rept;          ///< -1 = infinite looping, otherwise extra repetitions of the edit list (0 = play once)
        std::vector<EditUnit> edit_unit; ///< Edit units in the order they should be applied.
    };

    /**
     * @brief Identify a track or image item. */
    struct AltrIndexPair
    {
        unsigned int uniq_bsid; ///< Uniq_bsid of the context
        unsigned int item_indx; ///< 1-based index of image, or 0 if referring to a track context
    };
    typedef std::vector<AltrIndexPair> AltrIndexPairVector; ///< Convenience type for vector of alternatives

    /**
     * @brief Writer configuration for track and item alternatives */
    struct Altr
    {
        std::vector<AltrIndexPairVector> idxs_lists; ///< Alternative groups
    };

    /**
     * @brief Writer configuration for content master images.
     * @details There is one Master configuration in each Content. */
    struct Master
    {
        std::uint32_t contextId; ///< @todo This Context ID should be refactored away from configuration.
        std::uint32_t uniq_bsid; ///< Unique identifier for this context, used for referencing from another sections
        bool make_vide;          ///< Set true for tracks when a handler type 'vide' TrackBox should be generated in addition to 'pict' type.
        std::string file_path;   ///< Bitstream input file
        std::string hdlr_type;   ///< Handler type
        std::string code_type;   ///< Codec type
        std::string encp_type;   ///< Encapsulation type: 'meta' or 'trak'
        std::uint32_t disp_xdim; ///< Display width in pixels
        std::uint32_t disp_ydim; ///< Display height in pixels
        std::uint32_t disp_rate; ///< Display rate in frames per second (only for tracks)
        std::uint32_t tick_rate; ///< Tick rate (only for tracks)
        EditList edit_list;      ///< Edit list (only for tracks)
    };

    /**
     * @brief Writer configuration for content thumbnail images.
     * @details There can be 0 or many Thumbs configurations in each Content. */
    struct Thumbs
    {
        std::uint32_t uniq_bsid; ///< Unique identifier for this context, used for referencing from another sections
        std::string file_path;   ///< Bitstream input file
        std::string hdlr_type;   ///< Handler type
        std::string code_type;   ///< Codec type
        std::uint32_t disp_xdim; ///< Display width in pixels
        std::uint32_t disp_ydim; ///< Display height in pixels
        std::uint32_t tick_rate; ///< Tick rate (only for tracks)
        EditList edit_list;      ///< Edit list (only for tracks)
        std::uint32_t sync_rate; ///< Generate thumbnail for every sync_rate master image, alternate for sync_idxs
        std::vector<std::uint32_t> sync_idxs; ///< Running numbers of images to generate thumbnails for, alternate for sync_rate
    };

    /**
     * @brief Writer configuration for metadata */
    struct Metadata
    {
        std::string file_path; ///< Metadata input file
        std::string hdlr_type; ///< Handler type ('xml1', 'exif')
    };

    /**
     * @brief Writer configuration for entity groupings */
    struct Egroups
    {
        Altr altr; ///< Alternative ('altr') type groupings
    };

    /**
     * @brief Writer configuration for Image rotation Transformative item property */
    struct Irot
    {
        std::uint32_t uniq_bsid; ///< Unique identifier for this context, used for referencing from another sections
        bool essential;          ///< True if the property is essential to item. A reader is required to process essential properties.
        ReferenceList refs_list; ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;     ///< 1-based sequential numbers, one vector for each item in refs_list, to associate this property to
        std::uint32_t angle;     ///< Angle of rotation in anti-clockwise direction. Valid values are 0, 90, 180 and 270.
    };

    /**
     * @brief Writer configuration for Relative location Descriptive item property */
    struct Rloc
    {
        std::uint32_t uniq_bsid;         ///< Unique identifier for this context, used for referencing from another sections
        bool essential;                  ///< True if the property is essential to item. A reader is required to process essential properties.
        ReferenceList refs_list;         ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;             ///< 1-based sequential numbers, one vector for each item in refs_list, to associate this property to
        std::uint32_t horizontal_offset; ///< Horizontal offset in pixels of the left-most pixel column
        std::uint32_t vertical_offset;   ///< Vertical offset in pixels of the top-most pixel row
    };

    /**
     * @brief Writer configuration for Clean aperture Transformative item property */
    struct Clap
    {
        std::uint32_t uniq_bsid;   ///< Unique identifier for this context, used for referencing from another sections
        bool essential;            ///< True if the property is essential to item. A reader is required to process essential properties.
        ReferenceList refs_list;   ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;       ///< 1-based sequential numbers, one vector for each item in refs_list, to associate this property to
        std::uint32_t clapWidthN;  ///< Numerator of clean aperture width fractional value, in counted pixels
        std::uint32_t clapWidthD;  ///< Denominator of clean aperture width fractional value, in counted pixels
        std::uint32_t clapHeightN; ///< Numerator of clean aperture height fractional value, in counted pixels
        std::uint32_t clapHeightD; ///< Denominator of clean aperture height fractional value, in counted pixels
        std::uint32_t horizOffN;   ///< Numerator of fractional number to define the horizontal offset of clean aperture center minus (width-1)/2.
        std::uint32_t horizOffD;   ///< Denominator of fractional number to define the horizontal offset of clean aperture center minus (width-1)/2.
        std::uint32_t vertOffN;    ///< Numerator of fractional number to define the vertical offset of clean aperture center minus (height-1)/2.
        std::uint32_t vertOffD;    ///< Denominator of fractional number to define the vertical offset of clean aperture center minus (height-1)/2.
    };

    /**
     * @brief Writer configuration for Pre-derived coded images */
    struct PreDerived
    {
        std::uint32_t uniq_bsid;      ///< Unique identifier for this context, used for referencing from another sections
        ReferenceList pre_refs_list;  ///< uniq_bsids of contexts referenced in pre_idxs_list
        IndexList pre_idxs_list;      ///< 1-based sequential numbers, one vector for each item in pre_refs_list, to mark as pre-derived coded image
        ReferenceList base_refs_list; ///< uniq_bsids of contexts referenced in base_idxs_list
        IndexList base_idxs_list;     ///< 1-based sequential numbers, one vector for each item in base_refs_list, to mark as base for pre-derived coded image
    };

    /**
     * @brief Writer configuration of Image grid derivation */
    struct Grid
    {
        std::uint32_t uniq_bsid;    ///< Unique identifier for this context, used for referencing from another sections
        ReferenceList refs_list;    ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;        ///< 1-based sequential numbers, one vector for each item in refs_list, to refer to images used in the grid
        std::uint32_t outputWidth;  ///< Width of the reconstructed grid image
        std::uint32_t outputHeight; ///< Height of the reconstructed grid image
        std::uint32_t columns;      ///< Number of columns in the grid, must be 1 or more
        std::uint32_t rows;         ///< Number of rows in the grid, must be 1 or more
    };

    /**
     * @brief Writer configuration for Image overlay derivation */
    struct Iovl
    {
        std::uint32_t uniq_bsid;                    ///< Unique identifier for this context, used for referencing from another sections
        ReferenceList refs_list;                    ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;                        ///< 1-based sequential numbers, one vector for each item in refs_list, to refer to images used in the overlay
        std::vector<std::uint16_t> canvasFillValue; ///< Fill color for empty canvas. Values are in order R, G, B, A. Opacity A range is from 0 (transparent) to 65535 (fully opaque).
        std::uint32_t outputWidth;                  ///< Width of the reconstructed overlay image
        std::uint32_t outputHeight;                 ///< Height of the reconstructed overlay image

        /** @brief Offsets of an image as a part of overlay derivation */
        struct Offset
        {
            std::int32_t horizontal; ///< Horizontal offset of the image top-left corner, from the top-left corner of the canvas.
            std::int32_t vertical;   ///< Vertical offset of the image top-left corner, from the top-left corner of the canvas.
        };
        std::vector<Offset> offsets; ///< Image offsets. There must be one for each item in idxs_list.
    };

    /**
     * @brief Writer configuration for derived images of a content */
    struct Derived
    {
        std::uint32_t contextId;             ///< @todo This Context ID should be refactored away from configuration.
        std::vector<Irot> irots;             ///< Property configurations used to generate derived image items by identity transformation
        std::vector<Rloc> rlocs;             ///< Property configurations used to generate derived image items by identity transformation
        std::vector<Clap> claps;             ///< Property configurations used to generate derived image items by identity transformation
        std::vector<PreDerived> prederiveds; ///< Pre-derived coded image configurations
        std::vector<Grid> grids;             ///< Image grid derivation configurations
        std::vector<Iovl> iovls;             ///< Image overlay derivation configurations
    };

    /**
     * @brief Writer configuration Properties of a content
     * @todo Add support for Colour information descriptive item property 'colr'
     * @todo Add support for Pixel information descriptive item property 'pixi' */
    struct Property
    {
        std::uint32_t contextId; ///< @todo This Context ID should be refactored away from configuration.
        std::vector<Irot> irots; ///< Image rotation property configurations
        std::vector<Rloc> rlocs; ///< Relative location property configurations
        std::vector<Clap> claps; ///< Clean aperture property configurations
    };

    /**
     * @brief Writer configuration for an auxiliary image or images */
    struct Auxiliary
    {
        std::uint32_t uniq_bsid; ///< Unique identifier for this context, used for referencing from another sections
        std::string file_path;   ///< Bitstream input file
        std::string urn;         ///< URN to specify type of the auxiliary image. Format is "urn:mpeg:hevc:2015:auxid:xxx" where xxx comes from Table F.2 of ISO/IEC 23008-2.
        std::uint32_t disp_xdim; ///< Display width in pixels
        std::uint32_t disp_ydim; ///< Display height in pixels
        ReferenceList refs_list; ///< uniq_bsids of contexts referenced in idxs_list
        IndexList idxs_list;     ///< 1-based sequential numbers, one vector for each item in refs_list, to refer to master images from auxiliary images
        bool hidden;             ///< True if the image is not intended to be displayed
    };

    /**
     * @brief Writer configuration of one content */
    struct Content
    {
        Master master;                    ///< Configuration for master images
        std::vector<Thumbs> thumbs;       ///< Configurations for thumbnail images
        std::vector<Metadata> metadata;   ///< Configurations for metadata (xml, Exif)
        std::vector<Auxiliary> auxiliary; ///< Configurations for auxiliary images
        Derived derived;                  ///< Configurations for derived images
        Property property;                ///< Configurations for properties
    };

    /**
     * @brief Writer main configuration */
    struct Configuration
    {
        General general;              ///< Global configuration
        std::vector<Content> content; ///< Content configuration, 1 or more. Each has own Master configuration.
        Egroups egroups;              ///< Entity grouping configuration
    };
};

#endif /* end of include guard: ISOMEDIAFILE_HPP */
