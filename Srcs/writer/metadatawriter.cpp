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

#include "metadatawriter.hpp"
#include "idspace.hpp"
#include "metabox.hpp"
#include "writerconstants.hpp"

#include <fstream>
#include <memory>
#include <stdexcept>

#define ANDROID_STOI_HACK
#include "androidhacks.hpp"

MetadataWriter::MetadataWriter(const IsoMediaFile::Metadata& config) :
    MetaWriter(),
    mLength(0),
    mId(IdSpace::getValue()),
    mConfig(config)
{
    if (config.hdlr_type == EXIF_HANDLER)
    {
        mType = EXIF_ITEM_TYPE;
    }
    else if (config.hdlr_type == XML_HANDLER)
    {
        mType = XML_ITEM_TYPE;
    }
    else
    {
        throw std::runtime_error("Unknown metadata handler '" + config.hdlr_type + "'");
    }
}

void MetadataWriter::write(MetaBox* metaBox)
{
    MetaWriter::initWrite();
    mLength = getMetadataLength();
    metaBox->addIloc(mId, 0, mLength, getBaseOffset());
    iinfWrite(metaBox);
    irefWrite(metaBox);
}

std::uint32_t MetadataWriter::getFileSize(const std::string& filename) const
{
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in | std::ios::binary);
    if (not file.good())
    {
        throw std::runtime_error("Unable to open metadata file '" + mConfig.file_path + "'");
    }
    file.seekg(0, std::ios::end);
    return file.tellg();
}

/// @todo Add proper support for exif_tiff_header_offset.
std::uint32_t MetadataWriter::getMetadataLength() const
{
    std::uint32_t length = getFileSize(mConfig.file_path);
    if (mType == EXIF_ITEM_TYPE)
    {
        static const std::uint32_t TIFF_HEADER_OFFSET_LENGTH = 4; // size of ExifDataBlock.exif_tiff_header_offset
        length += TIFF_HEADER_OFFSET_LENGTH;
    }
    else
    {
        throw std::runtime_error("Unknown handler type '" + mType + "' while adding metadata.");
    }

    return length;
}

void MetadataWriter::iinfWrite(MetaBox* metaBox)
{
    if (mType == EXIF_ITEM_TYPE)
    {
        metaBox->addItem(mId, mType, "Exif data");
    }
    else if (mType == XML_ITEM_TYPE)
    {
        metaBox->addItem(mId, mType, "XML data");
    }
    else
    {
        throw std::runtime_error("Unknown item.mType '" + mType + "'");
    }
}

void MetadataWriter::irefWrite(MetaBox* metaBox)
{
    const std::vector<std::string> masterImages = getMasterStoreValue("item_indx");
    if (masterImages.size() == 0)
    {
        throw std::runtime_error("No image found for metadata");
    }
    const uint32_t imageId = std::stoi(masterImages.at(0));

    metaBox->addItemReference("cdsc", mId, imageId);
}
