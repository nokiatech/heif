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
#include <cstring>
#include "DescriptiveProperty.h"
#include "Helpers.h"

#define CLASS_NAME PixelInformationProperty

extern "C"
{
    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::PixelInformationProperty *nativeObject = new HEIFPP::PixelInformationProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }


    JNI_METHOD(jobject, getPixelInformationNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(nativeSelf->mPixelInformation.bitsPerChannel.elements,
                                        static_cast<jlong>(nativeSelf->mPixelInformation.bitsPerChannel.size));
    }

    JNI_METHOD_ARG(void, setPixelInformationNative, jbyteArray data)
    {
        NATIVE_SELF;
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        uint32_t dataSize = static_cast<uint32_t>(env->GetArrayLength(data));

        nativeSelf->mPixelInformation.bitsPerChannel = HEIF::Array<uint8_t>(dataSize);
        std::memcpy(nativeSelf->mPixelInformation.bitsPerChannel.elements, nativeData, dataSize);
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }
}
