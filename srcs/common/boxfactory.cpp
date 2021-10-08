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

#include "boxfactory.hpp"

#include "accessibilitytext.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperturebox.hpp"
#include "colourinformationbox.hpp"
#include "creationtimeinformation.hpp"
#include "freespacebox.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagescaling.hpp"
#include "imagespatialextentsproperty.hpp"
#include "jpegconfigurationbox.hpp"
#include "modificationtimeinformation.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "rawpropertybox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "userdescriptionproperty.hpp"

template <class T>
class BoxFactory::BoxWrapper : public BoxFactory::BoxWrapperBase
{
public:
    BoxWrapper() = default;
    FourCCInt getType() const override
    {
        T box;
        return box.getType();
    }

    std::shared_ptr<Box> makeNewBox() override
    {
        return makeCustomShared<T>();
    }
};

BoxFactory::BoxFactory()
{
    static const std::vector<std::shared_ptr<BoxWrapperBase>> BOXWRAPPERS = {
        std::make_shared<BoxWrapper<AccessibilityTextProperty>>(),
        std::make_shared<BoxWrapper<AuxiliaryTypeProperty>>(),
        std::make_shared<BoxWrapper<AvcConfigurationBox>>(),
        std::make_shared<BoxWrapper<CleanApertureBox>>(),
        std::make_shared<BoxWrapper<ColourInformationBox>>(),
        std::make_shared<BoxWrapper<CreationTimeProperty>>(),
        std::make_shared<BoxWrapper<HevcConfigurationBox>>(),
        std::make_shared<BoxWrapper<ImageMirror>>(),
        std::make_shared<BoxWrapper<ImageRelativeLocationProperty>>(),
        std::make_shared<BoxWrapper<ImageRotation>>(),
        std::make_shared<BoxWrapper<ImageScaling>>(),
        std::make_shared<BoxWrapper<ImageSpatialExtentsProperty>>(),
        std::make_shared<BoxWrapper<JpegConfigurationBox>>(),
        std::make_shared<BoxWrapper<ModificationTimeProperty>>(),
        std::make_shared<BoxWrapper<PixelAspectRatioBox>>(),
        std::make_shared<BoxWrapper<PixelInformationProperty>>(),
        std::make_shared<BoxWrapper<RequiredReferenceTypesProperty>>(),
        std::make_shared<BoxWrapper<UserDescriptionProperty>>(),
    };

    for (const auto& boxWrapper : BOXWRAPPERS)
    {
        mBoxWrapperMap.insert({boxWrapper->getType(), boxWrapper});
    }

    // Free box has at least two possible 4ccs
    auto freeBoxFactory = std::make_shared<BoxWrapper<FreeSpaceBox>>();
    mBoxWrapperMap.insert({"skip", freeBoxFactory});
    mBoxWrapperMap.insert({"free", freeBoxFactory});
}

std::shared_ptr<Box> BoxFactory::makeNewBox(const FourCCInt boxType)
{
    auto it = mBoxWrapperMap.find(boxType);
    if (it == mBoxWrapperMap.end())
    {
        return nullptr;
    }
    return it->second->makeNewBox();
}
