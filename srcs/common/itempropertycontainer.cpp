/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "itempropertycontainer.hpp"

#include "boxfactory.hpp"
#include "log.hpp"
#include "rawpropertybox.hpp"

ItemPropertyContainer::ItemPropertyContainer()
    : Box("ipco")
{
}

const Box* ItemPropertyContainer::getProperty(const size_t index) const
{
    if (mProperties.size() <= index)
    {
        return nullptr;
    }
    return mProperties.at(index).get();
}

Box* ItemPropertyContainer::getProperty(const size_t index)
{
    return const_cast<Box*>(static_cast<const ItemPropertyContainer*>(this)->getProperty(index));
}

std::uint16_t ItemPropertyContainer::addProperty(const std::shared_ptr<Box>& box)
{
    mProperties.push_back(box);
    return static_cast<std::uint16_t>(mProperties.size());
}

void ItemPropertyContainer::writeBox(BitStream& bitstream) const
{
    writeBoxHeader(bitstream);
    for (auto& property : mProperties)
    {
        property->writeBox(bitstream);
    }

    updateSize(bitstream);
}

void ItemPropertyContainer::parseBox(BitStream& bitstream)
{
    parseBoxHeader(bitstream);
    if (getType() != "ipco")
    {
        logError() << "Reading ipco, found '" << getType().getString() << "' instead." << std::endl;
    }

    BoxFactory boxFactory;

    while (bitstream.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitStream        = bitstream.readSubBoxBitStream(boxType);
        std::shared_ptr<Box> property = boxFactory.makeNewBox(boxType);
        if (property == nullptr)
        {
            property = std::make_shared<RawPropertyBox>();
        }
        property->parseBox(subBitStream);
        mProperties.push_back(property);
    }
}
