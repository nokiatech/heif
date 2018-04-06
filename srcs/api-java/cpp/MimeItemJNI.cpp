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
#define JNI_METHOD(return_type, method_name) JNIEXPORT return_type JNICALL Java_com_nokia_heif_MimeItem_##method_name
extern "C"
{
    JNI_METHOD(jobject, getItemDataNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_MIME_ITEM(nativeHandle, obj);
        return env->NewDirectByteBuffer((void *) nativeHandle->getData(), nativeHandle->getDataSize());
    }

    JNI_METHOD(void, setItemDataNative)(JNIEnv *env, jobject obj, jbyteArray data)
    {
        NATIVE_MIME_ITEM(nativeHandle, obj);
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        nativeHandle->setData((uint8_t *) nativeData, env->GetArrayLength(data));
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}