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
 *
 *
 */

#include <jni.h>

#include "Helpers.h"
#include "TransformativeProperty.h"

#define CLASS_NAME RotateProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::RotateProperty *nativeObject = new HEIFPP::RotateProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD_ARG(void, setRotationNative, jint rotation)
    {
        NATIVE_SELF;
        nativeSelf->mRotate.angle = static_cast<uint32_t>(rotation);
    }

    JNI_METHOD(jint, getRotationNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->mRotate.angle);
    }
}
