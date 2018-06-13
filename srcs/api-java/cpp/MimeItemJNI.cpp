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
#include "MimeItem.h"

#define CLASS_NAME MimeItem

extern "C"
{
    JNI_METHOD(jobject, getItemDataNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getData()),
                                        static_cast<jlong>(nativeSelf->getDataSize()));
    }

    JNI_METHOD_ARG(void, setItemDataNative, jbyteArray data)
    {
        NATIVE_SELF;
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        nativeSelf->setData((uint8_t *) nativeData, static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}