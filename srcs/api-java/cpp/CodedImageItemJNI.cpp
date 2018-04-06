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
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_CodedImageItem_##method_name
#include "CodedImageItem.h"
#include "Helpers.h"

extern "C"
{
    JNI_METHOD(jstring, getDecoderCodeTypeNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        return env->NewStringUTF(nativeHandle->getDecoderCodeType().value);
    }


    JNI_METHOD(jobject, getItemDataNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        return env->NewDirectByteBuffer((void *) nativeHandle->getItemData(), nativeHandle->getItemDataSize());
    }

    JNI_METHOD(void, setItemDataNative)(JNIEnv *env, jobject obj, jbyteArray data)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        jbyte *nativeData = env->GetByteArrayElements(data, 0);
        nativeHandle->setItemData((uint8_t *) nativeData, env->GetArrayLength(data));
        env->ReleaseByteArrayElements(data, nativeData, 0);
    }

    JNI_METHOD(jint, getBaseImageCountNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        return nativeHandle->getBaseImageCount();
    }

    JNI_METHOD(jobject, getBaseImageNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        return getJavaItem(env, getJavaHEIF(env, obj), (jobject) nativeHandle->getBaseImage(index));
    }

    JNI_METHOD(void, addBaseImageNative)(JNIEnv *env, jobject obj, jobject image)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->addBaseImage(nativeImage);
    }

    JNI_METHOD(void, removeBaseImageNative)(JNIEnv *env, jobject obj, jobject image)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->removeBaseImage(nativeImage);
    }

    JNI_METHOD(void, removeBaseImageByIndexNative)(JNIEnv *env, jobject obj, jint index)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->removeBaseImage(index);
    }

    JNI_METHOD(void, setBaseImageByIndexNative)(JNIEnv *env, jobject obj, jint index, jobject image)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeHandle->setBaseImage(index, nativeImage);
    }

    JNI_METHOD(void, setBaseImageNative)(JNIEnv *env, jobject obj, jobject oldImage, jobject newImage)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_IMAGE_ITEM(nativeOldImage, oldImage);
        NATIVE_IMAGE_ITEM(nativeNewImage, newImage);
        nativeHandle->setBaseImage(nativeOldImage, nativeNewImage);
    }

    JNI_METHOD(void, reserveBaseImagesNative)(JNIEnv *env, jobject obj, jint count)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        nativeHandle->reserveBaseImages(count);
    }

    JNI_METHOD(void, setDecoderConfigNative)(JNIEnv *env, jobject obj, jobject config)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        NATIVE_DECODER_CONFIG(nativeConfig, config);
        nativeHandle->setDecoderConfiguration(nativeConfig);
    }

    JNI_METHOD(jobject, getDecoderConfigNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CODED_IMAGE_ITEM(nativeHandle, obj);
        return getDecoderConfig(env, getJavaHEIF(env, obj), nativeHandle->getDecoderConfiguration());
    }
}
