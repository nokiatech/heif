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

#include <jni.h>

#include "Heif.h"
#include "Helpers.h"
#include "IdentityImageItem.h"
#include "jni_md.h"

#define CLASS_NAME IdentityImageItem

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = nativeHeif->create<HEIFPP::IdentityImageItem>();
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "IdentityImageItem.createContextNative")
    }

    JNI_METHOD(jobject, getImageNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), nativeSelf->getImage());
            }
        JNI_CATCH_RETURN(nullptr, "IdentityImageItem.getImageNative")
    }

    JNI_METHOD_ARG(void, setImageNative, jobject image)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->setImage(nativeImage);
            }
        JNI_CATCH_RETURN_VOID("IdentityImageItem.setImageNative")
    }
}
