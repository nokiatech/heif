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

#include "itempropertiesbox.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagespatialextentsproperty.hpp"
#include "log.hpp"

ItemPropertiesBox::ItemPropertiesBox() :
    Box("iprp")
{
}

std::uint32_t ItemPropertiesBox::findPropertyIndex(const PropertyType type, const std::uint32_t itemId) const
{
    const ItemPropertyAssociation::AssociationEntries propertyIndexVector = mAssociations.getAssociationEntries(itemId);
    for (const auto& entry : propertyIndexVector)
    {
        const Box* property = mContainer.getProperty(entry.index - 1);
        if (getPropertyType(property) == type)
        {
            return entry.index;
        }
    }
    return 0;
}

ItemPropertiesBox::PropertyType ItemPropertiesBox::getPropertyType(const Box* property) const
{
    PropertyType type = PropertyType::UNKNOWN;
    FourCCInt boxType = property->getType();
    static const std::map<FourCCInt, PropertyType> NAME_TO_ENUM_MAP =
    {
        { "auxC", PropertyType::AUXC },
        { "avcC", PropertyType::AVCC },
        { "clap", PropertyType::CLAP },
        { "hvcC", PropertyType::HVCC },
        { "imir", PropertyType::IMIR },
        { "irot", PropertyType::IROT },
        { "ispe", PropertyType::ISPE },
        { "lhvC", PropertyType::LHVC },
        { "lsel", PropertyType::LSEL },
        { "oinf", PropertyType::OINF },
        { "rloc", PropertyType::RLOC },
        { "tols", PropertyType::TOLS }
    };

    if (NAME_TO_ENUM_MAP.count(boxType))
    {
        type = NAME_TO_ENUM_MAP.at(boxType);
    }

    return type;
}


ItemPropertiesBox::PropertyInfos ItemPropertiesBox::getItemProperties(const std::uint32_t itemId) const
{
    PropertyInfos propertyInfoVector;
    const ItemPropertyAssociation::AssociationEntries associations = mAssociations.getAssociationEntries(itemId);

    for (const auto& entry : associations)
    {
        PropertyInfo propertyInfo;
        const Box* property = mContainer.getProperty(entry.index - 1);
        propertyInfo.type = getPropertyType(property);
        propertyInfo.index = entry.index - 1;
        propertyInfo.essential = entry.essential;
        propertyInfoVector.push_back(propertyInfo);
    }

    return propertyInfoVector;
}


void ItemPropertiesBox::addProperty(std::shared_ptr<Box> box, const std::vector<std::uint32_t>& itemIds, const bool essential)
{
    const unsigned int propertyIndex = mContainer.addProperty(box) + 1;
    for (const auto itemId : itemIds)
    {
        mAssociations.addEntry(itemId, propertyIndex, essential);
    }
}


void ItemPropertiesBox::associateProperty(const unsigned int index, const std::vector<std::uint32_t>& itemIds, const bool essential)
{
    for (const auto itemId : itemIds)
    {
        mAssociations.addEntry(itemId, index, essential);
    }
}


void ItemPropertiesBox::writeBox(BitStream& output)
{
    writeBoxHeader(output);

    mContainer.writeBox(output);
    mAssociations.writeBox(output);

    updateSize(output);
}


void ItemPropertiesBox::parseBox(BitStream& input)
{
    parseBoxHeader(input);

    FourCCInt subBoxType;
    BitStream subBitStream = input.readSubBoxBitStream(subBoxType);
    mContainer.parseBox(subBitStream);

    subBitStream = input.readSubBoxBitStream(subBoxType);
    if (subBoxType != "ipma")
    {
        throw std::runtime_error("ItemPropertiesBox includes a box which is not ipma");
    }

    // There could be several ItemPropertyAssociation boxes, but currently only one is supported.
    mAssociations.parseBox(subBitStream);

    if (input.numBytesLeft() > 0)
    {
        logWarning() << "ItemPropertiesBox::parseBox() supports currently only one ipma box, but there seems to  be more." << std::endl;
    }
}
