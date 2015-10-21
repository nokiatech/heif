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

#ifndef IMAGETHUMBSWRITER_HPP
#define IMAGETHUMBSWRITER_HPP

#include "rootmetaimagewriter.hpp"
#include "isomediafile.hpp"

class MetaBox;

/**
 * @brief MetaBox writer for writing thumbnail images.
 */
class ImageThumbsWriter : public RootMetaImageWriter
{
public:
    /**
    * @brief ImageThumbsWriter constructor
    * @param config      Configuration of this writer
    * @param handlerType Handler type
    * @param contextId   Context ID of this writer
    */
    ImageThumbsWriter(const IsoMediaFile::Thumbs& config, const std::vector<std::string>& handlerType, std::uint32_t contextId);
    virtual ~ImageThumbsWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    void write(MetaBox* metaBox);

private:
    typedef std::uint32_t ImageIndex;             ///< 0-based image index number
    typedef std::vector<ImageIndex> ImageIndexes; ///< A convenience type for ImageIndex vector
    IsoMediaFile::Thumbs mConfig;                 ///< Input configuration of the writer
    ImageIndexes mImagesWithThumbs;               ///< Sequence numbers of images which have thumbs (0-based)

    /** Add thumbnail references to the Item Reference Box */
    void addThumbReferences(MetaBox* metaBox);

    /** Generate index of needed images when processing thumbs meta */
    void generateThumbnailIndex();

    /**
     * @brief Generate vector of thumbnail image index numbers, based on sync_rate setting of input configuration.
     * @param masterImageCount Number of master images.
     * @return 0-based index numbers of thumbnail images. */
    ImageIndexes listSyncRateThumbs(unsigned int masterImageCount) const;

    /**
     * @brief Generate vector of thumbnail image index numbers, based on sync_idxs setting of input configuration.
     * @param masterImageCount Number of master images.
     * @return 0-based index numbers of thumbnail images. */
    ImageIndexes listSyncIndexThumbs(unsigned int masterImageCount) const;

    /**
     * Drop metadata of images which will not be used as thumbnails.
     */
    void removeUnusedImages();
};

#endif /* end of include guard: IMAGETHUMBSWRITER_HPP */

