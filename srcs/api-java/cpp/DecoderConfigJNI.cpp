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
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_DecoderConfig_##method_name
#include "CodedImageItem.h"
#include "Helpers.h"
extern "C"
{
    JNI_METHOD(void, destroyContextNative)(JNIEnv* env, jobject obj)
    {
        NATIVE_DECODER_CONFIG(nativeItem, obj);
        jobject javaHandle = (jobject) nativeItem->getContext();
        env->DeleteGlobalRef(javaHandle);
        setNativeHandle(env, obj, 0);
        delete nativeItem;
    }

    JNI_METHOD(void, setConfigNative)(JNIEnv* env, jobject obj, jbyteArray configdata)
    {
        NATIVE_DECODER_CONFIG(nativeHandle, obj);
        jbyte* nativeData = env->GetByteArrayElements(configdata, 0);
        nativeHandle->setConfig((uint8_t*) nativeData, env->GetArrayLength(configdata));
        env->ReleaseByteArrayElements(configdata, nativeData, 0);
    }

    JNI_METHOD(jobject, getConfigNative)(JNIEnv* env, jobject obj)
    {
        NATIVE_DECODER_CONFIG(nativeHandle, obj);
        const HEIF::Array<HEIF::DecoderSpecificInfo>& codecInfo = nativeHandle->getConfig();
        size_t totalSize                                        = 0;
        for (size_t index = 0; index < codecInfo.size; index++)
        {
            totalSize += codecInfo[index].decSpecInfoData.size;
        }
        jbyte* data       = new jbyte[totalSize];
        size_t writeIndex = 0;
        for (size_t index = 0; index < codecInfo.size; index++)
        {
            memcpy(data + writeIndex, codecInfo[index].decSpecInfoData.elements, codecInfo[index].decSpecInfoData.size);
            writeIndex += codecInfo[index].decSpecInfoData.size;
        }
        jbyteArray output = env->NewByteArray(totalSize);
        env->SetByteArrayRegion(output, 0, totalSize, data);
        delete[] data;
        return output;
    }
}