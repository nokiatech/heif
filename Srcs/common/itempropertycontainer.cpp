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

#include "itempropertycontainer.hpp"
#include "auxiliarytypeproperty.hpp"
#include "cleanaperture.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagespatialextentsproperty.hpp"
#include "log.hpp"

ItemPropertyContainer::ItemPropertyContainer() :
    FullBox("ipco", 0, 0)
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


unsigned int ItemPropertyContainer::addProperty(std::shared_ptr<Box> box)
{
    mProperties.push_back(box);
    return mProperties.size();
}


void ItemPropertyContainer::writeBox(BitStream& bitstream)
{
    writeFullBoxHeader(bitstream);
    for (auto& property : mProperties)
    {
        property->writeBox(bitstream);
    }

    updateSize(bitstream);
}


void ItemPropertyContainer::parseBox(BitStream& bitstream)
{
    parseFullBoxHeader(bitstream);
    if (getType() != "ipco")
    {
        logError() << "Reading ipco, found '" << getType() << "' instead." << std::endl;
    }

    // Read as many ItemProperty- or ItemFullProperty -derived boxes as there is
    while (bitstream.numBytesLeft() > 0)
    {
        std::string boxType;
        BitStream subBitStream = bitstream.readSubBoxBitStream(boxType);

        std::shared_ptr<Box> property;
        if (boxType == "hvcC")
        {
            property = std::make_shared<HevcConfigurationBox>();
        }
        else if (boxType == "ispe")
        {
            property = std::make_shared<ImageSpatialExtentsProperty>();
        }
        else if (boxType == "irot")
        {
            property = std::make_shared<ImageRotation>();
        }
        else if (boxType == "rloc")
        {
            property = std::make_shared<ImageRelativeLocationProperty>();
        }
        else if (boxType == "clap")
        {
            property = std::make_shared<CleanAperture>();
        }
        else if (boxType == "auxC")
        {
            property = std::make_shared<AuxiliaryTypeProperty>();
        }
        else
        {
            logWarning() << "Skipping an unknown box of type '" << boxType << "' in ItemPropertyContainer." << std::endl;

            /// A placeholder class to be used instead of properties of unknown type
            class UnknownProperty : public Box
            {
            public:
                UnknownProperty(const std::string& type) : Box(type.c_str()) {};
                virtual ~UnknownProperty() = default;
                virtual void writeBox(BitStream& /*bitstream*/) {};
                virtual void parseBox(BitStream& /*bitstream*/) {};
            };
            property = std::make_shared<UnknownProperty>(boxType);
        }
        property->parseBox(subBitStream);
        mProperties.push_back(property);
    }
}
