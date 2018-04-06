/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperturebox.hpp"
#include "colourinformationbox.hpp"
#include "freespacebox.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagespatialextentsproperty.hpp"
#include "jpegconfigurationbox.hpp"
#include "log.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
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

std::uint16_t ItemPropertyContainer::addProperty(std::shared_ptr<Box> box)
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
        logError() << "Reading ipco, found '" << getType() << "' instead." << std::endl;
    }

    // Read as many ItemProperty- or ItemFullProperty -derived boxes as there is
    while (bitstream.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitStream = bitstream.readSubBoxBitStream(boxType);

        std::shared_ptr<Box> property;
        if (boxType == "avcC")
        {
            property = makeCustomShared<AvcConfigurationBox>();
        }
        else if (boxType == "hvcC")
        {
            property = makeCustomShared<HevcConfigurationBox>();
        }
        else if (boxType == "jpgC")
        {
            property = makeCustomShared<JpegConfigurationBox>();
        }
        else if (boxType == "imir")
        {
            property = makeCustomShared<ImageMirror>();
        }
        else if (boxType == "ispe")
        {
            property = makeCustomShared<ImageSpatialExtentsProperty>();
        }
        else if (boxType == "irot")
        {
            property = makeCustomShared<ImageRotation>();
        }
        else if (boxType == "rloc")
        {
            property = makeCustomShared<ImageRelativeLocationProperty>();
        }
        else if (boxType == "clap")
        {
            property = makeCustomShared<CleanApertureBox>();
        }
        else if (boxType == "auxC")
        {
            property = makeCustomShared<AuxiliaryTypeProperty>();
        }
        else if (boxType == "pixi")
        {
            property = makeCustomShared<PixelInformationProperty>();
        }
        else if (boxType == "colr")
        {
            property = makeCustomShared<ColourInformationBox>();
        }
        else if (boxType == "pasp")
        {
            property = makeCustomShared<PixelAspectRatioBox>();
        }
        else if (boxType == "free" || boxType == "skip")
        {
            property = makeCustomShared<FreeSpaceBox>();
        }
        else
        {
            // Read unknown properties as binary blobs.
            property = makeCustomShared<RawPropertyBox>();
        }
        property->parseBox(subBitStream);
        mProperties.push_back(property);
    }
}
