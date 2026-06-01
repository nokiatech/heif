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

#include "boxfactory.hpp"
#include "customallocator.hpp"

#include "accessibilitytext.hpp"
#include "ambientviewingenvironmentbox.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "cleanaperturebox.hpp"
#include "colourinformationbox.hpp"
#include "contentcolourvolumebox.hpp"
#include "contentlightlevelbox.hpp"
#include "creationtimeinformation.hpp"
#include "freespacebox.hpp"
#include "hevcconfigurationbox.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagescaling.hpp"
#include "imagespatialextentsproperty.hpp"
#include "jpegconfigurationbox.hpp"
#include "layerselectorproperty.hpp"
#include "lhevcconfigurationbox.hpp"
#include "masteringdisplaycolourvolumebox.hpp"
#include "modificationtimeinformation.hpp"
#include "nominaldiffusewhitebox.hpp"
#include "operatingpointsinformationproperty.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "referenceviewingenvironmentbox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "subsampleinformationproperty.hpp"
#include "targetolsproperty.hpp"
#include "userdescriptionproperty.hpp"
#include "vvcconfigurationbox.hpp"
#include <utility>
#include <vector>

// C6262: MSVC /analyze reports ~60 KB stack for some BoxWrapper<T>
// instantiations because getType() constructs a temporary T on the stack.
// The largest T (e.g. OperatingPointsInformationProperty) is still well
// within the 1 MB default stack and the cost is transient.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 6262)
#endif
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
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

BoxFactory::BoxFactory()
{
    static const std::vector<std::shared_ptr<BoxWrapperBase>> BOXWRAPPERS = {
        std::make_shared<BoxWrapper<AccessibilityTextProperty>>(),
        std::make_shared<BoxWrapper<AmbientViewingEnvironmentBox>>(),
        std::make_shared<BoxWrapper<AuxiliaryTypeProperty>>(),
        std::make_shared<BoxWrapper<AvcConfigurationBox>>(),
        std::make_shared<BoxWrapper<CleanApertureBox>>(),
        std::make_shared<BoxWrapper<ColourInformationBox>>(),
        std::make_shared<BoxWrapper<ContentColourVolumeBox>>(),
        std::make_shared<BoxWrapper<ContentLightLevelBox>>(),
        std::make_shared<BoxWrapper<CreationTimeProperty>>(),
        std::make_shared<BoxWrapper<HevcConfigurationBox>>(),
        std::make_shared<BoxWrapper<ImageMirror>>(),
        std::make_shared<BoxWrapper<ImageRelativeLocationProperty>>(),
        std::make_shared<BoxWrapper<ImageRotation>>(),
        std::make_shared<BoxWrapper<ImageScaling>>(),
        std::make_shared<BoxWrapper<ImageSpatialExtentsProperty>>(),
        std::make_shared<BoxWrapper<JpegConfigurationBox>>(),
        std::make_shared<BoxWrapper<LayerSelectorProperty>>(),
        std::make_shared<BoxWrapper<LhevcConfigurationBox>>(),
        std::make_shared<BoxWrapper<MasteringDisplayColourVolumeBox>>(),
        std::make_shared<BoxWrapper<ModificationTimeProperty>>(),
        std::make_shared<BoxWrapper<NominalDiffuseWhiteBox>>(),
        std::make_shared<BoxWrapper<OperatingPointsInformationProperty>>(),
        std::make_shared<BoxWrapper<PixelAspectRatioBox>>(),
        std::make_shared<BoxWrapper<PixelInformationProperty>>(),
        std::make_shared<BoxWrapper<ReferenceViewingEnvironmentBox>>(),
        std::make_shared<BoxWrapper<RequiredReferenceTypesProperty>>(),
        std::make_shared<BoxWrapper<SubSampleInformationProperty>>(),
        std::make_shared<BoxWrapper<TargetOlsProperty>>(),
        std::make_shared<BoxWrapper<UserDescriptionProperty>>(),
        std::make_shared<BoxWrapper<VvcConfigurationBox>>(),
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
