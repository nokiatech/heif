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
 *
 *
 */

#include <DescriptiveProperty.h>
#include <jni.h>

#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"

#define CLASS_NAME OperatingPointsProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::OperatingPointsProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "OperatingPointsProperty.createContextNative")
    }

    JNI_METHOD(jint, getScalabilityMaskNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mOperatingPoints.scalabilityMask);
            }
        JNI_CATCH_RETURN(0, "OperatingPointsProperty.getScalabilityMaskNative")
    }

    JNI_METHOD(jint, getOperatingPointCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mOperatingPoints.operatingPoints.size);
            }
        JNI_CATCH_RETURN(0, "OperatingPointsProperty.getOperatingPointCountNative")
    }

    JNI_METHOD(jint, getProfileTierLevelCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mOperatingPoints.profileTierLevels.size);
            }
        JNI_CATCH_RETURN(0, "OperatingPointsProperty.getProfileTierLevelCountNative")
    }

    JNI_METHOD(jint, getLayerCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mOperatingPoints.layers.size);
            }
        JNI_CATCH_RETURN(0, "OperatingPointsProperty.getLayerCountNative")
    }
}
