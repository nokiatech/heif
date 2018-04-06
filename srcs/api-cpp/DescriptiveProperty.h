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
    protected:
        DescriptiveProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType);
        virtual ~DescriptiveProperty() = default;

    protected:
        // valid types: 'pasp','colr','pixi','rloc','auxc','lsel'

        // hmm: 'hvcC' 'lhvC' 'avcC' 'jpgC' decoder config/init properties not exposed.
        //     'subs'  'oinf' 'tols' decoder properties not exposed here.
        //     'ispe'   not exposed here.
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);

    private:
        DescriptiveProperty& operator=(const DescriptiveProperty&) = delete;
        DescriptiveProperty(const DescriptiveProperty&)            = delete;
        DescriptiveProperty(DescriptiveProperty&&)                 = delete;
        DescriptiveProperty()                                      = delete;
    };

    class PixelAspectRatioProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelAspectRatio mPixelAspectRatio;
        PixelAspectRatioProperty(Heif* aHeif);
        virtual ~PixelAspectRatioProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        PixelAspectRatioProperty& operator=(const PixelAspectRatioProperty&) = delete;
        PixelAspectRatioProperty(const PixelAspectRatioProperty&)            = delete;
        PixelAspectRatioProperty(PixelAspectRatioProperty&&)                 = delete;
        PixelAspectRatioProperty()                                           = delete;
    };

    class ColourInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::ColourInformation mColourInformation;
        ColourInformationProperty(Heif* aHeif);
        virtual ~ColourInformationProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        ColourInformationProperty& operator=(const ColourInformationProperty&) = delete;
        ColourInformationProperty(const ColourInformationProperty&)            = delete;
        ColourInformationProperty(ColourInformationProperty&&)                 = delete;
        ColourInformationProperty()                                            = delete;
    };

    class PixelInformationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::PixelInformation mPixelInformation;
        PixelInformationProperty(Heif* aHeif);
        virtual ~PixelInformationProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        PixelInformationProperty& operator=(const PixelInformationProperty&) = delete;
        PixelInformationProperty(const PixelInformationProperty&)            = delete;
        PixelInformationProperty(PixelInformationProperty&&)                 = delete;
        PixelInformationProperty()                                           = delete;
    };

    class RelativeLocationProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        HEIF::RelativeLocation mRelativeLocation;
        RelativeLocationProperty(Heif* aHeif);
        virtual ~RelativeLocationProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        RelativeLocationProperty& operator=(const RelativeLocationProperty&) = delete;
        RelativeLocationProperty(const RelativeLocationProperty&)            = delete;
        RelativeLocationProperty(RelativeLocationProperty&&)                 = delete;
        RelativeLocationProperty()                                           = delete;
    };

    class AuxProperty : public HEIFPP::DescriptiveProperty
    {
    public:
        AuxProperty(Heif* aHeif);
        virtual ~AuxProperty() = default;
        const std::string& auxType();
        void auxType(const std::string&);
        const std::vector<uint8_t>& subType();
        void subType(const std::vector<uint8_t>&);

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        std::string mAuxType;           ///< Type of the associated auxiliary image item. This is not null-terminated.
        std::vector<uint8_t> mSubType;  ///< Aux subtype, semantics depends on the auxType value
    private:
        AuxProperty& operator=(const AuxProperty&) = delete;
        AuxProperty(const AuxProperty&)            = delete;
        AuxProperty(AuxProperty&&)                 = delete;
        AuxProperty()                              = delete;
    };
}  // namespace HEIFPP