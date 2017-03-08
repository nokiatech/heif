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

#include "itempropertycontainer.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperture.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagespatialextentsproperty.hpp"
#include "layeredhevcconfigurationitemproperty.hpp"
#include "layerselectorproperty.hpp"
#include "log.hpp"
#include "operatingpointsinformation.hpp"
#include "targetolsproperty.hpp"

ItemPropertyContainer::ItemPropertyContainer() :
    Box("ipco")
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
    // Check if similar property already exists.
    int index = getIndex(box);

    if (index != -1)
    {
        return index;
    }

    mProperties.push_back(box);
    return mProperties.size() - 1;
}


int ItemPropertyContainer::getIndex(std::shared_ptr<Box> box) const
{
    ImageSpatialExtentsProperty* newIspe = dynamic_cast<ImageSpatialExtentsProperty*>(box.get());
    if (newIspe != nullptr)
    {
        for (size_t i = 0; i < mProperties.size(); ++i)
        {
            ImageSpatialExtentsProperty* ispeCandidate = dynamic_cast<ImageSpatialExtentsProperty*>(mProperties.at(i).get());
            if (ispeCandidate != nullptr)
            {
                if ((ispeCandidate->getDisplayHeight() == newIspe->getDisplayHeight()) &&
                    (ispeCandidate->getDisplayWidth() == newIspe->getDisplayWidth()))
                {
                    return i;
                }
            }
        }
    }
    return -1;
}

void ItemPropertyContainer::writeBox(BitStream& bitstream)
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
            property = std::make_shared<AvcConfigurationBox>();
        }
        else if (boxType == "hvcC")
        {
            property = std::make_shared<HevcConfigurationBox>();
        }
        else if (boxType == "imir")
        {
            property = std::make_shared<ImageMirror>();
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
        else if (boxType == "lhvC")
        {
            property = std::make_shared<LayeredHevcConfigurationItemProperty>();
        }
        else if (boxType == "lsel")
        {
            property = std::make_shared<LayerSelectorProperty>();
        }
        else if (boxType == "oinf")
        {
            property = std::make_shared<OperatingPointsInformation>();
        }
        else if (boxType == "tols")
        {
            property = std::make_shared<TargetOlsProperty>();
        }
        else
        {
            logWarning() << "Skipping an unknown box of type '" << boxType << "' in ItemPropertyContainer." << std::endl;

            /// A placeholder class to be used instead of properties of unknown type
            class UnknownProperty : public Box
            {
            public:
                UnknownProperty(const FourCCInt type) : Box(type) {};
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
