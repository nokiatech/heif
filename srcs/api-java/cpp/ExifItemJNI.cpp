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

#include "ExifItem.h"
#include "Heif.h"
#include "Helpers.h"
#include "jni_md.h"
#include <stdint.h>
#define CLASS_NAME ExifItem
extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = nativeHeif->create<HEIFPP::ExifItem>();
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "ExifItem.createContextNative")
    }

    JNI_METHOD(jobject, getItemDataNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getData()),
                                        static_cast<jlong>(nativeSelf->getDataSize()));
            }
        JNI_CATCH_RETURN(nullptr, "ExifItem.getItemDataNative")
    }

    JNI_METHOD_ARG(void, setItemDataNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(data, nullptr);
        nativeSelf->setData((uint8_t*) nativeData, static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("ExifItem.setItemDataNative")
    }
}
