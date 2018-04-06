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
#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_PixelInformationProperty_##method_name
#include "DescriptiveProperty.h"
#include "Helpers.h"
extern "C"
{
    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::PixelInformationProperty *nativeObject = new HEIFPP::PixelInformationProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }


    JNI_METHOD(jobject, getPixelInformationNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_PIXEL_INFORMATION_PROPERTY(nativeHandle, obj);
        return env->NewDirectByteBuffer(nativeHandle->mPixelInformation.bitsPerChannel.elements,
                                        nativeHandle->mPixelInformation.bitsPerChannel.size);
    }

    JNI_METHOD(void, setPixelInformationNative)(JNIEnv *env, jobject obj, jbyteArray data)
    {
        NATIVE_PIXEL_INFORMATION_PROPERTY(nativeHandle, obj);
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        uint32_t dataSize = env->GetArrayLength(data);

        nativeHandle->mPixelInformation.bitsPerChannel = HEIF::Array<uint8_t>(dataSize);
        memcpy(nativeHandle->mPixelInformation.bitsPerChannel.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}
