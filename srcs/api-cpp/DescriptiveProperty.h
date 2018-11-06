/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <ItemProperty.h>

namespace HEIFPP
{
    class DescriptiveProperty : public HEIFPP::ItemProperty
    {
    public:
        ~DescriptiveProperty() = default;

    protected:
        DescriptiveProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType, const HEIF::FourCC& aRawType);

    protected:
        // valid types: 'pasp','colr','pixi','rloc','auxc','lsel'

        // hmm: 'hvcC' 'lhvC' 'avcC' 'jpgC' decoder config/init properties not exposed.
        //     'subs'  'oinf' 'tols' decoder properties not exposed here.
        //     'ispe'   not exposed here.
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;

    private:
        DescriptiveProperty& operator=(const DescriptiveProperty&) = delete;
        DescriptiveProperty& operator=(DescriptiveProperty&&)      = delete;
        DescriptiveProperty(const DescriptiveProperty&)            = delete;
        DescriptiveProperty(DescriptiveProperty&&)                 = delete;
        DescriptiveProperty()                                      = delete;
    };

    class PixelAspectRatioProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelAspectRatio mPixelAspectRatio;
        PixelAspectRatioProperty(Heif* aHeif);
        ~PixelAspectRatioProperty() = default;
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        PixelAspectRatioProperty& operator=(const PixelAspectRatioProperty&) = delete;
        PixelAspectRatioProperty& operator=(PixelAspectRatioProperty&&)      = delete;
        PixelAspectRatioProperty(const PixelAspectRatioProperty&)            = delete;
        PixelAspectRatioProperty(PixelAspectRatioProperty&&)                 = delete;
        PixelAspectRatioProperty()                                           = delete;
    };

    class ColourInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::ColourInformation mColourInformation;
        ColourInformationProperty(Heif* aHeif);
        ~ColourInformationProperty() = default;
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        ColourInformationProperty& operator=(const ColourInformationProperty&) = delete;
        ColourInformationProperty& operator=(ColourInformationProperty&&)      = delete;
        ColourInformationProperty(const ColourInformationProperty&)            = delete;
        ColourInformationProperty(ColourInformationProperty&&)                 = delete;
        ColourInformationProperty()                                            = delete;
    };

    class PixelInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelInformation mPixelInformation;
        PixelInformationProperty(Heif* aHeif);
        ~PixelInformationProperty() = default;
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        PixelInformationProperty& operator=(const PixelInformationProperty&) = delete;
        PixelInformationProperty& operator=(PixelInformationProperty&&)      = delete;
        PixelInformationProperty(const PixelInformationProperty&)            = delete;
        PixelInformationProperty(PixelInformationProperty&&)                 = delete;
        PixelInformationProperty()                                           = delete;
    };

    class RelativeLocationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::RelativeLocation mRelativeLocation;
        RelativeLocationProperty(Heif* aHeif);
        ~RelativeLocationProperty() = default;
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        RelativeLocationProperty& operator=(const RelativeLocationProperty&) = delete;
        RelativeLocationProperty& operator=(RelativeLocationProperty&&)      = delete;
        RelativeLocationProperty(const RelativeLocationProperty&)            = delete;
        RelativeLocationProperty(RelativeLocationProperty&&)                 = delete;
        RelativeLocationProperty()                                           = delete;
    };

    class AuxiliaryProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        AuxiliaryProperty(Heif* aHeif);
        ~AuxiliaryProperty() = default;
        const std::string& auxType();
        void auxType(const std::string&);
        const std::vector<std::uint8_t>& subType();
        void subType(const std::vector<std::uint8_t>&);
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        std::string mAuxType;  ///< Type of the associated auxiliary image item. This is not null-terminated.
        std::vector<std::uint8_t> mSubType;  ///< Aux subtype, semantics depends on the auxType value
    private:
        AuxiliaryProperty& operator=(const AuxiliaryProperty&) = delete;
        AuxiliaryProperty& operator=(AuxiliaryProperty&&)      = delete;
        AuxiliaryProperty(const AuxiliaryProperty&)            = delete;
        AuxiliaryProperty(AuxiliaryProperty&&)                 = delete;
        AuxiliaryProperty()                              = delete;
    };
}  // namespace HEIFPP