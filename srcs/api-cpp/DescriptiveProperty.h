/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <ItemProperty.h>
#include "heifcommondatatypes.h"
#include <cstdint>
#include <string>
#include <vector>

namespace HEIFPP
{
    class DescriptiveProperty : public HEIFPP::ItemProperty
    {
    public:
        ~DescriptiveProperty() override = default;

    protected:
        DescriptiveProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType, const HEIF::FourCC& aRawType);

    protected:
        // valid types: 'pasp','colr','pixi','rloc','auxc','lsel','tols','oinf','subs'

        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;

    public:
        DescriptiveProperty& operator=(const DescriptiveProperty&) = delete;
        DescriptiveProperty& operator=(DescriptiveProperty&&) = delete;
        DescriptiveProperty(const DescriptiveProperty&)       = delete;
        DescriptiveProperty(DescriptiveProperty&&)            = delete;
        DescriptiveProperty()                                 = delete;
    };

    class PixelAspectRatioProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelAspectRatio mPixelAspectRatio;
        explicit PixelAspectRatioProperty(Heif* aHeif);
        ~PixelAspectRatioProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        PixelAspectRatioProperty& operator=(const PixelAspectRatioProperty&) = delete;
        PixelAspectRatioProperty& operator=(PixelAspectRatioProperty&&) = delete;
        PixelAspectRatioProperty(const PixelAspectRatioProperty&)       = delete;
        PixelAspectRatioProperty(PixelAspectRatioProperty&&)            = delete;
        PixelAspectRatioProperty()                                      = delete;
    };

    class ColourInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::ColourInformation mColourInformation;
        explicit ColourInformationProperty(Heif* aHeif);
        ~ColourInformationProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        ColourInformationProperty& operator=(const ColourInformationProperty&) = delete;
        ColourInformationProperty& operator=(ColourInformationProperty&&) = delete;
        ColourInformationProperty(const ColourInformationProperty&)       = delete;
        ColourInformationProperty(ColourInformationProperty&&)            = delete;
        ColourInformationProperty()                                       = delete;
    };

    class PixelInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelInformation mPixelInformation;
        explicit PixelInformationProperty(Heif* aHeif);
        ~PixelInformationProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        PixelInformationProperty& operator=(const PixelInformationProperty&) = delete;
        PixelInformationProperty& operator=(PixelInformationProperty&&) = delete;
        PixelInformationProperty(const PixelInformationProperty&)       = delete;
        PixelInformationProperty(PixelInformationProperty&&)            = delete;
        PixelInformationProperty()                                      = delete;
    };

    class RelativeLocationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::RelativeLocation mRelativeLocation;
        explicit RelativeLocationProperty(Heif* aHeif);
        ~RelativeLocationProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        RelativeLocationProperty& operator=(const RelativeLocationProperty&) = delete;
        RelativeLocationProperty& operator=(RelativeLocationProperty&&) = delete;
        RelativeLocationProperty(const RelativeLocationProperty&)       = delete;
        RelativeLocationProperty(RelativeLocationProperty&&)            = delete;
        RelativeLocationProperty()                                      = delete;
    };

    class AuxiliaryProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        explicit AuxiliaryProperty(Heif* aHeif);
        ~AuxiliaryProperty() override = default;
        const std::string& auxType();
        void auxType(const std::string&);
        const std::vector<std::uint8_t>& subType();
        void subType(const std::vector<std::uint8_t>& aType);

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        std::string mAuxType;  ///< Type of the associated auxiliary image item. This is not null-terminated.
        std::vector<std::uint8_t> mSubType;  ///< Aux subtype, semantics depends on the auxType value
    public:
        AuxiliaryProperty& operator=(const AuxiliaryProperty&) = delete;
        AuxiliaryProperty& operator=(AuxiliaryProperty&&) = delete;
        AuxiliaryProperty(const AuxiliaryProperty&)       = delete;
        AuxiliaryProperty(AuxiliaryProperty&&)            = delete;
        AuxiliaryProperty()                               = delete;
    };

    class LayerSelectorProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::LayerSelector mLayerSelector;
        explicit LayerSelectorProperty(Heif* aHeif);
        ~LayerSelectorProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        LayerSelectorProperty& operator=(const LayerSelectorProperty&) = delete;
        LayerSelectorProperty& operator=(LayerSelectorProperty&&) = delete;
        LayerSelectorProperty(const LayerSelectorProperty&)       = delete;
        LayerSelectorProperty(LayerSelectorProperty&&)            = delete;
        LayerSelectorProperty()                                   = delete;
    };

    class TargetOlsProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::TargetOls mTargetOls;
        explicit TargetOlsProperty(Heif* aHeif);
        ~TargetOlsProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        TargetOlsProperty& operator=(const TargetOlsProperty&) = delete;
        TargetOlsProperty& operator=(TargetOlsProperty&&) = delete;
        TargetOlsProperty(const TargetOlsProperty&)       = delete;
        TargetOlsProperty(TargetOlsProperty&&)            = delete;
        TargetOlsProperty()                               = delete;
    };

    class OperatingPointsProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::OperatingPointsInfo mOperatingPoints;
        explicit OperatingPointsProperty(Heif* aHeif);
        ~OperatingPointsProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        OperatingPointsProperty& operator=(const OperatingPointsProperty&) = delete;
        OperatingPointsProperty& operator=(OperatingPointsProperty&&) = delete;
        OperatingPointsProperty(const OperatingPointsProperty&)       = delete;
        OperatingPointsProperty(OperatingPointsProperty&&)            = delete;
        OperatingPointsProperty()                                     = delete;
    };

    class SubSampleProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::SubSampleInfo mSubSampleInfo;
        explicit SubSampleProperty(Heif* aHeif);
        ~SubSampleProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        SubSampleProperty& operator=(const SubSampleProperty&) = delete;
        SubSampleProperty& operator=(SubSampleProperty&&) = delete;
        SubSampleProperty(const SubSampleProperty&)       = delete;
        SubSampleProperty(SubSampleProperty&&)            = delete;
        SubSampleProperty()                               = delete;
    };
}  // namespace HEIFPP
