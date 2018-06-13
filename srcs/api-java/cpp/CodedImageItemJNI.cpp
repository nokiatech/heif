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
#include "CodedImageItem.h"
#include "Helpers.h"

#define CLASS_NAME CodedImageItem

extern "C"
{
    JNI_METHOD(jstring, getDecoderCodeTypeNative)
    {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->getDecoderCodeType().value);
    }

    JNI_METHOD(jobject, getItemDataNative)
    {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getItemData()),
                                        static_cast<jlong>(nativeSelf->getItemDataSize()));
    }

    JNI_METHOD_ARG(void, setItemDataNative, jbyteArray data)
    {
        NATIVE_SELF;
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        nativeSelf->setItemData((uint8_t*)(nativeData),
                                  static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }

    JNI_METHOD(jint, getBaseImageCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getBaseImageCount());
    }

    JNI_METHOD_ARG(jobject, getBaseImageNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self), (jobject) nativeSelf->getBaseImage(
                static_cast<uint32_t>(index)));
    }

    JNI_METHOD_ARG(void, addBaseImageNative, jobject image)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->addBaseImage(nativeImage);
    }

    JNI_METHOD_ARG(void, removeBaseImageNative, jobject image)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->removeBaseImage(nativeImage);
    }

    JNI_METHOD_ARG(void, removeBaseImageByIndexNative, jint index)
    {
        NATIVE_SELF;
        nativeSelf->removeBaseImage(static_cast<uint32_t>(index));
    }

    JNI_METHOD_ARG(void, setBaseImageByIndexNative, jint index, jobject image)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->setBaseImage(static_cast<uint32_t>(index), nativeImage);
    }

    JNI_METHOD_ARG(void, setBaseImageNative, jobject oldImage, jobject newImage)
    {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeOldImage, oldImage);
        NATIVE_IMAGE_ITEM(nativeNewImage, newImage);
        nativeSelf->setBaseImage(nativeOldImage, nativeNewImage);
    }

    JNI_METHOD_ARG(void, reserveBaseImagesNative, jint count)
    {
        NATIVE_SELF;
        nativeSelf->reserveBaseImages(static_cast<uint32_t>(count));
    }

    JNI_METHOD_ARG(void, setDecoderConfigNative, jobject config)
    {
        NATIVE_SELF;
        NATIVE_DECODER_CONFIG(nativeConfig, config);
        nativeSelf->setDecoderConfiguration(nativeConfig);
    }

    JNI_METHOD(jobject, getDecoderConfigNative)
    {
        NATIVE_SELF;
        return getDecoderConfig(env, getJavaHEIF(env, self), nativeSelf->getDecoderConfiguration());
    }
}
