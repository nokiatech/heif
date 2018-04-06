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
    class TransformativeProperty : public HEIFPP::ItemProperty
    {
    protected:
        TransformativeProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType);
        virtual ~TransformativeProperty() = default;
        // valid types: 'clap' 'irot' 'imir'
        virtual HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId);

    private:
        TransformativeProperty& operator=(const TransformativeProperty&) = delete;
        TransformativeProperty(const TransformativeProperty&)            = delete;
        TransformativeProperty(TransformativeProperty&&)                 = delete;
        TransformativeProperty()                                         = delete;
    };

    class CleanApertureProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::CleanAperture mClap;
        CleanApertureProperty(Heif* aHeif);
        virtual ~CleanApertureProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        CleanApertureProperty& operator=(const CleanApertureProperty&) = delete;
        CleanApertureProperty(const CleanApertureProperty&)            = delete;
        CleanApertureProperty(CleanApertureProperty&&)                 = delete;
        CleanApertureProperty()                                        = delete;
    };
    class RotateProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::Rotate mRotate;
        RotateProperty(Heif* aHeif);
        virtual ~RotateProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        RotateProperty& operator=(const RotateProperty&) = delete;
        RotateProperty(const RotateProperty&)            = delete;
        RotateProperty(RotateProperty&&)                 = delete;
        RotateProperty()                                 = delete;
    };
    class MirrorProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::Mirror mMirror;
        MirrorProperty(Heif* aHeif);
        virtual ~MirrorProperty() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        MirrorProperty& operator=(const MirrorProperty&) = delete;
        MirrorProperty(const MirrorProperty&)            = delete;
        MirrorProperty(MirrorProperty&&)                 = delete;
        MirrorProperty()                                 = delete;
    };

}  // namespace HEIFPP
