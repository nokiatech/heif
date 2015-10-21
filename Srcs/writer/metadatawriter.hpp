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

#ifndef METADATAWRITER_HPP
#define METADATAWRITER_HPP

#include "isomediafile.hpp"
#include "metawriter.hpp"
#include <memory>
#include <string>

class MetaBox;

/**
 * @brief MetaBox writer for Exif and XML metadata
 * @details This handles only one metadata/master image pair. The reference is made from the metadata to the
 *         first master image found.
 * @todo Proper handling of XML metadata.
 * @todo More flexible configuration of the target image item
 */
class MetadataWriter : public MetaWriter
{
public:
    MetadataWriter(const IsoMediaFile::Metadata& config);
    virtual ~MetadataWriter() = default;

    /**
     * @brief Add data to a MetaBox based on configuration data given during writer construction.
     * @see MetaWriter::write()
     */
    void write(MetaBox* metaBox);

private:
    std::string mType;     ///< Item type in 'iinf' box
    std::uint32_t mLength; ///< Length of the metadata
    std::uint32_t mId;     ///< Item ID of the related image
    IsoMediaFile::Metadata mConfig; ///< Input configuration of the writer

    /**
     * @brief Add entry to the Item Information Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     */
    void iinfWrite(MetaBox* metaBox);

    /**
     * @brief Add entry to the Item Reference Box
     * @param [in,out] metaBox Pointer to the MetaBox where to add data.
     * */
    void irefWrite(MetaBox* metaBox);

    /**
     * @return Length of the metadata in bytes, including possible header
     */
    std::uint32_t getMetadataLength() const;

    /**
     * Get size of a file
     * @param [in] filename Name of the file
     * @return Size of the file in bytes
     */
    std::uint32_t getFileSize(const std::string& filename) const;
};

#endif /* end of include guard: METADATAWRITER_HPP */

