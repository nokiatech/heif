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

#define CLASS_NAME MirrorProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::MirrorProperty *nativeObject = new HEIFPP::MirrorProperty(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jboolean, getMirrorNative)
    {
        NATIVE_MIRROR_PROPERTY(nativeHandle, self);
        return static_cast<jboolean >(nativeHandle->mMirror.horizontalAxis);
    }

    JNI_METHOD_ARG(void, setMirrorNative, jboolean mirror)
    {
        NATIVE_MIRROR_PROPERTY(nativeHandle, self);
        nativeHandle->mMirror.horizontalAxis = mirror;
    }
}