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

#include "CodedImageItem.h"
#include "Helpers.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME CodedImageItem

extern "C"
{
    JNI_METHOD(jstring, getDecoderCodeTypeNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewStringUTF(nativeSelf->getDecoderCodeType().value);
            }
        JNI_CATCH_RETURN(nullptr, "CodedImageItem.getDecoderCodeTypeNative")
    }

    JNI_METHOD(jobject, getItemDataNative)
    {
        // The JNI bridge intentionally hides the raw-Reader probe-and-fetch
        // protocol. The bytes are
        // pre-loaded by HEIFPP during Heif::load; we just wrap the cached
        // buffer in a java.nio.ByteBuffer whose lifetime is tied to the
        // parent HEIF instance.
        JNI_TRY
        {
        NATIVE_SELF;
        return env->NewDirectByteBuffer(const_cast<uint8_t*>(nativeSelf->getItemData()),
                                        static_cast<jlong>(nativeSelf->getItemDataSize()));
            }
        JNI_CATCH_RETURN(nullptr, "CodedImageItem.getItemDataNative")
    }

    JNI_METHOD_ARG(void, setItemDataNative, jbyteArray data)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        jbyte* nativeData = env->GetByteArrayElements(data, nullptr);
        nativeSelf->setItemData((uint8_t*) (nativeData), static_cast<uint64_t>(env->GetArrayLength(data)));
        env->ReleaseByteArrayElements(data, nativeData, 0);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.setItemDataNative")
    }

    JNI_METHOD(jint, getBaseImageCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getBaseImageCount());
            }
        JNI_CATCH_RETURN(0, "CodedImageItem.getBaseImageCountNative")
    }

    JNI_METHOD_ARG(jobject, getBaseImageNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItem(env, getJavaHEIF(env, self),
                           (jobject) nativeSelf->getBaseImage(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "CodedImageItem.getBaseImageNative")
    }

    JNI_METHOD_ARG(void, addBaseImageNative, jobject image)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->addBaseImage(nativeImage);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.addBaseImageNative")
    }

    JNI_METHOD_ARG(void, removeBaseImageNative, jobject image)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->removeBaseImage(nativeImage);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.removeBaseImageNative")
    }

    JNI_METHOD_ARG(void, removeBaseImageByIndexNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->removeBaseImage(static_cast<uint32_t>(index));
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.removeBaseImageByIndexNative")
    }

    JNI_METHOD_ARG(void, setBaseImageByIndexNative, jint index, jobject image)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeImage, image);
        nativeSelf->setBaseImage(static_cast<uint32_t>(index), nativeImage);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.setBaseImageByIndexNative")
    }

    JNI_METHOD_ARG(void, setBaseImageNative, jobject oldImage, jobject newImage)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_IMAGE_ITEM(nativeOldImage, oldImage);
        NATIVE_IMAGE_ITEM(nativeNewImage, newImage);
        nativeSelf->setBaseImage(nativeOldImage, nativeNewImage);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.setBaseImageNative")
    }

    JNI_METHOD_ARG(void, reserveBaseImagesNative, jint count)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->reserveBaseImages(static_cast<uint32_t>(count));
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.reserveBaseImagesNative")
    }

    JNI_METHOD_ARG(void, setDecoderConfigNative, jobject config)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_DECODER_CONFIG(nativeConfig, config);
        nativeSelf->setDecoderConfiguration(nativeConfig);
            }
        JNI_CATCH_RETURN_VOID("CodedImageItem.setDecoderConfigNative")
    }

    JNI_METHOD(jobject, getDecoderConfigNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getDecoderConfig(env, getJavaHEIF(env, self), nativeSelf->getDecoderConfiguration());
            }
        JNI_CATCH_RETURN(nullptr, "CodedImageItem.getDecoderConfigNative")
    }
}
