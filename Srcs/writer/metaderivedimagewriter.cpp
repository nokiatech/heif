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

#include "metaderivedimagewriter.hpp"
#include "cleanaperture.hpp"
#include "datastore.hpp"
#include "deriveditemmediawriter.hpp"
#include "idspace.hpp"
#include "imagegrid.hpp"
#include "imageoverlay.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagespatialextentsproperty.hpp"
#include "itempropertiesbox.hpp"
#include "itemreferencebox.hpp"
#include "metabox.hpp"
#include "services.hpp"
#include "writerconstants.hpp"

#include <string>
#include <memory>

#define ANDROID_STOI_HACK
#define ANDROID_TO_STRING_HACK
#include "androidhacks.hpp"

MetaDerivedImageWriter::MetaDerivedImageWriter(const IsoMediaFile::Derived& config,
    DerivedItemMediaWriter* mediaWriter) :
    MetaWriter("", config.contextId),
    mDerivedItemMediaWriter(mediaWriter),
    mConfig(config)
{
}

void MetaDerivedImageWriter::write(MetaBox* metaBox)
{
    MediaDataBox mdat;
    ItemLocations itemLocationVector;

    MetaWriter::initWrite();
    mDerivations.clear(); // clear because of dual-pass writing

    /// @todo Refactor so there would not be need for this.
    storeValue("multiple_uniq_bsids", "1");

    DerivationMap idenDerivations = processIdenDerivations();
    DerivationMap derivationMap = processOtherDerivations(mdat, itemLocationVector);
    mDerivations.insert(idenDerivations.cbegin(), idenDerivations.cend());
    mDerivations.insert(derivationMap.cbegin(), derivationMap.cend());

    // MetaDerivedImageWriter creates also the related MediaDataBox. Just give it to media writer.
    mDerivedItemMediaWriter->setMdat(std::move(mdat));

    mReferenceToItemIdMap = createReferenceToItemIdMap();
    addItemIdReferences(mDerivations);

    // First create the "iden"s
    iinfWrite(metaBox);
    // then write the properties...
    iprpWrite(metaBox);
    // ilocs...
    ilocWrite(metaBox, itemLocationVector);
    // then add the "iref"s.
    irefWrite(metaBox);
}

MetaDerivedImageWriter::ReferenceToItemIdMap MetaDerivedImageWriter::createReferenceToItemIdMap() const
{
    ReferenceToItemIdMap referenceToItemIdMap;

    // Add meta master contexts to the map
    const std::vector<std::uint32_t> storeIds = DataServe::getStoreIds();
    for (const auto storeId : storeIds)
    {
        const std::shared_ptr<DataStore> dataStore = DataServe::getStore(storeId);

        if (dataStore->isValueSet("uniq_bsid") &&
            dataStore->isValueSet("capsulation") &&
            dataStore->getValue("capsulation").at(0) == META_ENCAPSULATION)
        {
            const UniqBsid uniqBsid = std::stoi(dataStore->getValue("uniq_bsid").at(0));
            const std::vector<std::string> itemIdStrings = dataStore->getValue("item_indx");
            ItemIdVector itemIds;
            for (const auto& itemId : itemIdStrings)
            {
                itemIds.push_back(std::stoi(itemId));
            }
            referenceToItemIdMap.insert( { uniqBsid, itemIds });
        }
    }

    // Add derived items from this content
    for (const auto& derived : mDerivations)
    {
        referenceToItemIdMap.insert( { derived.second.uniqBsid, derived.second.itemIds });
    }

    return referenceToItemIdMap;
}


void MetaDerivedImageWriter::addItemIdReferences(MetaDerivedImageWriter::DerivationMap& derivations) const
{
    for (auto& derived : derivations)
    {
        for (unsigned int referenceIndex = 0; referenceIndex < derived.second.refsList.size(); ++referenceIndex)
        {
            const UniqBsid uniqBsid = derived.second.refsList.at(referenceIndex);
            for (const auto index : derived.second.indexList.at(referenceIndex))
            {
                derived.second.referenceItemIds.push_back(mReferenceToItemIdMap.at(uniqBsid).at(index - 1));
            }
        }
    }
}

MetaDerivedImageWriter::DerivationMap MetaDerivedImageWriter::processIdenDerivations() const
{
    DerivationMap derivationMap;

    for (const auto& imir : mConfig.imirs)
    {
        DerivationInfo info;
        info.indexList = imir.idxs_list;
        info.refsList = imir.refs_list;
        info.type = "iden";
        info.uniqBsid = imir.uniq_bsid;
        derivationMap.insert( { imir.uniq_bsid, info });
    }
    for (const auto& irot : mConfig.irots)
    {
        DerivationInfo info;
        info.indexList = irot.idxs_list;
        info.refsList = irot.refs_list;
        info.type = "iden";
        info.uniqBsid = irot.uniq_bsid;
        derivationMap.insert( { irot.uniq_bsid, info });
    }
    for (const auto& rloc : mConfig.rlocs)
    {
        DerivationInfo info;
        info.indexList = rloc.idxs_list;
        info.refsList = rloc.refs_list;
        info.type = "iden";
        info.uniqBsid = rloc.uniq_bsid;
        derivationMap.insert( { rloc.uniq_bsid, info });
    }
    for (const auto& clap : mConfig.claps)
    {
        DerivationInfo info;
        info.indexList = clap.idxs_list;
        info.refsList = clap.refs_list;
        info.type = "iden";
        info.uniqBsid = clap.uniq_bsid;
        derivationMap.insert( { clap.uniq_bsid, info });
    }

    // Create item IDs for every 'iden' (Identity transformation) type derived image
    for (auto& entry : derivationMap)
    {
        unsigned int items = 0;
        for (const auto& list : entry.second.indexList)
        {
            items += list.size();
        }
        for (unsigned int i = 0; i < items; ++i)
        {
            const std::uint32_t id = IdSpace::getValue();
            entry.second.itemIds.push_back(id);
        }
    }

    return derivationMap;
}


MetaDerivedImageWriter::DerivationMap MetaDerivedImageWriter::processOtherDerivations(MediaDataBox& mdat,
    ItemLocations& itemLocationVector) const
{
    DerivationMap derivationMap;
    // Handle 'grid' and 'iovl', also create related mdat section here. It will be later set to media writer.
    /** @todo MediaDataBox section creation should probably be refactored. Current separation to meta and media writers does
              not work very well with derived items. */
    unsigned int mdatOffset = 0;
    for (const auto& grid : mConfig.grids)
    {
        DerivationInfo info;
        info.indexList = grid.idxs_list;
        info.refsList = grid.refs_list;
        info.type = GRID_ITEM_TYPE;
        info.uniqBsid = grid.uniq_bsid;
        const ItemId itemId = IdSpace::getValue();
        info.itemIds.push_back(itemId);
        derivationMap.insert( { grid.uniq_bsid, info });

        ImageGrid gridItem;
        gridItem.columnsMinusOne = grid.columns - 1;
        gridItem.rowsMinusOne = grid.rows - 1;
        gridItem.outputHeight = grid.outputHeight;
        gridItem.outputWidth = grid.outputWidth;

        BitStream bitstream;
        writeImageGrid(gridItem, bitstream);
        ItemLocation location;
        location.length = bitstream.getSize();
        location.offset = mdatOffset;
        location.itemId = itemId;
        itemLocationVector.push_back(location);
        mdatOffset += location.length;
        mdat.addData(bitstream.getStorage());
    }
    for (const auto& iovl : mConfig.iovls)
    {
        DerivationInfo info;
        info.indexList = iovl.idxs_list;
        info.refsList = iovl.refs_list;
        info.type = OVERLAY_ITEM_TYPE;
        info.uniqBsid = iovl.uniq_bsid;
        const ItemId itemId = IdSpace::getValue();
        info.itemIds.push_back(itemId);
        derivationMap.insert( { iovl.uniq_bsid, info });

        ImageOverlay iovlItem;
        iovlItem.outputHeight = iovl.outputHeight;
        iovlItem.outputWidth = iovl.outputWidth;

        std::vector<ImageOverlay::Offset> offsets;
        for (const auto& offsetConfig : iovl.offsets)
        {
            ImageOverlay::Offset offset;
            offset.horizontalOffset = offsetConfig.horizontal;
            offset.verticalOffset = offsetConfig.vertical;
            offsets.push_back(offset);
        }
        iovlItem.offsets = offsets;
        iovlItem.canvasFillValueR = iovl.canvasFillValue.at(0);
        iovlItem.canvasFillValueG = iovl.canvasFillValue.at(1);
        iovlItem.canvasFillValueB = iovl.canvasFillValue.at(2);
        iovlItem.canvasFillValueA = iovl.canvasFillValue.at(3);

        BitStream bitstream;
        writeImageOverlay(iovlItem, bitstream);
        ItemLocation location;
        location.length = bitstream.getSize();
        location.offset = mdatOffset;
        location.itemId = itemId;
        itemLocationVector.push_back(location);
        mdatOffset += location.length;
        mdat.addData(bitstream.getStorage());
    }

    return derivationMap;
}

void MetaDerivedImageWriter::iprpWrite(MetaBox* metaBox)
{
    // Create 'ispe' properties for derived items.
    for (const auto& grid : mConfig.grids)
    {
        auto ispe = std::make_shared<ImageSpatialExtentsProperty>();
        ispe->setDisplayWidth(grid.outputWidth);
        ispe->setDisplayHeight(grid.outputHeight);
        metaBox->addProperty(ispe, mDerivations.at(grid.uniq_bsid).itemIds, false);
    }

    for (const auto& iovl : mConfig.iovls)
    {
        auto ispe = std::make_shared<ImageSpatialExtentsProperty>();
        ispe->setDisplayWidth(iovl.outputWidth);
        ispe->setDisplayHeight(iovl.outputHeight);
        metaBox->addProperty(ispe, mDerivations.at(iovl.uniq_bsid).itemIds, true);
    }

    // Add properties for 'iden' items, and link existing ispes to 'iden' items
    for (const auto& imir : mConfig.imirs)
    {
        linkIspeProperties(metaBox, mDerivations.at(imir.uniq_bsid).referenceItemIds, mDerivations.at(imir.uniq_bsid).itemIds);
        auto mirrorProperty = std::make_shared<ImageMirror>();
        mirrorProperty->setHorizontalAxis(imir.horizontalAxis);
        metaBox->addProperty(mirrorProperty, mDerivations.at(imir.uniq_bsid).itemIds, true);
    }

    for (const auto& irot : mConfig.irots)
    {
        linkIspeProperties(metaBox, mDerivations.at(irot.uniq_bsid).referenceItemIds, mDerivations.at(irot.uniq_bsid).itemIds);
        auto rotationProperty = std::make_shared<ImageRotation>();
        rotationProperty->setAngle(irot.angle);
        metaBox->addProperty(rotationProperty, mDerivations.at(irot.uniq_bsid).itemIds, true);
    }

    for (const auto& rloc : mConfig.rlocs)
    {
        linkIspeProperties(metaBox, mDerivations.at(rloc.uniq_bsid).referenceItemIds, mDerivations.at(rloc.uniq_bsid).itemIds);
        auto relativeLocationProperty = std::make_shared<ImageRelativeLocationProperty>();
        relativeLocationProperty->setHorizontalOffset(rloc.horizontal_offset);
        relativeLocationProperty->setVerticalOffset(rloc.vertical_offset);
        metaBox->addProperty(relativeLocationProperty, mDerivations.at(rloc.uniq_bsid).itemIds, true);
    }

    for (const auto& clap : mConfig.claps)
    {
        linkIspeProperties(metaBox, mDerivations.at(clap.uniq_bsid).referenceItemIds, mDerivations.at(clap.uniq_bsid).itemIds);
        auto clapProperty = std::make_shared<CleanAperture>();

        CleanAperture::Fraction value;
        value.numerator = clap.clapHeightN;
        value.denominator = clap.clapHeightD;
        clapProperty->setHeight(value);

        value.numerator = clap.clapWidthN;
        value.denominator = clap.clapWidthD;
        clapProperty->setWidth(value);

        value.numerator = clap.horizOffN;
        value.denominator = clap.horizOffD;
        clapProperty->setHorizOffset(value);

        value.numerator = clap.vertOffN;
        value.denominator = clap.vertOffD;
        clapProperty->setVertOffset(value);

        metaBox->addProperty(clapProperty, mDerivations.at(clap.uniq_bsid).itemIds, true);
    }
}

void MetaDerivedImageWriter::iinfWrite(MetaBox* metaBox)
{
    for (const auto& derived : mDerivations)
    {
        for (const auto itemId : derived.second.itemIds)
        {
            metaBox->addItem(itemId, derived.second.type, "Derived image");

            /// @todo Note: this sort of abuses the DataStore usage conventions, and differs from other writers. Refactor code to avoid this.
            // The reason to do this was that a single content has only one context id for all derived images which may have several uniq_bsids.
            storeValue("uniq_bsid", std::to_string(derived.second.uniqBsid));
            storeValue("item_indx", std::to_string(itemId));
        }
    }
}

void MetaDerivedImageWriter::irefWrite(MetaBox* metaBox)
{
    for (const auto& derivation : mDerivations)
    {
        insertDimgReferences(metaBox, derivation.second);
    }

    // Write base references for pre-derived
    for (const auto& preDerived : mConfig.prederiveds)
    {
        insertBaseReferences(metaBox, preDerived.pre_refs_list, preDerived.pre_idxs_list, preDerived.base_refs_list,
            preDerived.base_idxs_list);
    }
}

void MetaDerivedImageWriter::ilocWrite(MetaBox* metaBox, const ItemLocations& locations)
{
    for (const auto& item : locations)
    {
        metaBox->addIloc(item.itemId, item.offset, item.length, getBaseOffset());
    }
}

void MetaDerivedImageWriter::insertDimgReferences(MetaBox* metaBox, const DerivationInfo& derivation)
{
    // Handle two cases: 1. "iden" derivations which have pairs of reference and derived item IDs.
    //                   2. Derived items like "grid" and "iovl" have several references, but the result is only one item ID.

    const uint32_t referenceCount = derivation.referenceItemIds.size();
    const uint32_t derivedCount = derivation.itemIds.size();

    if (referenceCount == derivedCount)
    {
        for (unsigned int i = 0; i < derivedCount; ++i)
        {
            metaBox->addItemReference("dimg", derivation.itemIds.at(i), derivation.referenceItemIds.at(i));
        }
    }
    else if (derivedCount == 1 && referenceCount > 1)
    {
        for (unsigned int i = 0; i < referenceCount; ++i)
        {
            metaBox->addItemReference("dimg", derivation.itemIds.at(0), derivation.referenceItemIds.at(i));
        }
    }
    else
    {
        throw std::runtime_error("MetaDerivedImageWriter::insertItemReferences invalid parameters");
    }
}

void MetaDerivedImageWriter::insertBaseReferences(MetaBox* metaBox,
    const IsoMediaFile::ReferenceList& preRefsList, const IsoMediaFile::IndexList& preIdxsList,
    const IsoMediaFile::ReferenceList& baseRefsList, const IsoMediaFile::IndexList& baseIdxsList) const
{
    ItemIdVector prederivedItemIds;
    for (unsigned int preRefIndex = 0; preRefIndex < preRefsList.size(); ++preRefIndex)
    {
        const UniqBsid uniqBsid = preRefsList.at(preRefIndex);
        for (const auto index : preIdxsList.at(preRefIndex))
        {
            prederivedItemIds.push_back(mReferenceToItemIdMap.at(uniqBsid).at(index - 1));
        }
    }

    ItemIdVector baseItemIds;
    for (unsigned int baseRefIndex = 0; baseRefIndex < baseRefsList.size(); ++baseRefIndex)
    {
        const UniqBsid uniqBsid = baseRefsList.at(baseRefIndex);
        for (const auto index : baseIdxsList.at(baseRefIndex))
        {
            baseItemIds.push_back(mReferenceToItemIdMap.at(uniqBsid).at(index - 1));
        }
    }

    for (const auto fromId : prederivedItemIds)
    {
        for (const auto toId : baseItemIds)
        {
            metaBox->addItemReference("base", fromId, toId);
        }
    }
}

void MetaDerivedImageWriter::linkIspeProperties(MetaBox* metaBox, const ItemIdVector& fromItems, const ItemIdVector& toItems) const
{
    const ItemPropertiesBox& iprpBox = metaBox->getItemPropertiesBox();

    if (fromItems.size() != toItems.size())
    {
        throw std::runtime_error("MetaDerivedImageWriter::linkIspe Different size to and from item ID vectors.");
    }

    for (unsigned int i = 0; i < fromItems.size(); ++i)
    {
        const std::uint32_t prpIndex = iprpBox.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, fromItems.at(i));
        metaBox->addProperty(prpIndex, { toItems.at(i) }, false);
    }
}
