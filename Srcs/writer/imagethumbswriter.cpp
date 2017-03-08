/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "imagethumbswriter.hpp"
#include "log.hpp"
#include "metabox.hpp"
#include "writerconstants.hpp"

#define ANDROID_STOI_HACK
#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

ImageThumbsWriter::ImageThumbsWriter(const IsoMediaFile::Thumbs& config, const std::vector<std::string>& handlerType, std::uint32_t contextId) :
    RootMetaImageWriter(handlerType, contextId),
    mConfig(config)
{
}

void ImageThumbsWriter::write(MetaBox* metaBox)
{
    RootMetaImageWriter::initWrite();
    storeValue("uniq_bsid", std::to_string(mConfig.uniq_bsid));
    storeValue("capsulation", META_ENCAPSULATION);
    generateThumbnailIndex();
    RootMetaImageWriter::parseInputBitStream(mConfig.file_path, mConfig.code_type);
    removeUnusedImages();
    RootMetaImageWriter::ilocWrite(metaBox);
    RootMetaImageWriter::iinfWrite(metaBox);
    addThumbReferences(metaBox);
    RootMetaImageWriter::iprpWrite(metaBox);
}

void ImageThumbsWriter::removeUnusedImages()
{
    std::vector<MetaItem> finalThumbs;
    for (auto imageIndex : mImagesWithThumbs)
    {
        finalThumbs.push_back(mMetaItems.at(imageIndex));
    }
    mMetaItems = finalThumbs;
}

ImageThumbsWriter::ImageIndexes ImageThumbsWriter::listSyncRateThumbs(const unsigned int masterImageCount) const
{
    const int syncRate = mConfig.sync_rate;
    ImageIndexes result;
    for (ImageIndex i = 0; i < masterImageCount; i += syncRate)
    {
        result.push_back(i);
    }
    return result;
}

ImageThumbsWriter::ImageIndexes ImageThumbsWriter::listSyncIndexThumbs(const unsigned int masterImageCount) const
{
    ImageIndexes result;
    for (auto image : mConfig.sync_idxs)
    {
        ImageIndex imageNumber = image - 1; // "sync_idxs" indexing starts from 1
        if (imageNumber >= masterImageCount)
        {
            throw std::runtime_error("No master image for thumbnail index " + std::to_string(image));
        }
        result.push_back(imageNumber);
    }
    return result;
}

void ImageThumbsWriter::generateThumbnailIndex()
{
    mImagesWithThumbs.clear(); // Clear because of dual pass
    const unsigned int masterImageCount = getMasterStoreValue("item_indx").size();

    if (mConfig.sync_rate != 0)
    {
        mImagesWithThumbs = listSyncRateThumbs(masterImageCount);
    }
    else if (mConfig.sync_idxs.size() > 0)
    {
        mImagesWithThumbs = listSyncIndexThumbs(masterImageCount);
    }
    else
    {
        logWarning() << "No sync_rate or sync_idxs for thumbs found. Discarding possible thumbnails." << std::endl;
    }
}


void ImageThumbsWriter::addThumbReferences(MetaBox* metaBox)
{
    std::vector<std::string> masterImageIds = getMasterStoreValue("item_indx");

    ImageIndex thumbIndex = 0;
    for (const auto& item : mMetaItems)
    {
        int masterImageIndex = mImagesWithThumbs.at(thumbIndex);
        int masterImageId = std::stoi(masterImageIds.at(masterImageIndex));
        metaBox->addItemReference("thmb", item.mId, masterImageId);
        ++thumbIndex;
    }
}
