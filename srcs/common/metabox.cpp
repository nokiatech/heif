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

#include "metabox.hpp"

#include "bitstream.hpp"

#include "heiferrorinfo.h"
#include "ambientviewingenvironmentbox.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "bbox.hpp"
#include "colourinformationbox.hpp"
#include "contentcolourvolumebox.hpp"
#include "contentlightlevelbox.hpp"
#include "decoderconfigrecord.hpp"
#include "decoderconfigurationbox.hpp"
#include "entitytogroupbox.hpp"
#include "freespacebox.hpp"
#include "hevcconfigurationbox.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "imagemirror.hpp"
#include "imagerotation.hpp"
#include "imagespatialextentsproperty.hpp"
#include "jpegconfigurationbox.hpp"
#include "log.hpp"
#include "masteringdisplaycolourvolumebox.hpp"
#include "nominaldiffusewhitebox.hpp"
#include "pixelinformationproperty.hpp"
#include "rawpropertybox.hpp"
#include "referenceviewingenvironmentbox.hpp"
#include "vvcconfigurationbox.hpp"
#include "vvcdecoderconfigrecord.hpp"
#include <vector>

MetaBox::MetaBox()
    : FullBox("meta", 0, 0)
    , 
     mHandlerBox()
    , mPrimaryItemBox()
    , mDataInformationBox()
    , mItemLocationBox()
    , mItemProtectionBox()
    , mItemInfoBox()
    , mItemReferenceBox()
    , mItemDataBox()
    , mItemPropertiesBox()
    , mGroupsListBox()
{
}

FourCCInt MetaBox::getHandlerType() const
{
    return mHandlerBox.getHandlerType();
}

void MetaBox::setHandlerType(FourCCInt handler)
{
    mHandlerBox.setHandlerType(handler);
}

const PrimaryItemBox& MetaBox::getPrimaryItemBox() const
{
    return mPrimaryItemBox;
}

void MetaBox::setPrimaryItem(const std::uint32_t itemId)
{
    mPrimaryItemBox.setItemId(itemId);
}

const ItemInfoBox& MetaBox::getItemInfoBox() const
{
    return mItemInfoBox;
}

ItemInfoBox& MetaBox::getItemInfoBox()
{
    return mItemInfoBox;
}

void MetaBox::setItemInfoBox(const ItemInfoBox& itemInfoBox)
{
    mItemInfoBox = itemInfoBox;
}

const ItemLocationBox& MetaBox::getItemLocationBox() const
{
    return mItemLocationBox;
}

const ItemReferenceBox& MetaBox::getItemReferenceBox() const
{
    return mItemReferenceBox;
}

const ItemPropertiesBox& MetaBox::getItemPropertiesBox() const
{
    return mItemPropertiesBox;
}

std::uint16_t MetaBox::addProperty(const std::shared_ptr<Box>& box, const Vector<std::uint32_t>& itemIds, const bool essential)
{
    return mItemPropertiesBox.addProperty(box, itemIds, essential);
}

void MetaBox::addProperty(const std::uint16_t index, const Vector<std::uint32_t>& itemIds, const bool essential)
{
    mItemPropertiesBox.associateProperty(index, itemIds, essential);
}

const GroupsListBox& MetaBox::getGroupsListBox() const
{
    return mGroupsListBox;
}

void MetaBox::addEntityGrouping(const FourCCInt type, const std::uint32_t groupId, const Vector<std::uint32_t>& itemIds)
{
    EntityToGroupBox entityToGroupBox(type);
    entityToGroupBox.setEntityIds(itemIds);
    entityToGroupBox.setGroupId(groupId);
    mGroupsListBox.addEntityToGroupBox(entityToGroupBox);
}

const DataInformationBox& MetaBox::getDataInformationBox() const
{
    return mDataInformationBox;
}

void MetaBox::addItemReference(const FourCCInt type, const std::uint32_t fromId, const std::uint32_t toId)
{
    mItemReferenceBox.add(type, fromId, toId);
}

void MetaBox::addIloc(const std::uint32_t itemId,
                      const std::uint64_t offset,
                      const std::uint64_t length,
                      const std::uint64_t baseOffset)
{
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = length;

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.setBaseOffset(baseOffset);
    itemLocation.addExtent(locationExtent);

    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItem(const std::uint32_t itemId, FourCCInt type, const String& name, const bool hidden)
{
    ItemInfoEntry infeBox;
    infeBox.setVersion(2);
    infeBox.setItemType(type);
    infeBox.setItemID(itemId);
    infeBox.setItemName(name);

    if (hidden)
    {
        // Set (flags & 1) == 1 to indicate the item is hidden.
        infeBox.setFlags(1);
    }

    mItemInfoBox.addItemInfoEntry(infeBox);
}

void MetaBox::addItem(const ItemInfoEntry& itemInfoEntry)
{
    mItemInfoBox.addItemInfoEntry(itemInfoEntry);
}

void MetaBox::addMdatItem(const std::uint32_t itemId,
                          FourCCInt type,
                          const String& name,
                          const std::uint64_t baseOffset)
{
    addItem(itemId, type, name);

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.setBaseOffset(baseOffset);
    itemLocation.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItemExtent(const std::uint32_t itemId, const std::uint64_t offset, const std::uint64_t length)
{
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = length;
    mItemLocationBox.addExtent(itemId, locationExtent);
}

void MetaBox::addIdatItem(const std::uint32_t itemId, FourCCInt type, const String& name, const Vector<uint8_t>& data)
{
    const uint64_t offset = mItemDataBox.addData(data);
    addItem(itemId, type, name);
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = data.size();

    ItemLocation itemLocation;
    itemLocation.setItemID(itemId);
    itemLocation.addExtent(locationExtent);
    itemLocation.setConstructionMethod(ItemLocation::ConstructionMethod::IDAT_OFFSET);
    mItemLocationBox.addLocation(itemLocation);
}

void MetaBox::addItemIdatExtent(const std::uint32_t itemId, const Vector<uint8_t>& data)
{
    const uint64_t offset = mItemDataBox.addData(data);
    ItemLocationExtent locationExtent;
    locationExtent.mExtentOffset = offset;
    locationExtent.mExtentLength = data.size();
    mItemLocationBox.addExtent(itemId, locationExtent);
}

void MetaBox::setItemFileOffsetBase(const std::uint64_t baseOffset)
{
    auto& itemLocations = mItemLocationBox.getItemLocations();
    for (auto& iloc : itemLocations)
    {
        if (iloc.getConstructionMethod() == ItemLocation::ConstructionMethod::FILE_OFFSET)
        {
            iloc.setBaseOffset(baseOffset);
        }
    }
}

const ItemDataBox& MetaBox::getItemDataBox() const
{
    return mItemDataBox;
}

const ProtectionSchemeInfoBox& MetaBox::getProtectionSchemeInfoBox(std::uint16_t index) const
{
    return mItemProtectionBox.getEntry(index);
}

void MetaBox::setImageHidden(const std::uint32_t itemId, const bool isHidden)
{
    auto& infe = mItemInfoBox.getItemById(itemId);
    // bit 0 indicates the image is hidden
    if (isHidden)
    {
        infe.setFlags(infe.getFlags() | 1);
    }
    else
    {
        infe.setFlags(infe.getFlags() & ~1u);
    }
}

const DecoderConfigurationBox* MetaBox::getConfigBox(uint32_t itemid) const
{
    uint32_t confidx = 0;
    bool found = false;

    for(auto property : mItemPropertiesBox.getItemProperties(itemid)){
        const Box* box = mItemPropertiesBox.getPropertyByIndex(property.index);
        if (box == nullptr) { continue; }
        const FourCCInt& type = box->getType();
        if( type == "vvcC" || type == "hvcC" ||
            type == "avcC" || type == "jpgC" ){
            confidx = property.index; 
            found = true;
            break;
        }
    }

    if (!found) {
        return nullptr;  // No config found
    }

    const DecoderConfigurationBox* configbox = 
        dynamic_cast<const DecoderConfigurationBox*>(
            mItemPropertiesBox.getPropertyByIndex(confidx)
        );
    
    return configbox;
}


const FourCCInt MetaBox::getConfigType(uint32_t itemid) const
{
    for(auto property : mItemPropertiesBox.getItemProperties(itemid)){
        const Box* box = mItemPropertiesBox.getPropertyByIndex(property.index);
        if (box == nullptr) { continue; }
        const FourCCInt& type = box->getType();
        if( type == "vvcC" || type == "hvcC" ||
            type == "avcC" || type == "jpgC" ){
            return type;
        }
    }

    return FourCCInt();
}


void MetaBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    mHandlerBox.writeBox(bitstr);
    mPrimaryItemBox.writeBox(bitstr);
    // The optional 'dinf' is currently not used, so it is not written:
    // mDataInformationBox.writeBox(bitstr);
    mItemLocationBox.writeBox(bitstr);
    mItemProtectionBox.writeBox(bitstr);
    mItemInfoBox.writeBox(bitstr);
    // Not writing optional box IPMPControlBox
    mItemReferenceBox.writeBox(bitstr);
    mItemDataBox.writeBox(bitstr);
    mItemPropertiesBox.writeBox(bitstr);
    mGroupsListBox.writeBox(bitstr);

    updateSize(bitstr);
}

void MetaBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "hdlr")
        {
            mHandlerBox.parseBox(subBitstr);
        }
        else if (boxType == "pitm")
        {
            mPrimaryItemBox.parseBox(subBitstr);
        }
        else if (boxType == "iloc")
        {
            mItemLocationBox.parseBox(subBitstr);
        }
        else if (boxType == "iinf")
        {
            mItemInfoBox.parseBox(subBitstr);
        }
        else if (boxType == "iref")
        {
            mItemReferenceBox.parseBox(subBitstr);
        }
        else if (boxType == "iprp")
        {
            mItemPropertiesBox.parseBox(subBitstr);
        }
        else if (boxType == "grpl")
        {
            mGroupsListBox.parseBox(subBitstr);
        }
        else if (boxType == "dinf")
        {
            mDataInformationBox.parseBox(subBitstr);
        }
        else if (boxType == "idat")
        {
            mItemDataBox.parseBox(subBitstr);
        }
        else if (boxType == "ipro")
        {
            mItemProtectionBox.parseBox(subBitstr);
        }
        // unsupported boxes are skipped
    }
}

HEIF::ErrorCode MetaBox::writeMiniBox(ISOBMFF::BitStream& output){

    ///@brief A placeholder bitstream to contain the written mini.
    ///       Written to output only when no errors are raised at any point in this method.
    ISOBMFF::BitStream bitstr;

    //Get Item ids for every item going to be in the mini (main item, alpha item, tonemap item)
    ///@todo this should actually check the case of tmap, where primary is not an actual image item, but a dimg.
    ///     in such scenario, the base image should be "the" image of minibox along with maps.
    const uint32_t PRIMARY_ITEM_ID = mPrimaryItemBox.getItemId();
    const FourCCInt PRIMARY_ITEM_TYPE = mItemInfoBox.getItemById(PRIMARY_ITEM_ID).getItemType();

    const auto auxl = mItemReferenceBox.getReferencesOfType("auxl");

    ///@brief Check that user is not trying to write mini with multiple primary items. (Unsupported, max is prim + alpha + gainmap item)
    ///@todo this could be more robust, checking typings etc.
    Map<FourCCInt,uint8_t> itemCounter;
    for(const auto& iinf : mItemInfoBox.getItemInfoList()){
        if(itemCounter.find(iinf.getItemType()) != itemCounter.end()){
            //Multiple same type items is allowed in case of main+alpha+tmap
            if(itemCounter.at(iinf.getItemType()) >= 3){
                return HEIF_FAIL(HEIF::ErrorCode::TMI,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: more than 3 items of the same type (max is prim + alpha + gainmap)");
            }else{
                itemCounter.at(iinf.getItemType())++;
            }
        }else{
            itemCounter.insert({iinf.getItemType(),static_cast<uint8_t>(1)});
        }
    }


    //Check for existance of alpha item
    const bool alpha_flag = auxl.size() != 0;

    //mini is 1:1 for main + alpha
    if (alpha_flag && auxl.size() != 1) {
        return HEIF_FAIL(HEIF::ErrorCode::INVALID_REFERENCE_COUNT,
                        HEIF::ErrorOrigin::Writer_Mini,
                        "writeMiniBox: mini supports exactly one auxl reference for alpha");
    }

    const uint32_t ALPHA_ITEM_ID = [&]() -> uint32_t {
        if (!alpha_flag) return 0;
        for (const auto& item : auxl) {
            for (const auto& idx : item.getToItemIds()) {
                if (idx == PRIMARY_ITEM_ID) {
                    return item.getFromItemID();
                }
            }
        }
        return 0;
    }();


    //Check for premultiplication of alpha in main item
    const bool alpha_premultiplied_flag = [&]() -> bool {
        if (!alpha_flag) return false;
        for (const auto& ref : mItemReferenceBox.getReferencesOfType("prem")) {
            if (ref.getFromItemID() != PRIMARY_ITEM_ID) continue;
            for (const auto& id : ref.getToItemIds()) {
                if (id == ALPHA_ITEM_ID) return true;
            }
        }
        return false;
    }();


    ///@todo recognize natively HDR item? BPC of main item pixi?
    ///@todo scenario where there is no tmap item?? there is only 2 items with same infetype
    ///@todo scenario with only icc tonemapping?
    const auto* tmap = mItemInfoBox.findItemWithType("tmap");
    const bool hdr_flag = tmap != nullptr;

    const uint32_t TONEMAP_ITEM_ID = (tmap != nullptr) ? tmap->getItemID() : UINT32_MAX;

    // Validate dimg reference count. Tmap item references base + gainmap item
    if (hdr_flag) {
        for (const auto& ref : mItemReferenceBox.getReferencesOfType("dimg")) {
            if (ref.getFromItemID() == TONEMAP_ITEM_ID && ref.getToItemIds().size() != 2) {
                return HEIF_FAIL(HEIF::ErrorCode::INVALID_REFERENCE_COUNT,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: tmap dimg reference must point to exactly 2 items (base + gainmap)");
            }
        }
    }

    const uint32_t GAINMAP_ITEM_ID = [&]() -> uint32_t {
        if (!hdr_flag) return UINT32_MAX;
        for (const auto& ref : mItemReferenceBox.getReferencesOfType("dimg")) {
            if (ref.getFromItemID() != TONEMAP_ITEM_ID) continue;
            const auto& toIDs = ref.getToItemIds();
            if (toIDs.at(0) == PRIMARY_ITEM_ID) {
                return toIDs.at(1);   // gain map is the second input image
            }
        }
        return UINT32_MAX;
    }();

    const bool gainmap_flag = (GAINMAP_ITEM_ID != UINT32_MAX);


    //Get all required properties to write minibox
    const PixelInformationProperty* pixi = nullptr;
    const ColourInformationBox* colr = nullptr;
    const ImageRotation* irot = nullptr;
    const ImageMirror* imir = nullptr;
    const ImageSpatialExtentsProperty* ispe = nullptr;

    for(auto prop : mItemPropertiesBox.getItemProperties(PRIMARY_ITEM_ID)){
        switch(prop.type){
            case ItemPropertiesBox::PropertyType::PIXI:
                pixi = dynamic_cast<const PixelInformationProperty*>(mItemPropertiesBox.getPropertyByIndex(prop.index)); break;
            case ItemPropertiesBox::PropertyType::COLR:
                colr = dynamic_cast<const ColourInformationBox*>(mItemPropertiesBox.getPropertyByIndex(prop.index)); break;
            case ItemPropertiesBox::PropertyType::IROT:
                irot = dynamic_cast<const ImageRotation*>(mItemPropertiesBox.getPropertyByIndex(prop.index)); break;
            case ItemPropertiesBox::PropertyType::IMIR:
                imir = dynamic_cast<const ImageMirror*>(mItemPropertiesBox.getPropertyByIndex(prop.index)); break;
            case ItemPropertiesBox::PropertyType::ISPE:
                ispe = dynamic_cast<const ImageSpatialExtentsProperty*>(mItemPropertiesBox.getPropertyByIndex(prop.index)); break;
            default:
                //continue, maybe check for unsupported properties?
                break;
        }
    }

    // pixi is optional for VVC (float_flag shall be 0 per spec); mandatory for other codecs.
    if (pixi == nullptr && mItemInfoBox.getItemById(PRIMARY_ITEM_ID).getItemType() != "vvc1"){
        logError() << "Error: Missing PixelInformationProperty for primary item." << '\n';
        return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                        HEIF::ErrorOrigin::Writer_Mini,
                        "writeMiniBox: PixelInformationProperty required for non-VVC primary item");
    }
    if (ispe == nullptr){
        logError() << "Error: Missing ImageSpatialExtentsProperty for primary item." << '\n';
        return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                        HEIF::ErrorOrigin::Writer_Mini,
                        "writeMiniBox: ImageSpatialExtentsProperty required for primary item");
    }

    //These can be defaulted to
    uint32_t imageRot = (irot != nullptr) ? irot->getAngle() : 0u;

    const bool hasMir = (imir != nullptr);
    const bool mirHorizontalAxis = hasMir && imir->getHorizontalAxis();


    //Get decoder config box for primary item
    const DecoderConfigurationBox* confbox = this->getConfigBox(PRIMARY_ITEM_ID);

    //If no config is found, return error to avoid broken file output
    if(confbox == nullptr){
        return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                        HEIF::ErrorOrigin::Writer_Mini,
                        "writeMiniBox: no DecoderConfigurationBox found for primary item");
    }



    ///####
    ///START: Writing sequence
    ///####



    bitstr.writeBits(0,2);  //bit(2) version = 0


    bool explicit_codec_types_flag = !(PRIMARY_ITEM_TYPE == "vvc1" ||
                                       PRIMARY_ITEM_TYPE == "hvc1" ||
                                       PRIMARY_ITEM_TYPE == "avc1" ||
                                       PRIMARY_ITEM_TYPE == "jpg1");

    bitstr.writeBits(explicit_codec_types_flag,1);  //bit(1) explicit_codec_types_flag


    bool float_flag = pixi == nullptr ? false : pixi->getComponentFormat(2);
    bitstr.writeBits(float_flag,1); //bit(1) = float_flag;
    bitstr.writeBits(colr == nullptr ? false : colr->getFullRangeFlag(),1);   //bit(1) full_range_flag


    bitstr.writeBits(alpha_flag, 1); //bit(1) alpha_flag


    const bool icc_flag = mItemInfoBox.findItemWithType("rICC") != nullptr;
    const bool explicit_cicp_flag = colr == nullptr ? false : colr->getExplicitCICPFlag(icc_flag);
    bitstr.writeBits(explicit_cicp_flag, 1); //bit(1) explicit_cicp_flag


    ///@todo check for existance of SDR2HDR gainmaps or item being hdr itself
    ///     Current "4cc" might not retrieve the correct flag value
    bitstr.writeBits(hdr_flag, 1); //bit(1) hdr_flag

    bitstr.writeBits(icc_flag, 1);  //bit(1) icc_flag

    bool exif_flag = mItemInfoBox.findItemWithType("Exif") != nullptr, exif_compressed_flag = false;
    if(!exif_flag){
        exif_flag = mItemInfoBox.findItemWithType("dExf") != nullptr; exif_compressed_flag = exif_flag;
    }
    bitstr.writeBits(exif_flag, 1);  //bit(1) exif_flag

    bool xmp_flag = mItemInfoBox.findItemWithType("mime") != nullptr;
    bitstr.writeBits(xmp_flag, 1);  //bit(1) xmp_flag


    uint8_t chroma_subsampling = pixi == nullptr ? 1 :
                            pixi->getNumChannels() == 1 ? 0 : //YUV400
                            pixi->getSubSamplingType(3) == 2 ? 1 : //YUV420
                            pixi->getSubSamplingType(4) == 1 ? 2 : //YUV422
                            3;  //YUV444

    bitstr.writeBits(chroma_subsampling,2); //bit(2) chroma_subsampling


    // Map (irot angle, imir axis) -> Exif orientation per JEITA CP-3451.
    // HEIF applies irot first, then imir (mini property slots #9, #10).
    const uint8_t orientation = [&]() -> uint8_t {
        if (mirHorizontalAxis) {
            // Mirror about horizontal axis = vertical flip
            switch (imageRot) {
                case 0:   return 4;  // Exif 4: flip vertical
                case 90:  return 7;  // Exif 7: transverse
                case 180: return 2;  // Exif 2: rot180+flipV = flipH
                case 270: return 5;  // Exif 5: transpose
            }
        } else if (hasMir) {
            // Mirror about vertical axis = horizontal flip
            switch (imageRot) {
                case 0:   return 2;  // Exif 2: flip horizontal
                case 90:  return 5;  // Exif 5: rot90+flipH = transpose
                case 180: return 4;  // Exif 4: rot180+flipH = flipV
                case 270: return 7;  // Exif 7: rot270+flipH = transverse
            }
        } else {
            switch (imageRot) {
                case 0:   return 1;  // Exif 1: normal
                case 90:  return 6;  // Exif 6: rotate 90 CW
                case 180: return 3;  // Exif 3: rotate 180
                case 270: return 8;  // Exif 8: rotate 270 CW
            }
        }
        return 1;
    }();
    bitstr.writeBits(orientation - 1,3);  //bit(3) orientation_minus1


    uint32_t height_minus1 =ispe->getDisplayHeight() -1;
    uint32_t width_minus1 = ispe->getDisplayWidth() -1;

    bool large_dimensions_flag = width_minus1 > 63 ? true : height_minus1 > 63 ? true : false;

    bitstr.writeBits(large_dimensions_flag,1);  //bit(1) large_dimensions_flag
    bitstr.writeBits(width_minus1,large_dimensions_flag ? 15 : 7); //width_minus1
    bitstr.writeBits(height_minus1,large_dimensions_flag ? 15 : 7); //height_minus1


    //Per spec: if chroma is centered, flag is to be false (inverse bool value)
    std::pair<bool,bool> subsamp = pixi == nullptr ? std::pair<bool,bool>(false,false) : pixi->getChromaCenterdness(4);
    if(chroma_subsampling == 1 || chroma_subsampling == 2){
        bitstr.writeBits(!subsamp.first,1);  //bit(1) chroma_is_horizontally_centered
    }
    if(chroma_subsampling == 1){
        bitstr.writeBits(!subsamp.second,1);  //bit(1) chroma_is_vertically_centered
    }


    if (float_flag){
        if(pixi==nullptr){
            logError() << "Error: Missing PixelInformationProperty for primary item with float bpc." << '\n';
            return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                            HEIF::ErrorOrigin::Writer_Mini,
                            "writeMiniBox: PixelInformationProperty required for float bit-depth primary item");
        }
        auto bpc = pixi->getBitsPerChannels().at(1);
        uint8_t bit_depth_log2_minus4 = 0;
        if(bpc == 16){bit_depth_log2_minus4 = 0;}
        else if(bpc == 32){bit_depth_log2_minus4 = 1;}
        else if(bpc == 64){bit_depth_log2_minus4 = 2;}
        bitstr.writeBits(bit_depth_log2_minus4,2);  ///bit(2) bit_depth_log2_minus4;
    }else {
        bool high_bit_depth_flag = pixi == nullptr ? false : pixi->getBitsPerChannels().at(0) > 8 ? true : false; //bit(1) high_bit_depth_flag;
        bitstr.writeBits(high_bit_depth_flag,1);
        if (high_bit_depth_flag){
            bitstr.writeBits(pixi->getBitsPerChannels().at(0) - 9, 3);  //bit(3) bit_depth_minus9
        }
    }


    if(alpha_flag){
        bitstr.writeBits(alpha_premultiplied_flag,1);  //bit(1) alpha_is_premultiplied
    }


    if(explicit_cicp_flag){
        bitstr.writeBits(colr->getColourPrimaries(),8);  //bit(8) colour_primaries
        bitstr.writeBits(colr->getTransferCharacteristics(),8);  //bit(8) transfer_characteristics
        bitstr.writeBits(colr->getMatrixCoefficients(),8);   //bit(8) matrix_coefficients
    }//else: not carried in bitstream


    if(explicit_codec_types_flag){
        bitstr.writeBits(mItemInfoBox.getItemById(PRIMARY_ITEM_ID).getItemType().getUInt32(),32);   //bit(32) infe_type
        bitstr.writeBits(this->getConfigType(PRIMARY_ITEM_ID).getUInt32(),32);   //bit(32) codec_config_type
    }//else: not carried in bitstream


    bool tmap_icc_flag = false; const ColourInformationBox* tmap_colr = nullptr;

    if(hdr_flag){
        bitstr.writeBits(gainmap_flag,1);   //bit(1) gainmap_flag
        if(gainmap_flag){

            const ImageSpatialExtentsProperty* gainmap_ispe = dynamic_cast<const ImageSpatialExtentsProperty*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE,GAINMAP_ITEM_ID) - 1));
            if (gainmap_ispe == nullptr)
            {
                logError() << "Error: Missing ImageSpatialExtentsProperty for gainmap item." << '\n';
                return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: ImageSpatialExtentsProperty required for gainmap item");
            }
            bool gainmap_dimensions_same_as_main_item_flag = false;
            if(gainmap_ispe->getDisplayHeight()-1 == height_minus1 &&
                gainmap_ispe->getDisplayWidth()-1 == width_minus1){
                    gainmap_dimensions_same_as_main_item_flag = true;
            }
            bitstr.writeBits(gainmap_dimensions_same_as_main_item_flag,1); //bit(1) gainmap_dimensions_same_as_main_item_flag


            if(!gainmap_dimensions_same_as_main_item_flag){
                bitstr.writeBits(gainmap_ispe->getDisplayWidth()-1,large_dimensions_flag ? 15 : 7); //gainmap_width_minus1
                bitstr.writeBits(gainmap_ispe->getDisplayHeight()-1,large_dimensions_flag ? 15 : 7);    //gainmap_height_minus1
            }

            
            const ColourInformationBox* gainmap_colr = dynamic_cast<const ColourInformationBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::COLR,GAINMAP_ITEM_ID) - 1));
            if (gainmap_colr == nullptr)
            {
                logError() << "Error: Missing ColourInformationBox for gainmap item." << '\n';
                return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: ColourInformationBox required for gainmap item");
            }
            bitstr.writeBits(gainmap_colr->getMatrixCoefficients(),8);  //bit(8) gainmap_matrix_coefficients
            bitstr.writeBits(gainmap_colr->getFullRangeFlag(),1);   //bit(1) gainmap_full_range_flag


            const PixelInformationProperty* gainmap_pixi = dynamic_cast<const PixelInformationProperty*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::PIXI,GAINMAP_ITEM_ID) - 1));
            if (gainmap_pixi == nullptr)
            {
                logError() << "Error: Missing PixelInformationProperty for gainmap item." << '\n';
                return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: PixelInformationProperty required for gainmap item");
            }
            uint8_t gainmap_chroma_subsampling = gainmap_pixi->getNumChannels() == 1 ? 0 :
                            gainmap_pixi->getSubSamplingType(3) == 2 ? 1 :
                            gainmap_pixi->getSubSamplingType(4) == 1 ? 2 :
                            3;
            bitstr.writeBits(gainmap_chroma_subsampling,2); //bit(2) gainmap_chroma_subsampling
            std::pair<bool,bool> gainmap_subsamp = gainmap_pixi->getChromaCenterdness(3);
            if(gainmap_chroma_subsampling == 1 || gainmap_chroma_subsampling == 2){
                bitstr.writeBits(!gainmap_subsamp.first,1);  //bit(1) gainmap_chroma_is_horizontally_centered
            }
            if(gainmap_chroma_subsampling == 1){
                bitstr.writeBits(!gainmap_subsamp.second,1);  //bit(1) gainmap_chroma_is_vertically_centered
            }


            bool gainmap_float_flag = gainmap_pixi->getComponentFormat(2);
            bitstr.writeBits(gainmap_float_flag,1); //bit(1) gainmap_float_flag
            if (gainmap_float_flag){
                auto gainmap_bpc = gainmap_pixi->getBitsPerChannels().at(1);
                uint8_t gainmap_bit_depth_log2_minus4 = 0;
                if(gainmap_bpc == 16){gainmap_bit_depth_log2_minus4 = 0;}
                else if(gainmap_bpc == 32){gainmap_bit_depth_log2_minus4 = 1;}
                else if(gainmap_bpc == 64){gainmap_bit_depth_log2_minus4 = 2;}
                bitstr.writeBits(gainmap_bit_depth_log2_minus4,2);  ///bit(2) gainmap_bit_depth_log2_minus4;
            }else {
                bool gainmap_high_bit_depth_flag = gainmap_pixi->getBitsPerChannels().at(0) > 8 ? true : false;
                bitstr.writeBits(gainmap_high_bit_depth_flag,1); //bit(1) gainmap_high_bit_depth_flag
                if (gainmap_high_bit_depth_flag){
                    bitstr.writeBits(gainmap_pixi->getBitsPerChannels().at(0) - 9, 3);  //bit(3) gainmap_bit_depth_minus9
                }
            }


            tmap_colr = nullptr;
            for (auto prop : mItemPropertiesBox.getItemProperties(TONEMAP_ITEM_ID)) {
                if (prop.type == ItemPropertiesBox::PropertyType::COLR) {
                    const auto* candidate = dynamic_cast<const ColourInformationBox*>(
                        mItemPropertiesBox.getPropertyByIndex(prop.index));
                    if (candidate && candidate->getColourType() == "prof") {
                        tmap_colr = candidate;
                        break;
                    }
                }
            }
            tmap_icc_flag = tmap_colr != nullptr;
            bitstr.writeBits(tmap_icc_flag,1);   //bit(1) tmap_icc_flag

            const ColourInformationBox* tmap_nclx_colr = nullptr;
            for (auto prop : mItemPropertiesBox.getItemProperties(TONEMAP_ITEM_ID)) {
                if (prop.type == ItemPropertiesBox::PropertyType::COLR) {
                    const auto* candidate = dynamic_cast<const ColourInformationBox*>(
                        mItemPropertiesBox.getPropertyByIndex(prop.index));
                    if (candidate && candidate->getColourType() == "nclx") {
                        tmap_nclx_colr = candidate;
                        break;
                    }
                }
            }
            if(tmap_nclx_colr == nullptr && !tmap_icc_flag){
                logError() << "Error: Missing ColourInformationBox for tonemap item." << '\n';
                return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                                HEIF::ErrorOrigin::Writer_Mini,
                                "writeMiniBox: ColourInformationBox required for tonemap item");
            }

            const ColourInformationBox* tmap_cicp_source = tmap_nclx_colr ? tmap_nclx_colr : tmap_colr;
            const bool tmap_explicit_cicp_flag = tmap_cicp_source->getExplicitCICPFlag();
            bitstr.writeBits(tmap_explicit_cicp_flag,1);
            if(tmap_explicit_cicp_flag){
                bitstr.writeBits(tmap_cicp_source->getColourPrimaries(),8);    //bit(8) tmap_colour_primaries
                bitstr.writeBits(tmap_cicp_source->getTransferCharacteristics(),8);    //bit(8) tmap_transfer_characteristics;
                bitstr.writeBits(tmap_cicp_source->getMatrixCoefficients(),8); //bit(8) tmap_matrix_coefficients;
                bitstr.writeBits(tmap_cicp_source->getFullRangeFlag(),1);  //bit(1) tmap_full_range_flag
            }//else: not carried in the bitstream


            const ContentLightLevelBox* clli = dynamic_cast<const ContentLightLevelBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::CLLI,PRIMARY_ITEM_ID) - 1));
            const bool clli_flag = clli != nullptr;
            const MasteringDisplayColourVolumeBox* mdcv = dynamic_cast<const MasteringDisplayColourVolumeBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::MDCV,PRIMARY_ITEM_ID) - 1));
            const bool mdcv_flag = mdcv != nullptr;
            const ContentColourVolumeBox* cclv = dynamic_cast<const ContentColourVolumeBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::CCLV,PRIMARY_ITEM_ID) - 1));
            const bool cclv_flag = cclv != nullptr;
            const AmbientViewingEnvironmentBox* amve = dynamic_cast<const AmbientViewingEnvironmentBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::AMVE,PRIMARY_ITEM_ID) - 1));
            const bool amve_flag = amve != nullptr;
            const ReferenceViewingEnvironmentBox* reve = dynamic_cast<const ReferenceViewingEnvironmentBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::REVE,PRIMARY_ITEM_ID) - 1));
            const bool reve_flag = reve != nullptr;
            const NominalDiffuseWhiteBox* ndwt = dynamic_cast<const NominalDiffuseWhiteBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::NDWT,PRIMARY_ITEM_ID) - 1));
            const bool ndwt_flag = ndwt != nullptr;

            bitstr.writeBits(clli_flag,1);  //bit(1) clli_flag
            bitstr.writeBits(mdcv_flag,1);  //bit(1) mdcv_flag
            bitstr.writeBits(cclv_flag,1);  //bit(1) cclv_flag
            bitstr.writeBits(amve_flag,1);  //bit(1) amve_flag
            bitstr.writeBits(reve_flag,1);  //bit(1) reve_flag
            bitstr.writeBits(ndwt_flag,1);  //bit(1) ndwt_flag

            if(clli_flag){ clli->writeBox(bitstr); }
            if(mdcv_flag){ mdcv->writeBox(bitstr); }
            if(cclv_flag){ cclv->writeBox(bitstr); }
            if(amve_flag){ amve->writeBox(bitstr); }
            if(reve_flag){ reve->writeBox(bitstr); }
            if(ndwt_flag){ ndwt->writeBox(bitstr); }

            // Defensive guard -- the outer block already verifies gainmap_flag,
            // but this inner re-test stays so a future refactor moving the inner
            // block out of the outer one doesn't silently dereference unset state.
            // NOLINTBEGIN(bugprone-redundant-branch-condition)
            if(gainmap_flag){
            // NOLINTEND(bugprone-redundant-branch-condition)
                const ContentLightLevelBox* tmap_clli = dynamic_cast<const ContentLightLevelBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::CLLI,TONEMAP_ITEM_ID) - 1));
                const bool tmap_clli_flag = tmap_clli != nullptr;
                const MasteringDisplayColourVolumeBox* tmap_mdcv = dynamic_cast<const MasteringDisplayColourVolumeBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::MDCV,TONEMAP_ITEM_ID) - 1));
                const bool tmap_mdcv_flag = tmap_mdcv != nullptr;
                const ContentColourVolumeBox* tmap_cclv = dynamic_cast<const ContentColourVolumeBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::CCLV,TONEMAP_ITEM_ID) - 1));
                const bool tmap_cclv_flag = tmap_cclv != nullptr;
                const AmbientViewingEnvironmentBox* tmap_amve = dynamic_cast<const AmbientViewingEnvironmentBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::AMVE,TONEMAP_ITEM_ID) - 1));
                const bool tmap_amve_flag = tmap_amve != nullptr;
                const ReferenceViewingEnvironmentBox* tmap_reve = dynamic_cast<const ReferenceViewingEnvironmentBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::REVE,TONEMAP_ITEM_ID) - 1));
                const bool tmap_reve_flag = tmap_reve != nullptr;
                const NominalDiffuseWhiteBox* tmap_ndwt = dynamic_cast<const NominalDiffuseWhiteBox*>(mItemPropertiesBox.getPropertyByIndex(mItemPropertiesBox.findPropertyIndex(ItemPropertiesBox::PropertyType::NDWT,TONEMAP_ITEM_ID) - 1));
                const bool tmap_ndwt_flag = tmap_ndwt != nullptr;

                bitstr.writeBits(tmap_clli_flag,1); //bit(1) tmap_clli_flag
                bitstr.writeBits(tmap_mdcv_flag,1); //bit(1) tmap_mdcv_flag
                bitstr.writeBits(tmap_cclv_flag,1); //bit(1) tmap_cclv_flag
                bitstr.writeBits(tmap_amve_flag,1); //bit(1) tmap_amve_flag
                bitstr.writeBits(tmap_reve_flag,1); //bit(1) tmap_reve_flag
                bitstr.writeBits(tmap_ndwt_flag,1); //bit(1) tmap_ndwt_flag

                if(tmap_clli_flag){ tmap_clli->writeBox(bitstr); }
                if(tmap_mdcv_flag){ tmap_mdcv->writeBox(bitstr); }
                if(tmap_cclv_flag){ tmap_cclv->writeBox(bitstr); }
                if(tmap_amve_flag){ tmap_amve->writeBox(bitstr); }
                if(tmap_reve_flag){ tmap_reve->writeBox(bitstr); }
                if(tmap_ndwt_flag){ tmap_ndwt->writeBox(bitstr); }
            }
        }
    }


    bool large_metadata_flag = false;
    if(icc_flag || xmp_flag || exif_flag ||(hdr_flag && gainmap_flag)){
        for(const auto& item : mItemLocationBox.getItemLocations()){
            if(item.getItemSize() > (1<<10)){large_metadata_flag = true;break;}
        }
        bitstr.writeBits(large_metadata_flag,1);    //bit(1) large_metadata_flag
    }
    

    ISOBMFF::BitStream conf_bitstr;
    //writeCompactDecConfigRecord() defaults to writeDecConfigRecord() if not supported
    confbox->getConfiguration().writeCompactDecConfigRecord(conf_bitstr);
    bool large_codec_config_flag = conf_bitstr.getSize() > (1<<3);
    bitstr.writeBits(large_codec_config_flag,1);    //bit(1) large_codec_config_flag


    bool large_item_data_flag = false;
    for(const auto& item : mItemLocationBox.getItemLocations()){
        const uint64_t& itemsize = item.getItemSize();
        if(itemsize > (1<<15)){
            large_item_data_flag = true;
            break;
        }
    }
    bitstr.writeBits(large_item_data_flag,1);   //bit(1) large_item_data_flag


    uint64_t icc_size = 0;
    if(icc_flag){
        auto rICC_item = mItemInfoBox.findItemWithType("rICC");
        if (rICC_item != nullptr) {
            icc_size = mItemLocationBox.getItemLocationForID(rICC_item->getItemID()).getItemSize();
            bitstr.writeBits(static_cast<uint32_t>(icc_size - 1),large_metadata_flag ? 20 : 10);   //icc_data_size_minus1
        } else {
            logError() << "Error: ICC item not found for 'rICC' type." << '\n';
            return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                            HEIF::ErrorOrigin::Writer_Mini,
                            "writeMiniBox: icc_flag set but no 'rICC' item found in ItemInfoBox");
        }
    }

    uint32_t tmap_icc_data_size = 0;
    if(hdr_flag && gainmap_flag && tmap_icc_flag){
        tmap_icc_data_size = static_cast<uint32_t>(tmap_colr->getIccProfile().size());
        bitstr.writeBits(tmap_icc_data_size - 1,large_metadata_flag ? 20 : 10); //tmap_icc_data_size_minus1
    }

    uint32_t gainmap_metadata_size = 0;
    if(hdr_flag && gainmap_flag){
        // The tmap item payload is: 1-byte version + gainmap metadata (ISO 21496-1).
        // Retrieve the tmap item size from the location box; subtract 1 for the version byte.
        const auto& tmapLoc = mItemLocationBox.getItemLocationForID(TONEMAP_ITEM_ID);
        const uint64_t tmapItemSize = tmapLoc.getItemSize();
        if(tmapItemSize > 1){
            gainmap_metadata_size = static_cast<uint32_t>(tmapItemSize - 1);
        }
        bitstr.writeBits(gainmap_metadata_size, large_metadata_flag ? 20 : 10); //gainmap_metadata_size
    }

    uint64_t gainmap_item_data_size = 0;
    if(hdr_flag && gainmap_flag){
        gainmap_item_data_size = mItemLocationBox.getItemLocationForID(GAINMAP_ITEM_ID).getItemSize();
        bitstr.writeBits(gainmap_item_data_size, large_item_data_flag ? 28 :15);    //gainmap_item_data_size
    }

    BitStream gainmap_config;
    uint32_t gainmap_config_size = 0;
    if(hdr_flag && gainmap_flag && gainmap_item_data_size > 0){
        this->getConfigBox(GAINMAP_ITEM_ID)->getConfiguration().writeCompactDecConfigRecord(gainmap_config);
        gainmap_config_size = static_cast<uint32_t>(gainmap_config.getSize());
        bitstr.writeBits(gainmap_config_size, large_codec_config_flag ? 12 : 3);  //gainmap_codec_config_size
    }


    bitstr.writeBits(conf_bitstr.getSize(), large_codec_config_flag ? 12 : 3);  //main_item_codec_config_size

    uint32_t main_item_size = static_cast<uint32_t>(mItemLocationBox.getItemLocationForID(PRIMARY_ITEM_ID).getItemSize());
    //Write itemdata without excess bytes if applicable. Should only happen with compact VVC
    main_item_size = mWriteCompact ? main_item_size - (mItemLocationBox.getLengthSize() + 2) : main_item_size;
    bitstr.writeBits(main_item_size - 1,large_item_data_flag ? 28 : 15);  //main_item_data_size_minus1


    uint32_t alpha_item_data_size = 0;
    ISOBMFF::BitStream alphaconfbitstr;
    if(alpha_flag){
        alpha_item_data_size = static_cast<uint32_t>(mItemLocationBox.getItemLocationForID(ALPHA_ITEM_ID).getItemSize());
        bitstr.writeBits(alpha_item_data_size,large_item_data_flag ? 28 : 15);  //alpha_item_data_size
        if(alpha_item_data_size > 0){
            this->getConfigBox(ALPHA_ITEM_ID)->getConfiguration().writeCompactDecConfigRecord(alphaconfbitstr);
            bitstr.writeBits(alphaconfbitstr.getSize(),large_codec_config_flag ? 12 : 3); //alpha_item_codec_config_size
        }
    }


    if(exif_flag || xmp_flag){
        bitstr.writeBits(exif_compressed_flag,1);   //bit(1) exif_xmp_compressed_flag
    }

    uint32_t exif_data_size = 0;
    uint32_t xmp_data_size = 0;
    if(exif_flag){
        auto exifItem = mItemInfoBox.findItemWithType(exif_compressed_flag ? FourCCInt("dExf") : FourCCInt("Exif"));
        if (exifItem != nullptr) {
            exif_data_size = static_cast<uint32_t>(mItemLocationBox.getItemLocationForID(exifItem->getItemID()).getItemSize());
            bitstr.writeBits(exif_data_size - 1, large_metadata_flag ? 20 : 10);  //exif_data_size_minus1
        } else {
            logError() << "Error: Exif item not found." << '\n';
            return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                            HEIF::ErrorOrigin::Writer_Mini,
                            "writeMiniBox: exif_flag set but no Exif/dExf item found in ItemInfoBox");
        }
    }
    if(xmp_flag){
        auto xmpItem = mItemInfoBox.findItemWithType("mime");
        if (xmpItem != nullptr) {
            xmp_data_size = static_cast<uint32_t>(mItemLocationBox.getItemLocationForID(xmpItem->getItemID()).getItemSize());
            bitstr.writeBits(xmp_data_size - 1, large_metadata_flag ? 20 : 10);  //xmp_data_size_minus1
        } else {
            logError() << "Error: XMP item not found." << '\n';
            return HEIF_FAIL(HEIF::ErrorCode::MISSING_PROPERTIES,
                            HEIF::ErrorOrigin::Writer_Mini,
                            "writeMiniBox: xmp_flag set but no 'mime' item found in ItemInfoBox");
        }
    }


    bitstr.addTrailingBits();   //bit padding till byte alignment


    bitstr.writeBitStream(conf_bitstr);   //main_item_codec_config[main_item_codec_config_size]


    //Get config associated with the alpha item and ensure it is not the same as the main item config
    //If so, write the explicit alpha item config
    if(alpha_item_data_size > 0 && alphaconfbitstr.getSize() > 0){
        auto alpha = this->getConfigBox(ALPHA_ITEM_ID);
        if(alpha != nullptr && alpha != this->getConfigBox(PRIMARY_ITEM_ID)){
            alpha->getConfiguration().writeCompactDecConfigRecord(bitstr); //alpha_item_explicit_codec_config
        }
    }


    if(hdr_flag && gainmap_flag  && gainmap_item_data_size > 0 && gainmap_config.getSize() != 0){
        if( this->getConfigBox(GAINMAP_ITEM_ID) != this->getConfigBox(PRIMARY_ITEM_ID)){
            bitstr.writeBitStream(gainmap_config);  //gainmap_item_explicit_codec_config
        }
    }


    //###
    //Instead of writing the items here, it is done in a higher level to access the items
    //###


    //Construct another bitstream to ease adding of miniBox header and allowing calculating size
    ISOBMFF::BitStream fullMiniOutput;

    //calculate size for rest of the mdat
    uint64_t itemSizeSum = main_item_size + icc_size + tmap_icc_data_size
                            + gainmap_metadata_size + alpha_item_data_size
                            + gainmap_item_data_size + exif_data_size + xmp_data_size;


    //Insert box header information to the start of the bytestream
    const uint8_t BOX_HEADER_SIZE = 8;
    fullMiniOutput.write32Bits(static_cast<uint32_t>(bitstr.getSize() + itemSizeSum + BOX_HEADER_SIZE));
    fullMiniOutput.write32Bits(FourCCInt("mini").getUInt32());
    fullMiniOutput.writeBitStream(bitstr);

    //Write the minibox to output (item mdat payloads excluded)
    output.writeBitStream(fullMiniOutput);

    return HEIF::ErrorCode::OK;
}


namespace
{
    // Walk a length-prefixed VVC item payload and count its NAL units.
    // Used by parseMiniBox to record per-item NAL counts up-front, so the
    // reader can predict the post-transform (start-code-substituted) buffer
    // size without re-reading the bytes.
    //
    // Returns 0 if the chain is malformed (length runs past the end).
    uint32_t countLengthPrefixedNalUnits(const Vector<uint8_t>& storage,
                                         uint64_t offset,
                                         uint64_t size,
                                         uint8_t  lengthSize)
    {
        if (lengthSize == 0 || lengthSize > 4) return 0;
        if (offset + size > storage.size())   return 0;

        uint32_t count = 0;
        uint64_t pos   = 0;
        while (pos < size)
        {
            //Need at least lengthSize bytes for the next length field.
            if (size - pos < lengthSize) return 0;

            uint32_t nalLen = 0;
            for (uint8_t i = 0; i < lengthSize; ++i)
            {
                nalLen = (nalLen << 8) | storage[offset + pos + i];
            }
            pos += lengthSize;
            if (nalLen == 0 || nalLen > size - pos) return 0;
            pos += nalLen;
            ++count;
        }
        return count;
    }
}


// C6262: MSVC /analyze reports ~185 KB stack for this function.
// Root cause: 69 std::shared_ptr<Box> locals (HDR metadata boxes, config
// boxes, colour info, etc.) whose destructors the analyser counts as
// simultaneously live on the worst-case exception-unwind path.  Actual
// runtime usage is well within the 1 MB default Windows stack.  Splitting
// the function into sub-functions would lose the linear parse-then-populate
// structure that mirrors the MiniBox bitstream layout.
#if defined(_MSC_VER)
#pragma warning(suppress : 6262)
#endif
HEIF::ErrorCode MetaBox::parseMiniBox(ISOBMFF::BitStream &bitstr, FourCCInt mMinorVersion)
{
    //Box headers are read earlier, but not parsed. Do that here.
    parseBoxHeader(bitstr);

    bitstr.readBits(2); //mini_version

    //Mini Flags
    bool explicit_codec_types_flag = bitstr.readBits(1);
    bool float_flag = bitstr.readBits(1);
    bool full_range_flag = bitstr.readBits(1);
    bool alpha_flag = bitstr.readBits(1);
    bool explicit_cicp_flag = bitstr.readBits(1);
    bool hdr_flag = bitstr.readBits(1);
    bool icc_flag = bitstr.readBits(1);
    bool exif_flag = bitstr.readBits(1);
    bool xmp_flag = bitstr.readBits(1);

    uint8_t chroma_subsampling = static_cast<uint8_t>(bitstr.readBits(2));
    uint8_t orientation_minus1 = static_cast<uint8_t>(bitstr.readBits(3));

    //Spatial extents
    bool large_dimensions_flag = bitstr.readBits(1);
    uint32_t width_minus1 = bitstr.readBits(large_dimensions_flag ? 15 : 7);
    uint32_t height_minus1 = bitstr.readBits(large_dimensions_flag ? 15 : 7);

    //Pixel Information
    bool chroma_is_horizontally_centered = false, chroma_is_vertically_centered = false;
    if (chroma_subsampling == 1 || chroma_subsampling == 2){
        chroma_is_horizontally_centered = bitstr.readBits(1);
    }if (chroma_subsampling == 1){
        chroma_is_vertically_centered = bitstr.readBits(1);
    }

    //Bit Depth
    uint8_t bit_depth_log2_minus4 = 0, bit_depth_minus9 = 0;
    bool high_bit_depth_flag = false;
    if(float_flag){
        bit_depth_log2_minus4 = static_cast<uint8_t>(bitstr.readBits(2));
    }else{
        high_bit_depth_flag = bitstr.readBits(1);
        if(high_bit_depth_flag){
            bit_depth_minus9 = static_cast<uint8_t>(bitstr.readBits(3));
        }
    }

    //Alpha premultiplication flag
    bool alpha_is_premultiplied = false;
    if(alpha_flag){
        alpha_is_premultiplied = bitstr.readBits(1);
    }

    //CICP
    uint8_t colour_primaries = 0, transfer_characteristics = 0, matrix_coefficients = 0;
    if(explicit_cicp_flag){
        colour_primaries = static_cast<uint8_t>(bitstr.readBits(8));
        transfer_characteristics = static_cast<uint8_t>(bitstr.readBits(8));
        matrix_coefficients = static_cast<uint8_t>(bitstr.readBits(8));
    }else{
          colour_primaries = icc_flag ? 2 : 1;
          transfer_characteristics = icc_flag ? 2 : 13;
          matrix_coefficients = (chroma_subsampling == 0) ? 2 : 6;
    }

    //Codectype
    FourCCInt codec_config_type , infe_type;
    if(explicit_codec_types_flag){
        infe_type = bitstr.readBits(32);
        codec_config_type = bitstr.readBits(32);
    }else{
        // codec information is defined by the brand in the minor_version field of FileTypeBox
        if(mMinorVersion == "heic"){
            infe_type = "hvc1";
            codec_config_type = "hvcC";
        }else if(mMinorVersion == "avic"){
            infe_type = "avc1";
            codec_config_type = "avcC";
        }else if(mMinorVersion == "vvic"){
            infe_type = "vvc1";
            codec_config_type = "vvcC";
        }else if(mMinorVersion == "vvi3"){//Compact VVC
            infe_type = "vvc1";
            codec_config_type = "vvcC";
        }else if(mMinorVersion == "jpeg"){
            infe_type = "jpeg";
            codec_config_type = "jpgC";
        }else if(mMinorVersion == "avif"){
            infe_type = "avif";
            codec_config_type = "TEST";
        }else{
            return HEIF_FAIL(HEIF::ErrorCode::INVALID_MEDIA_FORMAT,
                            HEIF::ErrorOrigin::Reader_Mini,
                            "parseMiniBox: unsupported minor_version brand for explicit codec types");
        }
    }

    //HDR properties

    //flags used inside and outside of this hdr parsing
    bool gainmap_flag = false;
    bool tmap_icc_flag = false;
    bool tmap_explicit_cicp_flag = false;
    bool gainmap_full_range_flag = false;

    bool gainmap_chroma_is_horizontally_centered = false;
    bool gainmap_chroma_is_vertically_centered = false;

    bool clli_flag = false;
    std::shared_ptr<ContentLightLevelBox> clli = std::make_shared<ContentLightLevelBox>();
    bool mdcv_flag = false;
    std::shared_ptr<MasteringDisplayColourVolumeBox> mdcv = std::make_shared<MasteringDisplayColourVolumeBox>();
    bool cclv_flag = false;
    std::shared_ptr<ContentColourVolumeBox> cclv = std::make_shared<ContentColourVolumeBox>();
    bool amve_flag = false;
    std::shared_ptr<AmbientViewingEnvironmentBox> amve = std::make_shared<AmbientViewingEnvironmentBox>();
    bool reve_flag = false;
    std::shared_ptr<ReferenceViewingEnvironmentBox> reve = std::make_shared<ReferenceViewingEnvironmentBox>();
    bool ndwt_flag = false;
    std::shared_ptr<NominalDiffuseWhiteBox> ndwt = std::make_shared<NominalDiffuseWhiteBox>();

    bool tmap_clli_flag = false;
    std::shared_ptr<ContentLightLevelBox> tmap_clli = std::make_shared<ContentLightLevelBox>();
    bool tmap_mdcv_flag = false;
    std::shared_ptr<MasteringDisplayColourVolumeBox> tmap_mdcv = std::make_shared<MasteringDisplayColourVolumeBox>();
    bool tmap_cclv_flag = false;
    std::shared_ptr<ContentColourVolumeBox> tmap_cclv = std::make_shared<ContentColourVolumeBox>();
    bool tmap_amve_flag = false;
    std::shared_ptr<AmbientViewingEnvironmentBox> tmap_amve = std::make_shared<AmbientViewingEnvironmentBox>();
    bool tmap_reve_flag = false;
    std::shared_ptr<ReferenceViewingEnvironmentBox> tmap_reve = std::make_shared<ReferenceViewingEnvironmentBox>();
    bool tmap_ndwt_flag = false;
    std::shared_ptr<NominalDiffuseWhiteBox> tmap_ndwt = std::make_shared<NominalDiffuseWhiteBox>();

    uint8_t tmap_colour_primaries = 0, tmap_transfer_characteristics = 0, tmap_matrix_coefficients = 0;
    bool tmap_full_range_flag = true;

    uint8_t gainmap_matrix_coefficients = 0;
    uint8_t gainmap_bit_depth_log2_minus4 = 0, gainmap_bit_depth_minus9 = 0;
    uint8_t gainmap_chroma_subsampling = 0;
    uint32_t gainmap_width_minus1 = 0, gainmap_height_minus1=0;
    bool gainmap_float_flag = false, gainmap_high_bit_depth_flag = false;

    if(hdr_flag){
        gainmap_flag = bitstr.readBits(1);
        if(gainmap_flag){
            bool gainmap_dimension_same_as_main_item_flag = bitstr.readBits(1);

            if(gainmap_dimension_same_as_main_item_flag){
                gainmap_width_minus1 = width_minus1;
                gainmap_height_minus1 = height_minus1;
            }else{
                gainmap_width_minus1 = bitstr.readBits(large_dimensions_flag ? 15 : 7);
                gainmap_height_minus1 = bitstr.readBits(large_dimensions_flag ? 15 : 7);
            }

            gainmap_matrix_coefficients = static_cast<uint8_t>(bitstr.readBits(8));
            gainmap_full_range_flag = bitstr.readBits(1);

            //Chroma
            gainmap_chroma_subsampling = static_cast<uint8_t>(bitstr.readBits(2));
            if(gainmap_chroma_subsampling == 1 || gainmap_chroma_subsampling ==2){
                gainmap_chroma_is_horizontally_centered = bitstr.readBits(1);
            }if(gainmap_chroma_subsampling == 1){
                gainmap_chroma_is_vertically_centered = bitstr.readBits(1);
            }

            //Bitdepth
            gainmap_float_flag = bitstr.readBits(1);
            if(gainmap_float_flag){
                gainmap_bit_depth_log2_minus4 = static_cast<uint8_t>(bitstr.readBits(2));
            }else{
                gainmap_high_bit_depth_flag = bitstr.readBits(1);
                if(gainmap_high_bit_depth_flag){
                    gainmap_bit_depth_minus9 = static_cast<uint8_t>(bitstr.readBits(3));
                }
            }

            //Color info
            tmap_icc_flag = bitstr.readBits(1);
            tmap_explicit_cicp_flag = bitstr.readBits(1);
            if(tmap_explicit_cicp_flag){
                tmap_colour_primaries = static_cast<uint8_t>(bitstr.readBits(8));
                tmap_transfer_characteristics = static_cast<uint8_t>(bitstr.readBits(8));
                tmap_matrix_coefficients = static_cast<uint8_t>(bitstr.readBits(8));
                tmap_full_range_flag = bitstr.readBits(1);
            }else{
                tmap_colour_primaries = 1;
                tmap_transfer_characteristics = 13;
                tmap_matrix_coefficients = 6;
                tmap_full_range_flag = true;
            }
        }

        clli_flag = bitstr.readBits(1);
        mdcv_flag = bitstr.readBits(1);
        cclv_flag = bitstr.readBits(1);
        amve_flag = bitstr.readBits(1);
        reve_flag = bitstr.readBits(1);
        ndwt_flag = bitstr.readBits(1);

        if(clli_flag){
            clli->parseBox(bitstr);
        }
        if(mdcv_flag){
            mdcv->parseBox(bitstr);
        }
        if(cclv_flag){
            cclv->parseBox(bitstr);
        }
        if(amve_flag){
            amve->parseBox(bitstr);
        }
        if(reve_flag){
            reve->parseBox(bitstr);
        }
        if(ndwt_flag){
            ndwt->parseBox(bitstr);
        }
        if(gainmap_flag){
            tmap_clli_flag = bitstr.readBits(1);
            tmap_mdcv_flag = bitstr.readBits(1);
            tmap_cclv_flag = bitstr.readBits(1);
            tmap_amve_flag = bitstr.readBits(1);
            tmap_reve_flag = bitstr.readBits(1);
            tmap_ndwt_flag = bitstr.readBits(1);

            if(tmap_clli_flag){
                tmap_clli->parseBox(bitstr);
            }
            if(tmap_mdcv_flag){
                tmap_mdcv->parseBox(bitstr);
            }
            if(tmap_cclv_flag){
                tmap_cclv->parseBox(bitstr);
            }
            if(tmap_amve_flag){
                tmap_amve->parseBox(bitstr);
            }
            if(tmap_reve_flag){
                tmap_reve->parseBox(bitstr);
            }
            if(tmap_ndwt_flag){
                tmap_ndwt->parseBox(bitstr);
            }
        }
    } //HDR

    //Chunk sizes
    bool large_metadata_flag = false;
    if (icc_flag || exif_flag || xmp_flag || (hdr_flag && gainmap_flag)){
        large_metadata_flag = bitstr.readBits(1);
    }
    bool large_codec_config_flag = bitstr.readBits(1);
    bool large_item_data_flag = bitstr.readBits(1);

    uint32_t icc_data_size_minus1 = 0;
    if(icc_flag){
        icc_data_size_minus1 = bitstr.readBits(large_metadata_flag ? 20 : 10);
    }

    //Tonemap and Gainmap setup
    uint32_t tmap_icc_data_size_minus1 = 0;
    if(hdr_flag && gainmap_flag && tmap_icc_flag){
        tmap_icc_data_size_minus1 = bitstr.readBits(large_metadata_flag ? 20 : 10);
    }
    uint32_t gainmap_item_data_size = 0, gainmap_metadata_size = 0, gainmap_item_codec_config_size = 0;
    if(hdr_flag && gainmap_flag){
        gainmap_metadata_size =bitstr.readBits(large_metadata_flag ? 20 : 10);
    }
    if(hdr_flag && gainmap_flag){
        gainmap_item_data_size =bitstr.readBits(large_item_data_flag ? 28 : 15);
    }
    if(hdr_flag && gainmap_flag && gainmap_item_data_size > 0){
        gainmap_item_codec_config_size = bitstr.readBits(large_codec_config_flag ? 12 : 3);
    }

    //Codec config size in bytes
    uint32_t main_item_codec_config_size = bitstr.readBits(large_codec_config_flag ? 12 : 3);
    uint32_t main_item_data_size_minus1 = bitstr.readBits(large_item_data_flag ? 28 : 15);

    //Alpha data setup
    uint32_t alpha_item_data_size = 0, alpha_item_codec_config_size = 0;
    if(alpha_flag){
        alpha_item_data_size = bitstr.readBits(large_item_data_flag ? 28 : 15);
    }
    if(alpha_flag && alpha_item_data_size > 0){
        alpha_item_codec_config_size = bitstr.readBits(large_codec_config_flag ? 12 : 3);
    }

    //Exif & XMP setup
    uint32_t exif_data_size_minus1 = 0, xmp_data_size_minus1 = 0;
    bool exif_xmp_compressed_flag = false;
    if(exif_flag || xmp_flag){
        exif_xmp_compressed_flag = bitstr.readBits(1);
    }if(exif_flag){
        exif_data_size_minus1 = bitstr.readBits(large_metadata_flag ? 20 : 10);
    }if(xmp_flag){
        xmp_data_size_minus1 = bitstr.readBits(large_metadata_flag ? 20 : 10);
    }

    bitstr.skipToByteBoundary();    //trailing_bits()

    uint64_t mdat_offset;

    ISOBMFF::BitStream main_item_codec_config,
                        alpha_item_codec_config,
                        gainmap_item_codec_config;

    const uint32_t MAX_ICC_SIZE = UINT32_MAX;
    if (icc_data_size_minus1 >= MAX_ICC_SIZE || tmap_icc_data_size_minus1 >= MAX_ICC_SIZE) {
        return HEIF_FAIL(HEIF::ErrorCode::MEMORY_TOO_SMALL_BUFFER,
                        HEIF::ErrorOrigin::Reader_Mini,
                        "parseMiniBox: ICC data size exceeds maximum supported allocation");
    }

    // Check for overflow
    if (icc_data_size_minus1 == UINT32_MAX || tmap_icc_data_size_minus1 == UINT32_MAX) {
        return HEIF_FAIL(HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                        HEIF::ErrorOrigin::Reader_Mini,
                        "parseMiniBox: ICC data size_minus1 == UINT32_MAX would overflow");
    }

    Vector<uint8_t> tmap_icc_data(tmap_icc_data_size_minus1 + 1),
                    icc_data(icc_data_size_minus1 + 1);

    //Per-item byte offsets into bitstr.getStorage() -- captured during the
    //data-skip section, consumed after the codec configs are parsed (so
    //lengthSize is known before we walk the NAL length-prefix chain).
    uint64_t alpha_item_byte_offset = 0;
    uint64_t main_item_byte_offset  = 0;

    try{
        //Main Item Codec Config
        main_item_codec_config = BitStream(bitstr, main_item_codec_config_size); 

        //Alpha Item Codec Config
        if(alpha_flag && alpha_item_data_size > 0){
            if(alpha_item_codec_config_size == 0){
                alpha_item_codec_config_size = main_item_codec_config_size;
                alpha_item_codec_config = main_item_codec_config;
            }else{
                //Explicit alpha item codec config
                alpha_item_codec_config = BitStream(bitstr, alpha_item_codec_config_size);
            }
        }

        //Gainmap Item Codec Config
        if(hdr_flag && gainmap_flag && gainmap_item_data_size > 0){
            if(gainmap_item_codec_config_size == 0){
                gainmap_item_codec_config_size = main_item_codec_config_size;
                gainmap_item_codec_config = main_item_codec_config;
            }else{
                //Explicit gainmap item codec config
                gainmap_item_codec_config = BitStream(bitstr, gainmap_item_codec_config_size);
            }
        }

        //Read data
        if(icc_flag){
            bitstr.read8BitsArray(icc_data, icc_data_size_minus1 + 1);
        }
        if(hdr_flag && gainmap_flag && tmap_icc_flag){
            bitstr.read8BitsArray(tmap_icc_data, tmap_icc_data_size_minus1 + 1);
        }
        if(hdr_flag && gainmap_flag && gainmap_metadata_size > 0){
            Vector<uint8_t> tmap_item_payload;
            tmap_item_payload.push_back(0);  // version byte (shall be 0)
            bitstr.read8BitsArray(tmap_item_payload, gainmap_metadata_size);
            mItemDataBox.addData(tmap_item_payload);
        }

        //Start of MediaDataBox
        mdat_offset = bitstr.getPos();

        //Alpha Item Data
        if(alpha_flag && alpha_item_data_size > 0){
            alpha_item_byte_offset = bitstr.getPos();
            bitstr.skipBytes(alpha_item_data_size);
        }
        if(hdr_flag && gainmap_flag && gainmap_item_data_size > 0){
            bitstr.skipBytes(gainmap_item_data_size);
        }

        //Main item data
        main_item_byte_offset = bitstr.getPos();
        bitstr.skipBytes(main_item_data_size_minus1 + 1);

        //Metadata data
        if(exif_flag){
            bitstr.skipBytes(exif_data_size_minus1 + 1);
        }
        if(xmp_flag){
            bitstr.skipBytes(xmp_data_size_minus1 + 1);
        }
    }
    catch(RuntimeError& e){
        logError() << "Error: Item size too small, or missing item data from bitstream" << '\n';
        logError() << e.what() << '\n';
        return HEIF_FAIL(HEIF::ErrorCode::BYTESTREAM_READ_ERROR,
                        HEIF::ErrorOrigin::Reader_Mini,
                        "parseMiniBox: item data read failed (item size exceeds available bitstream)");
    }

    //Memsize check
    if(bitstr.numBytesLeft() > 0){
        logError() << "Error: Minibox read "<< bitstr.getPos() << " bytes, but box size is " << bitstr.getSize() << '\n';
        return HEIF_FAIL(HEIF::ErrorCode::BYTESTREAM_READ_ERROR,
                        HEIF::ErrorOrigin::Reader_Mini,
                        "parseMiniBox: unconsumed trailing bytes after parsing minibox");
    }


    //END OF MINI BITSTREAM


    //
    //Populating the MetaBox with read data
    //

    mHandlerBox.setHandlerType("pict");
    mPrimaryItemBox.setItemId(1);

    //Populate ItemInfoBox

    mItemInfoBox.setVersion(2);
    ItemInfoEntry one;
    one.setItemID(1); one.setItemType(infe_type);
    mItemInfoBox.addItemInfoEntry(one);

    if(alpha_item_data_size != 0){
        ItemInfoEntry two; two.setVersion(2);
        two.setFlags(1);
        two.setItemID(2); two.setItemType(infe_type);
        mItemInfoBox.addItemInfoEntry(two);
    }
    if(gainmap_flag){
        ItemInfoEntry three; three.setVersion(2);
        three.setFlags(0);
        three.setItemID(3); three.setItemType("tmap");
        mItemInfoBox.addItemInfoEntry(three);
    }
    if(gainmap_item_data_size != 0){
        ItemInfoEntry four; four.setVersion(2);
        four.setFlags(1);
        four.setItemID(4); four.setItemType(infe_type);
        mItemInfoBox.addItemInfoEntry(four);
    }
    if(exif_flag){
        ItemInfoEntry five; five.setVersion(2);
        five.setFlags(1); five.setItemID(6);
        if(exif_xmp_compressed_flag){
            five.setItemType("dExf");
        }else{
            five.setItemType("Exif");
        }
        mItemInfoBox.addItemInfoEntry(five);
    }
    if(xmp_flag){
        ItemInfoEntry six; six.setVersion(2);six.setFlags(1); six.setItemID(7);
        six.setItemType("mime"); six.setContentType("application/rdf+xml");
        if(exif_xmp_compressed_flag){
            six.setContentEncoding("deflate");
        }
        mItemInfoBox.addItemInfoEntry(six);
    }


    //Populating Item Reference Box

    if(alpha_item_data_size != 0){  //item_ID 2
        mItemReferenceBox.add("auxl",2,1);
        if(alpha_is_premultiplied){
            mItemReferenceBox.add("prem",1,2);
        }
    }
    if(gainmap_flag && gainmap_item_data_size != 0){//item_ID 4
        mItemReferenceBox.add("dimg",3,1);
        mItemReferenceBox.add("dimg",3,4);
    }
    if(gainmap_flag && gainmap_item_data_size == 0){//alt item_ID 4
        mItemReferenceBox.add("dimg",3,1);
    }
    if(exif_flag){  //item_ID 6
        mItemReferenceBox.add("cdsc",6,1);
    }
    if(xmp_flag){   //item_ID 7
        mItemReferenceBox.add("cdsc",7,1);
    }


    //Populating Entity To Group Box
    if(gainmap_flag){
        EntityToGroupBox entg; entg.setVersion(0);entg.setFlags(0);entg.setGroupId(5);
        entg.setType("altr"); entg.setEntityIds({3,1});
        mGroupsListBox.addEntityToGroupBox(entg);
    }


    //Populating Item Properties

    //Item Property #1 Main Item Decoder Configuration

    //Values passed to compact vvc config expansion
    ///@note this instance is used for all items, with only id needing to be changed
    VvcDecoderConfigurationRecord::ConfigValues conf_vals;
    conf_vals.chroma_subsampling = chroma_subsampling;
    conf_vals.high_bit_depth_flag = high_bit_depth_flag;
    conf_vals.bit_depth_minus9 = bit_depth_minus9;
    conf_vals.width_minus1 = width_minus1;
    conf_vals.height_minus1 = height_minus1;
    conf_vals.gainmap_chroma_subsampling = gainmap_chroma_subsampling;
    conf_vals.gainmap_high_bit_depth_flag = gainmap_high_bit_depth_flag;
    conf_vals.gainmap_bit_depth_minus9 = gainmap_bit_depth_minus9;
    conf_vals.gainmap_width_minus1 = gainmap_width_minus1;
    conf_vals.gainmap_height_minus1 = gainmap_height_minus1;

    if(main_item_codec_config_size != 0){
        if(codec_config_type == "hvcC"){
            std::shared_ptr<HevcConfigurationBox> hvcC = std::make_shared<HevcConfigurationBox>();
            hvcC->setType(codec_config_type);
            HevcDecoderConfigurationRecord hvCR;
            hvCR.parseConfig(main_item_codec_config);
            hvcC->setConfiguration(hvCR);
            mItemPropertiesBox.addProperty(hvcC,{},"");
        }else if(infe_type == "avc1"){
            std::shared_ptr<AvcConfigurationBox> avcC = std::make_shared<AvcConfigurationBox>();
            avcC->setType(codec_config_type);
            AvcDecoderConfigurationRecord avCR;
            avCR.parseConfig(main_item_codec_config);
            avcC->setConfiguration(avCR);
            mItemPropertiesBox.addProperty(avcC, {}, "");
        }else if(infe_type == "jpeg"){
            std::shared_ptr<JpegConfigurationBox> jpeg1 = std::make_shared<JpegConfigurationBox>();
            jpeg1->parseBox(main_item_codec_config);
            mItemPropertiesBox.addProperty(jpeg1,{},"");
        }else if(infe_type == "vvc1"){
            std::shared_ptr<VvcConfigurationBox> vvcC = std::make_shared<VvcConfigurationBox>();
            VvcDecoderConfigurationRecord vvCR;
            conf_vals.item_id = 1;
            vvCR.parseCompactConfig(main_item_codec_config, conf_vals);
            mMultilayer = vvCR.getMultiLayerFlag();
            vvcC->setConfiguration(vvCR);
            mItemPropertiesBox.addProperty(vvcC,{},"");

            //multi_layer_flag=1 means the item payload is a chain of
            //length-prefixed NAL units (formatted normally).  Walk the chain
            //now and record numNALs[itemId=1] so the reader can compute the
            //post-transform size (numNALs * (4 - lengthSize)) without I/O.
            //
            //multi_layer_flag=0 paths leave the count at 0 -- those items are
            //bare NAL payload and the reader handles them with a fixed +6.
            if (mMultilayer)
            {
                mNalUnitCounts[1] = countLengthPrefixedNalUnits(
                    bitstr.getStorage(),
                    main_item_byte_offset,
                    main_item_data_size_minus1 + 1,
                    vvCR.getLengthSize());
            }
        }else{  //unrecognized/unsupported config

            ///@todo add passing of raw configs for spec codecs, which have no implementation

            if(explicit_codec_types_flag){  //Allow passing of config as raw, no format defined
                logWarning() << "Warning: Unknown explicit codec: " << infe_type.getString() << '\n';;
            }else{
                //if not explicit, minor brand should've defined it, and it should be supported
                logWarning() << "Warning: Unsupported codec: " << infe_type.getString() << '\n';
            }
            std::shared_ptr<RawPropertyBox> rawConf = std::make_shared<RawPropertyBox>();
            rawConf->setType(codec_config_type);
            rawConf->setData(main_item_codec_config);
            mItemPropertiesBox.addProperty(rawConf,{},"");
        }
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #2 - Main Image Spatial Extents
    std::shared_ptr<ImageSpatialExtentsProperty> ispe = std::make_shared<ImageSpatialExtentsProperty>();
    ispe->setDisplayWidth(width_minus1 + 1); ispe->setDisplayHeight(height_minus1 + 1);
    mItemPropertiesBox.addProperty(ispe,{},"");

    //Item Property #3 - Main Image Pixel Information
    std::shared_ptr<PixelInformationProperty> pixi = std::make_shared<PixelInformationProperty>();
    pixi->setVersion(0);pixi->setFlags(1);

    //define the main components (channels, like Y U V, monochrome has only one etc.) 
    uint8_t main_components;
    if(chroma_subsampling == 0){main_components = 1;}else{main_components = 3;}

    //Set bitdepth for each channel
    uint8_t bitdepth_per_channel = 8;
    if(float_flag == 1){
        bitdepth_per_channel = 1 << (bit_depth_log2_minus4 + 4);
    }else if(high_bit_depth_flag){
        bitdepth_per_channel = bit_depth_minus9 + 9;
    }
    Vector<uint8_t> bits_per_channel;
    for(int i=0;i<main_components;i++){
        bits_per_channel.push_back(bitdepth_per_channel);
    }

    //Extended pixi (pxflags != 0)
    std::map<uint8_t, PixelInformationProperty::ChannelInfo> inputChannels;
    if(alpha_flag){  //entry if alpha flag
        PixelInformationProperty::ChannelInfo alphaChannelInfo;
        alphaChannelInfo.ComponentFormat = float_flag;
        alphaChannelInfo.ChannelLabelFlag = false;
        alphaChannelInfo.SubSamplingType = 0;
        alphaChannelInfo.SubSamplingLocation = 0;

        inputChannels.emplace(static_cast<uint8_t>(5),alphaChannelInfo);
    }
    if(main_components > 0){    //entry if main_components > 0
        PixelInformationProperty::ChannelInfo firstChannelInfo;
        firstChannelInfo.ComponentFormat = float_flag;
        firstChannelInfo.ChannelLabelFlag = false;
        firstChannelInfo.SubSamplingType = 0;
        firstChannelInfo.SubSamplingLocation = 0;
        inputChannels.emplace(static_cast<uint8_t>(2),firstChannelInfo);
    }
    if(main_components > 1){    //entry if main_components > 1
        for(int i = 0;i<=1;i++){
            PixelInformationProperty::ChannelInfo chromaChannelInfo;
            chromaChannelInfo.ComponentFormat = float_flag;
            chromaChannelInfo.SubSamplingFlag = true;

            uint8_t subsampling_type = 0;
            if(chroma_subsampling == 1){
                subsampling_type=2;
            }else if(chroma_subsampling == 2){
                subsampling_type=1;
            }else if(chroma_subsampling == 3){
                subsampling_type=0;
            }

            uint8_t subsampling_location = 0;
            if(chroma_is_horizontally_centered && chroma_is_vertically_centered){
                subsampling_location = 1;
            }else if(chroma_is_horizontally_centered && !chroma_is_vertically_centered){
                subsampling_location = 3;
            }else if(!chroma_is_horizontally_centered && chroma_is_vertically_centered){
                subsampling_location = 0;
            }else if(!chroma_is_horizontally_centered && !chroma_is_vertically_centered){
                subsampling_location = 2;
            }

            chromaChannelInfo.SubSamplingType = subsampling_type;
            chromaChannelInfo.SubSamplingLocation = subsampling_location;

            inputChannels.emplace(static_cast<uint8_t>(3+i),chromaChannelInfo);
        }
    }
    pixi->setNumChannels(main_components);
    pixi->setBitsPerChannels(bits_per_channel);
    pixi->setChannelData(inputChannels);
    mItemPropertiesBox.addProperty(pixi, {},"");


    //Item Property #4 - Main Image Colour Information
    std::shared_ptr<ColourInformationBox> colr = std::make_shared<ColourInformationBox>();
    colr->setColourType("nclx");colr->setColourPrimaries(colour_primaries);
    colr->setTransferCharacteristics(transfer_characteristics);
    colr->setMatrixCoefficients(matrix_coefficients);
    colr->setFullRangeFlag(full_range_flag);
    mItemPropertiesBox.addProperty(colr,{},"");

    //Item Property #5 - ICC Type Main Image Colour Information
    if(icc_flag){
        std::shared_ptr<ColourInformationBox> colr2 = std::make_shared<ColourInformationBox>();
        colr2->setColourType("prof");colr2->setIccProfile(icc_data);
        mItemPropertiesBox.addProperty(colr2,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #6 - Alpha Item Decoder Configuration
    if(alpha_item_codec_config_size != 0){
        if(infe_type == "hvc1"){
            std::shared_ptr<HevcConfigurationBox> hvc1 = std::make_shared<HevcConfigurationBox>();
            hvc1->setType(codec_config_type);
            HevcDecoderConfigurationRecord hevc;
            hevc.parseConfig(alpha_item_codec_config);
            hvc1->setConfiguration(hevc);
            mItemPropertiesBox.addProperty(hvc1,{},"");
        }else if(infe_type == "avc1"){
            std::shared_ptr<AvcConfigurationBox> avc1 = std::make_shared<AvcConfigurationBox>();
            avc1->setType(codec_config_type);
            AvcDecoderConfigurationRecord avc;
            avc.parseConfig(alpha_item_codec_config);
            avc1->setConfiguration(avc);
            mItemPropertiesBox.addProperty(avc1, {}, "");
        }else if(infe_type == "vvc1"){
            std::shared_ptr<VvcConfigurationBox> vvcC = std::make_shared<VvcConfigurationBox>();
            VvcDecoderConfigurationRecord vvCR;
            conf_vals.item_id = 2;
            vvCR.parseCompactConfig(alpha_item_codec_config, conf_vals);
            if(vvCR.getMultiLayerFlag() && !mMultilayer){
                //Is it possible for main item to be multilayer and alpha being a separate item?
                logWarning() << "Warning: Mismatch in config multilayerflags" << '\n';
                mMultilayer = true;
            }
            vvcC->setConfiguration(vvCR);
            mItemPropertiesBox.addProperty(vvcC,{},"");

            //See main-item branch above for rationale.  Alpha is item id 2.
            if (vvCR.getMultiLayerFlag())
            {
                mNalUnitCounts[2] = countLengthPrefixedNalUnits(
                    bitstr.getStorage(),
                    alpha_item_byte_offset,
                    alpha_item_data_size,
                    vvCR.getLengthSize());
            }
        }else{  //unrecognized/unsupported config

            ///@todo add passing of raw configs for spec codecs, which have no implementation

            if(explicit_codec_types_flag){  //Allow passing of config as raw, no format defined
                logWarning() << "Warning: Unknown explicit codec: " << infe_type.getString() << '\n';;
            }else{
                //if not explicit, minor brand should've defined it, and it should be supported
                logWarning() << "Warning: Unsupported codec: " << infe_type.getString() << '\n';
            }
            std::shared_ptr<RawPropertyBox> rawConf = std::make_shared<RawPropertyBox>();
            rawConf->setType(codec_config_type);
            rawConf->setData(alpha_item_codec_config);
            mItemPropertiesBox.addProperty(rawConf,{},"");
        }
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #7 - Alpha Item Auxiliary Type
    if(alpha_item_data_size != 0){
        std::shared_ptr<AuxiliaryTypeProperty> auxi = std::make_shared<AuxiliaryTypeProperty>();
        auxi->setAuxType("urn:mpeg:mpegB:cicp:systems:auxiliary:alpha");
        mItemPropertiesBox.addProperty(auxi,{}, "");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #8 - Alpha Item Pixel Information
    if(alpha_item_data_size != 0){
        std::shared_ptr<PixelInformationProperty> alpha_pixi = std::make_shared<PixelInformationProperty>();
        alpha_pixi->setVersion(0);alpha_pixi->setFlags(1);

        PixelInformationProperty::ChannelInfo alphaChannelInfos;
        uint8_t alpha_main_components = 0;
        uint8_t alpha_bitdepth_per_channel = 8;
        if(float_flag == 1){
            alpha_bitdepth_per_channel = 1 << (bit_depth_log2_minus4 + 4);
        }else if(high_bit_depth_flag){
            alpha_bitdepth_per_channel = bit_depth_minus9 + 9;
        }

        alpha_pixi->setNumChannels(alpha_main_components + 1);


        // alpha_channel_idc is implicit from the auxiliary type info box
        uint8_t alpha_component_format=float_flag;
        bool alpha_subsampling_flag = true; //assumed to be true, although not specifically defined in spec
        bool alpha_channel_label_flag=false;
        uint8_t alpha_subsampling_type=0;
        uint8_t alpha_subsampling_location=0;
        Vector<uint8_t> alpha_bpc;

        for(int i=0;i<alpha_pixi->getNumChannels();i++){
            alpha_bpc.push_back(alpha_bitdepth_per_channel);
        }

        alpha_pixi->setBitsPerChannels(alpha_bpc);

        alphaChannelInfos.ComponentFormat = alpha_component_format;
        alphaChannelInfos.SubSamplingFlag = alpha_subsampling_flag;
        alphaChannelInfos.ChannelLabelFlag = alpha_channel_label_flag;
        alphaChannelInfos.SubSamplingType = alpha_subsampling_type;
        alphaChannelInfos.SubSamplingLocation = alpha_subsampling_location;

        std::map<uint8_t,PixelInformationProperty::ChannelInfo> m{{static_cast<uint8_t>(5),alphaChannelInfos}};
        alpha_pixi->setChannelData(m);

        mItemPropertiesBox.addProperty(alpha_pixi,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #9 Image Rotation
    std::shared_ptr<ImageRotation> irot = std::make_shared<ImageRotation>();
    bool addirot = false;
    switch(orientation_minus1){
        case 0: break;  //normal
        case 1: break;  //mirror horizontal
        case 2: irot->setAngle(180);addirot=true; break; //rotate 180 CW
        case 3: break;  //mirror vertical
        case 4: irot->setAngle(270);addirot=true; break; //mirror horizontal and rotate 270 CW
        case 5: irot->setAngle(90);addirot=true; break;    //rotate 90 CW
        case 6: irot->setAngle(90);addirot=true; break; //mirror horizontal and rotate 90 CW
        case 7: irot->setAngle(270);addirot=true; break;
        default: return HEIF_FAIL(HEIF::ErrorCode::INVALID_EXIF_ORIENTATION_VALUE,
                                HEIF::ErrorOrigin::Reader_Mini,
                                "parseMiniBox: orientation_minus1 out of range for irot mapping"); break;
    }
    if(addirot){mItemPropertiesBox.addProperty(irot,{},"");}
    else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #10 Image Mirroring
    std::shared_ptr<ImageMirror> imir = std::make_shared<ImageMirror>();
    bool addimir = false;

    switch(orientation_minus1){
        case 0: break;  // Exif 1: normal
        case 1: imir->setHorizontalAxis(false); addimir=true; break;  // Exif 2: flip horizontal (vertical axis)
        case 2: break;  // Exif 3: rotate 180
        case 3: imir->setHorizontalAxis(true); addimir=true; break;   // Exif 4: flip vertical (horizontal axis)
        case 4: imir->setHorizontalAxis(true); addimir=true; break;   // Exif 5: transpose
        case 5: break;  // Exif 6: rotate 90 CW
        case 6: imir->setHorizontalAxis(true); addimir=true; break;   // Exif 7: transverse
        case 7: break;  // Exif 8: rotate 270 CW
        default: return HEIF_FAIL(HEIF::ErrorCode::INVALID_EXIF_ORIENTATION_VALUE,
                                HEIF::ErrorOrigin::Reader_Mini,
                                "parseMiniBox: orientation_minus1 out of range for imir mapping"); break;
    }
    if(addimir){mItemPropertiesBox.addProperty(imir,{},"");}
    else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

     //Item Property #11 Main Item Content Light Level
    if(clli_flag){
        mItemPropertiesBox.addProperty(clli,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #12 Main Item Mastering display colour volume
    if(mdcv_flag){
        mItemPropertiesBox.addProperty(mdcv,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #13 Main Item Content colour volume
    if(cclv_flag){
        mItemPropertiesBox.addProperty(cclv,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #14 Main Item Ambient Viewing Environment
    if(amve_flag){
        mItemPropertiesBox.addProperty(amve,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #15 Main Item Reference Viewing Environment
    if(reve_flag){
        mItemPropertiesBox.addProperty(reve,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #16 Main Item Nominal Diffuse White
    if(ndwt_flag){
        mItemPropertiesBox.addProperty(ndwt,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #17 Gainmap Item Codec Config
    if(gainmap_item_codec_config_size != 0){
        if(infe_type == "hvc1"){
            std::shared_ptr<HevcConfigurationBox> hvc1 = std::make_shared<HevcConfigurationBox>();
            hvc1->setType(codec_config_type);
            HevcDecoderConfigurationRecord hevc;
            hevc.parseConfig(gainmap_item_codec_config);
            hvc1->setConfiguration(hevc);
            mItemPropertiesBox.addProperty(hvc1,{},"");
        }else if(infe_type == "avc1"){
            std::shared_ptr<AvcConfigurationBox> avc1 = std::make_shared<AvcConfigurationBox>();
            avc1->setType(codec_config_type);
            AvcDecoderConfigurationRecord avc;
            avc.parseConfig(gainmap_item_codec_config);
            avc1->setConfiguration(avc);
            mItemPropertiesBox.addProperty(avc1, {}, "");
        }else if(infe_type == "vvc1"){
            std::shared_ptr<VvcConfigurationBox> vvcC = std::make_shared<VvcConfigurationBox>();
            VvcDecoderConfigurationRecord vvCR;
            conf_vals.item_id = 3;
            vvCR.parseCompactConfig(gainmap_item_codec_config, conf_vals);
            if(vvCR.getMultiLayerFlag() && !mMultilayer){
                //Is it possible for main item to be multilayer and alpha being a separate item?
                logWarning() << "Warning: Mismatch in config multilayerflags" << '\n';
                mMultilayer = true;
            }
            vvcC->setConfiguration(vvCR);
            mItemPropertiesBox.addProperty(vvcC,{},"");
        }else if(infe_type == "jpeg"){
            std::shared_ptr<JpegConfigurationBox> jpeg1 = std::make_shared<JpegConfigurationBox>();
            jpeg1->parseBox(gainmap_item_codec_config);
            mItemPropertiesBox.addProperty(jpeg1,{},"");
        }else{  //unrecognized/unsupported config

            if(explicit_codec_types_flag){  //Allow passing of config as raw, no format defined
                logWarning() << "Warning: Unknown explicit codec: " << infe_type.getString() << '\n';;
            }else{
                //if not explicit, minor brand should've defined it, and it should be supported
                logWarning() << "Warning: Unsupported codec: " << infe_type.getString() << '\n';
            }
            std::shared_ptr<RawPropertyBox> rawConf = std::make_shared<RawPropertyBox>();
            rawConf->setType(codec_config_type);
            rawConf->setData(gainmap_item_codec_config);
            mItemPropertiesBox.addProperty(rawConf,{},"");
        }
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #18 Gainmap Image Spatial Extents
    if(gainmap_item_data_size != 0){
        std::shared_ptr<ImageSpatialExtentsProperty> gainmap_ispe = std::make_shared<ImageSpatialExtentsProperty>();
        gainmap_ispe->setDisplayWidth(gainmap_width_minus1 + 1);
        gainmap_ispe->setDisplayHeight(gainmap_height_minus1 + 1);
        mItemPropertiesBox.addProperty(gainmap_ispe,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

        //Item Property #19 Gainmap Pixel Information
        if(gainmap_item_data_size != 0){
            std::shared_ptr<PixelInformationProperty> gainmap_pixi = std::make_shared<PixelInformationProperty>();
            gainmap_pixi->setVersion(0);gainmap_pixi->setFlags(1);

            uint8_t gainmap_main_components;
            if(gainmap_chroma_subsampling==0){gainmap_main_components = 1;}else{gainmap_main_components = 3;}
            
            uint8_t gainmap_bpc = 8;
            if(gainmap_float_flag == 1){
                gainmap_bpc = 1 << (gainmap_bit_depth_log2_minus4 + 4);
            }else if(gainmap_high_bit_depth_flag){
                gainmap_bpc = gainmap_bit_depth_minus9 + 9;
            }

            gainmap_pixi->setNumChannels(gainmap_main_components);

            Vector<uint8_t> gainmap_pixinput;
            for(int i=0;i<gainmap_pixi->getNumChannels();i++){
                gainmap_pixinput.push_back(gainmap_bpc);
            }
            gainmap_pixi->setBitsPerChannels(gainmap_pixinput);

            std::map<uint8_t, PixelInformationProperty::ChannelInfo> gainmapChannels;

            if(gainmap_main_components > 0){    //entry if main_components > 0
                PixelInformationProperty::ChannelInfo firstChannelInfo;
                firstChannelInfo.ComponentFormat = gainmap_float_flag;
                firstChannelInfo.ChannelLabelFlag = false;
                firstChannelInfo.SubSamplingType = 0;
                firstChannelInfo.SubSamplingLocation = 0;
                gainmapChannels.emplace(static_cast<uint8_t>(2), firstChannelInfo);
            }
            if(gainmap_main_components > 1){    //entry if main_components > 1
                for(int i = 0;i<=1;i++){
                    PixelInformationProperty::ChannelInfo chromaChannelInfo;
                    chromaChannelInfo.ComponentFormat = gainmap_float_flag;
                    chromaChannelInfo.SubSamplingFlag = true;

                    uint8_t subsampling_type = 0;
                    if(gainmap_chroma_subsampling == 1){
                        subsampling_type = 2;
                    }else if(gainmap_chroma_subsampling == 2){
                        subsampling_type = 1;
                    }else if(gainmap_chroma_subsampling == 3){
                        subsampling_type = 0;
                    }

                    uint8_t subsampling_location = 0;
                    if(gainmap_chroma_is_horizontally_centered && gainmap_chroma_is_vertically_centered){
                        subsampling_location = 1;
                    }else if(gainmap_chroma_is_horizontally_centered && !gainmap_chroma_is_vertically_centered){
                        subsampling_location = 3;
                    }else if(!gainmap_chroma_is_horizontally_centered && gainmap_chroma_is_vertically_centered){
                        subsampling_location = 0;
                    }else if(!gainmap_chroma_is_horizontally_centered && !gainmap_chroma_is_vertically_centered){
                        subsampling_location = 2;
                    }

                    chromaChannelInfo.SubSamplingType = subsampling_type;
                    chromaChannelInfo.SubSamplingLocation = subsampling_location;

                    gainmapChannels.emplace(static_cast<uint8_t>(3+i), chromaChannelInfo);
                }
            }

            gainmap_pixi->setChannelData(gainmapChannels);

            mItemPropertiesBox.addProperty(gainmap_pixi, {},"");
        }else{  //Placeholder FreeSpaceBox
            std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
            mItemPropertiesBox.addProperty(free, {},"");
        }

    //Item Property #20 Gainmap Colour Information
    if(gainmap_item_data_size != 0){
        std::shared_ptr<ColourInformationBox> colr3 = std::make_shared<ColourInformationBox>();
        colr3->setColourType("nclx");colr3->setColourPrimaries(2);
        colr3->setTransferCharacteristics(2);
        colr3->setMatrixCoefficients(gainmap_matrix_coefficients);
        colr3->setFullRangeFlag(gainmap_full_range_flag);
        
        mItemPropertiesBox.addProperty(colr3, {},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #21 Gainmap Image Spatial Extents with irot taken into account
    if(gainmap_flag){
        std::shared_ptr<ImageSpatialExtentsProperty> gainmap_ispe2 = std::make_shared<ImageSpatialExtentsProperty>();
        if(orientation_minus1 == 0 || orientation_minus1 == 1||
                orientation_minus1 == 2 || orientation_minus1 == 3){
            gainmap_ispe2->setDisplayWidth(width_minus1 + 1); gainmap_ispe2->setDisplayHeight(height_minus1 + 1);
        }else{
            gainmap_ispe2->setDisplayWidth(height_minus1 + 1); gainmap_ispe2->setDisplayHeight(width_minus1 + 1);
        }
        mItemPropertiesBox.addProperty(gainmap_ispe2,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #22 Tonemap "nclx" Colour Information
    if(gainmap_flag && (tmap_explicit_cicp_flag || !tmap_icc_flag)){
            std::shared_ptr<ColourInformationBox> colr4 = std::make_shared<ColourInformationBox>();
            colr4->setColourType("nclx");
            colr4->setColourPrimaries(tmap_colour_primaries);
            colr4->setTransferCharacteristics(tmap_transfer_characteristics);
            colr4->setMatrixCoefficients(tmap_matrix_coefficients);
            colr4->setFullRangeFlag(tmap_full_range_flag);
            mItemPropertiesBox.addProperty(colr4,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #23 Tonemap "prof" Colour Information
    if(gainmap_flag && tmap_icc_flag){
        std::shared_ptr<ColourInformationBox> colr5 = std::make_shared<ColourInformationBox>();
        colr5->setColourType("prof");
        colr5->setIccProfile(tmap_icc_data);

        mItemPropertiesBox.addProperty(colr5,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #24 Tonemap Content Light Level Information
    if(tmap_clli_flag){
        mItemPropertiesBox.addProperty(tmap_clli,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #25 Tonemap Mastering Display Colour Volume
    if(tmap_mdcv_flag){
        mItemPropertiesBox.addProperty(tmap_mdcv,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #26 Tonemap Content Colour Volume
    if(tmap_cclv_flag){
        mItemPropertiesBox.addProperty(tmap_cclv,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #27 Tonemap Ambient Viewing Environment
    if(tmap_amve_flag){
        mItemPropertiesBox.addProperty(tmap_amve,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #28 Tonemap Reference Viewing Environment
    if(tmap_reve_flag){
        mItemPropertiesBox.addProperty(tmap_reve,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #29 Tonemap Nominal Diffuse white
    if(tmap_ndwt_flag){
        mItemPropertiesBox.addProperty(tmap_ndwt,{},"");
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Property #30 Alpha Information
    if(alpha_flag && alpha_item_data_size != 0){
        //Not supported
    }else{  //Placeholder FreeSpaceBox
        std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
        mItemPropertiesBox.addProperty(free, {},"");
    }

    //Item Properties #31 & #32 (Reserved)
    std::shared_ptr<FreeSpaceBox> free = std::make_shared<FreeSpaceBox>();
    mItemPropertiesBox.addProperty(free, {},"");
    std::shared_ptr<FreeSpaceBox> free2 = std::make_shared<FreeSpaceBox>();
    mItemPropertiesBox.addProperty(free2, {},"");

    //Associating Item Properties

    //Item 1 Association
    mItemPropertiesBox.associateProperty(1, {1}, true);
    mItemPropertiesBox.associateProperty(2, {1}, false);
    mItemPropertiesBox.associateProperty(3, {1}, false);
    mItemPropertiesBox.associateProperty(4, {1}, true);
    mItemPropertiesBox.associateProperty(5, {1}, true);

    if(alpha_flag && alpha_item_data_size == 0){
        mItemPropertiesBox.associateProperty(30, {1}, true);
    }

    if(hdr_flag){
        mItemPropertiesBox.associateProperty(11, {1}, false);
        mItemPropertiesBox.associateProperty(12, {1}, false);
        mItemPropertiesBox.associateProperty(13, {1}, false);
        mItemPropertiesBox.associateProperty(14, {1}, false);
        mItemPropertiesBox.associateProperty(15, {1}, false);
        mItemPropertiesBox.associateProperty(16, {1}, false);
    }
    mItemPropertiesBox.associateProperty(9, {1}, false);
    mItemPropertiesBox.associateProperty(10, {1}, false);

    //Item 2 Association
    if(alpha_item_data_size != 0){
        mItemPropertiesBox.associateProperty(6, {2}, true);
        mItemPropertiesBox.associateProperty(2, {2}, false);
        mItemPropertiesBox.associateProperty(7, {2}, true);
        mItemPropertiesBox.associateProperty(8, {2}, false);
        mItemPropertiesBox.associateProperty(9, {2}, true);
        mItemPropertiesBox.associateProperty(10, {2}, true);
    }

    //Item 3 Association
    if(gainmap_flag){
        mItemPropertiesBox.associateProperty(21, {3}, false);
        mItemPropertiesBox.associateProperty(22, {3}, true);
        mItemPropertiesBox.associateProperty(23, {3}, true);
        mItemPropertiesBox.associateProperty(24, {3}, false);
        mItemPropertiesBox.associateProperty(25, {3}, false);
        mItemPropertiesBox.associateProperty(26, {3}, false);
        mItemPropertiesBox.associateProperty(27, {3}, false);
        mItemPropertiesBox.associateProperty(28, {3}, false);
        mItemPropertiesBox.associateProperty(29, {3}, false);
    }

    //Item 4 Association
    if(gainmap_item_data_size != 0){
        mItemPropertiesBox.associateProperty(17, {4}, true);
        mItemPropertiesBox.associateProperty(18, {4}, false);
        mItemPropertiesBox.associateProperty(19, {4}, false);
        mItemPropertiesBox.associateProperty(20, {4}, true);
        mItemPropertiesBox.associateProperty(9, {4}, true);
        mItemPropertiesBox.associateProperty(10, {4}, true);
    }

    //Tonemap Image Metadata
    //not supported
    uint64_t tmap_item_data_size = 0;
    if(gainmap_flag){
        //tmap_item_data is defined as a data chunk of tmap_item_data_size bytes,
        //being gainmap_metadata_size+1 bytes
        tmap_item_data_size = gainmap_metadata_size + 1;
    }

    //Item Location Box
    mItemLocationBox.setVersion(1);//1 or 2

    // ftyp for mini = 4 (size) + 4 ('ftyp') + 4 (major_brand) + 4 (minor_version)
    // = 16 bytes. The mini path never adds compatible brands (mif1 is
    // suppressed at WriterImpl::addImage), so this is exact.
    const uint64_t FTYP_SIZE = 16;

    //Item Location #1
    ItemLocation iloc1; iloc1.setItemID(1);
    iloc1.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
    iloc1.setBaseOffset(FTYP_SIZE + mdat_offset + alpha_item_data_size + gainmap_item_data_size);
    ItemLocationExtent ext1; ext1.mExtentIndex = 0;
    ext1.mExtentLength = main_item_data_size_minus1 + 1;
    iloc1.addExtent(ext1); mItemLocationBox.addLocation(iloc1);


    //Iloc ID2 (Optional) Alpha item data
    if(alpha_item_data_size != 0){
        ItemLocation iloc2;iloc2.setItemID(2);
        iloc2.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
        iloc2.setBaseOffset(FTYP_SIZE + mdat_offset);
        ItemLocationExtent iloc2ext; iloc2ext.mExtentIndex = 0;
        iloc2ext.mExtentOffset = 0;
        iloc2ext.mExtentLength = alpha_item_data_size;
        iloc2.addExtent(iloc2ext);
        mItemLocationBox.addLocation(iloc2);
    }

    //Iloc ID3 (Optional) Tonemap item data -- stored in ItemDataBox
    if(tmap_item_data_size != 0){
        ItemLocation iloc3;iloc3.setItemID(3);
        iloc3.setConstructionMethod(ItemLocation::ConstructionMethod::IDAT_OFFSET);
        ItemLocationExtent ext3; ext3.mExtentIndex = 0;
        ext3.mExtentOffset = 0;
        ext3.mExtentLength = tmap_item_data_size;
        iloc3.addExtent(ext3); mItemLocationBox.addLocation(iloc3);
    }

    //Iloc ID4 (Optional) Gainmap item Data
    if(gainmap_item_data_size != 0){
        ItemLocation iloc4; iloc4.setItemID(4);
        iloc4.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
        iloc4.setBaseOffset(FTYP_SIZE + mdat_offset);
        ItemLocationExtent ext4; ext4.mExtentIndex = 0;
        ext4.mExtentLength = gainmap_item_data_size;
        ext4.mExtentOffset = alpha_item_data_size;
        iloc4.addExtent(ext4); mItemLocationBox.addLocation(iloc4);
    }

    //Iloc ID5

    //Iloc ID6 (Optional) Exif data
    if(exif_flag){
        ItemLocation iloc6; iloc6.setItemID(6);
        iloc6.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
        iloc6.setBaseOffset(FTYP_SIZE + mdat_offset);
        ItemLocationExtent ext6; ext6.mExtentIndex = 0;
        ext6.mExtentLength = exif_data_size_minus1 + 1;
        ext6.mExtentOffset = main_item_data_size_minus1 + 1 + alpha_item_data_size
                                    + gainmap_item_data_size;
        iloc6.addExtent(ext6);mItemLocationBox.addLocation(iloc6);
    }

    //Iloc ID7 (Optional) XMP data
    if(xmp_flag){
        ItemLocation iloc7; iloc7.setItemID(7);
        iloc7.setConstructionMethod(ItemLocation::ConstructionMethod::FILE_OFFSET);
        iloc7.setBaseOffset(FTYP_SIZE + mdat_offset);
        ItemLocationExtent ext7; ext7.mExtentIndex = 0;
        ext7.mExtentLength = xmp_data_size_minus1 + 1;
        ext7.mExtentOffset = main_item_data_size_minus1 + 1 + alpha_item_data_size
                                + gainmap_item_data_size + exif_data_size_minus1 + 1;
        iloc7.addExtent(ext7);mItemLocationBox.addLocation(iloc7);
    }

    return HEIF::ErrorCode::OK;
}
