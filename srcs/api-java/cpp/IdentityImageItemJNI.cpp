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
#include "IdentityImageItem.h"

#define CLASS_NAME IdentityImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::Identity *nativeObject = new HEIFPP::Identity(nativeHeif);
        nativeObject->setContext(static_cast<void*>(env->NewGlobalRef(self)));
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jobject, getImageNative)
    {
        NATIVE_IDENTITY_IMAGE_ITEM(nativeHandle, self);
        return getJavaItem(env, getJavaHEIF(env, self), nativeHandle->getImage());
    }

    JNI_METHOD_ARG(void, setImageNative, jobject image)
    {
        NATIVE_IDENTITY_IMAGE_ITEM(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->setImage(nativeImage);
    }
}