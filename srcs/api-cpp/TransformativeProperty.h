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

namespace HEIFPP
{
    class TransformativeProperty : public HEIFPP::ItemProperty
    {
    public:
        ~TransformativeProperty() override = default;

    protected:
        TransformativeProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType, const HEIF::FourCC& aRawType);
        // valid types: 'clap' 'irot' 'imir'
        HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId) override;

    public:
        TransformativeProperty& operator=(const TransformativeProperty&) = delete;
        TransformativeProperty& operator=(TransformativeProperty&&) = delete;
        TransformativeProperty(const TransformativeProperty&)       = delete;
        TransformativeProperty(TransformativeProperty&&)            = delete;
        TransformativeProperty()                                    = delete;
    };

    class CleanApertureProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::CleanAperture mClap;
        explicit CleanApertureProperty(Heif* aHeif);
        ~CleanApertureProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        CleanApertureProperty& operator=(const CleanApertureProperty&) = delete;
        CleanApertureProperty& operator=(CleanApertureProperty&&) = delete;
        CleanApertureProperty(const CleanApertureProperty&)       = delete;
        CleanApertureProperty(CleanApertureProperty&&)            = delete;
        CleanApertureProperty()                                   = delete;
    };
    class RotateProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::Rotate mRotate;
        explicit RotateProperty(Heif* aHeif);
        ~RotateProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        RotateProperty& operator=(const RotateProperty&) = delete;
        RotateProperty& operator=(RotateProperty&&) = delete;
        RotateProperty(const RotateProperty&)       = delete;
        RotateProperty(RotateProperty&&)            = delete;
        RotateProperty()                            = delete;
    };
    class MirrorProperty : public HEIFPP::TransformativeProperty
    {
    public:
        HEIF::Mirror mMirror;
        explicit MirrorProperty(Heif* aHeif);
        ~MirrorProperty() override = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        MirrorProperty& operator=(const MirrorProperty&) = delete;
        MirrorProperty& operator=(MirrorProperty&&) = delete;
        MirrorProperty(const MirrorProperty&)       = delete;
        MirrorProperty(MirrorProperty&&)            = delete;
        MirrorProperty()                            = delete;
    };

}  // namespace HEIFPP
