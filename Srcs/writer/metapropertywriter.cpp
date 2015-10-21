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

#include "metapropertywriter.hpp"
#include "cleanaperture.hpp"
#include "datastore.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "metabox.hpp"
#include "services.hpp"
#include "writerconstants.hpp"

#include <algorithm>
#include <string>
#include <memory>

#define ANDROID_STOI_HACK
#include "androidhacks.hpp"

MetaPropertyWriter::MetaPropertyWriter(const IsoMediaFile::Configuration& config,
    const IsoMediaFile::Content& contentConfig) :
    MetaWriter("", contentConfig.property.contextId),
    mConfig(config),
    mContentConfig(contentConfig)
{
    linkMasterDataStore(contentConfig.master.contextId);
}

void MetaPropertyWriter::write(MetaBox* metaBox)
{
    MetaWriter::initWrite();

    writeIrot(metaBox);
    writeRloc(metaBox);
    writeClap(metaBox);
}

void MetaPropertyWriter::writeIrot(MetaBox* metaBox) const
{
    for (const IsoMediaFile::Irot& irot : mContentConfig.property.irots)
    {
        const std::vector<uint32_t> itemIds = getReferenceItemIds(irot.refs_list, irot.idxs_list);
        auto irotBox = std::make_shared<ImageRotation>();
        irotBox->setAngle(irot.angle);
        metaBox->addProperty(irotBox, itemIds, irot.essential);
    }
}

void MetaPropertyWriter::writeRloc(MetaBox* metaBox) const
{
    for (const IsoMediaFile::Rloc& rloc : mContentConfig.property.rlocs)
    {
        const std::vector<uint32_t> itemIds = getReferenceItemIds(rloc.refs_list, rloc.idxs_list);
        auto rlocBox = std::make_shared<ImageRelativeLocationProperty>();
        rlocBox->setHorizontalOffset(rloc.horizontal_offset);
        rlocBox->setVerticalOffset(rloc.vertical_offset);
        metaBox->addProperty(rlocBox, itemIds, rloc.essential);
    }
}

void MetaPropertyWriter::writeClap(MetaBox* metaBox) const
{
    for (const IsoMediaFile::Clap& clap : mContentConfig.property.claps)
    {
        const std::vector<uint32_t> itemIds = getReferenceItemIds(clap.refs_list, clap.idxs_list);
        auto clapBox = std::make_shared<CleanAperture>();
        CleanAperture::Fraction value;
        value.numerator = clap.clapHeightN;
        value.denominator = clap.clapHeightD;
        clapBox->setHeight(value);
        value.numerator = clap.clapWidthN;
        value.denominator = clap.clapWidthD;
        clapBox->setWidth(value);
        value.numerator = clap.horizOffN;
        value.denominator = clap.horizOffD;
        clapBox->setHorizOffset(value);
        value.numerator = clap.vertOffN;
        value.denominator = clap.vertOffD;
        clapBox->setVertOffset(value);
        metaBox->addProperty(clapBox, itemIds, clap.essential);
    }
}


std::vector<uint32_t> MetaPropertyWriter::getReferenceItemIds(const IsoMediaFile::ReferenceList& refsList,
    const IsoMediaFile::IndexList& indexList) const
{
    // We have to search all possible masters because properties can span multiple
    // content declarations although they are defined at individual content scope.
    std::vector<uint32_t> itemIds;
    for (const IsoMediaFile::Content& content : mConfig.content)
    {
        int refsindex = 0;
        for (const auto uniq_bsid : refsList)
        {
            /// @todo Add support for searches in derived content as well.
            if (uniq_bsid == content.master.uniq_bsid)
            {
                // Now we found the master configuration from which we'll take the item Ids.
                std::shared_ptr<DataStore> dataStore = DataServe::getStore(content.master.contextId);
                for (const auto itemIdindex : indexList.at(refsindex))
                {
                    itemIds.push_back(std::stoi(dataStore->getValue("item_indx").at(itemIdindex - 1)));
                }
            }
            ++refsindex;
        }
    }
    return itemIds;
}
