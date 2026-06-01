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

#include "Helpers.h"
#include "TransformativeProperty.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME CleanApertureProperty

extern "C"
{
    jobject createFraction(JNIEnv* env, jobject obj, int numerator, int denominator);
    jobject createFraction(JNIEnv* env, jobject obj, int numerator, int denominator)
    {
        const jclass javaClass = env->GetObjectClass(obj);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createFraction", "(II)Lcom/nokia/heif/CleanApertureProperty$Fraction;");
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(obj, createMethodId, numerator, denominator);
    }


    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        JNI_TRY
        {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        auto* nativeObject = new HEIFPP::CleanApertureProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
            }
        JNI_CATCH_RETURN(0, "CleanApertureProperty.createContextNative")
    }

    JNI_METHOD(jobject, getWidthNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.widthN),
                              static_cast<jint>(nativeSelf->mClap.widthD));
            }
        JNI_CATCH_RETURN(nullptr, "CleanApertureProperty.getWidthNative")
    }

    JNI_METHOD_ARG(void, setWidthNumNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.widthN = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setWidthNumNative")
    }

    JNI_METHOD_ARG(void, setWidthDenNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.widthD = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setWidthDenNative")
    }

    JNI_METHOD(jobject, getHeightNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.heightN),
                              static_cast<jint>(nativeSelf->mClap.heightD));
            }
        JNI_CATCH_RETURN(nullptr, "CleanApertureProperty.getHeightNative")
    }

    JNI_METHOD_ARG(void, setHeightNumNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.heightN = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setHeightNumNative")
    }

    JNI_METHOD_ARG(void, setHeightDenNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.heightD = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setHeightDenNative")
    }

    JNI_METHOD(jobject, getHorizontalOffsetNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.horizontalOffsetN),
                              static_cast<jint>(nativeSelf->mClap.horizontalOffsetD));
            }
        JNI_CATCH_RETURN(nullptr, "CleanApertureProperty.getHorizontalOffsetNative")
    }

    JNI_METHOD_ARG(void, setHorizontalOffsetNumNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.horizontalOffsetN = static_cast<int32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setHorizontalOffsetNumNative")
    }

    JNI_METHOD_ARG(void, setHorizontalOffsetDenNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.horizontalOffsetD = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setHorizontalOffsetDenNative")
    }

    JNI_METHOD(jobject, getVerticalOffsetNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.verticalOffsetN),
                              static_cast<jint>(nativeSelf->mClap.verticalOffsetD));
            }
        JNI_CATCH_RETURN(nullptr, "CleanApertureProperty.getVerticalOffsetNative")
    }

    JNI_METHOD_ARG(void, setVerticalOffsetNumNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.verticalOffsetN = static_cast<int32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setVerticalOffsetNumNative")
    }

    JNI_METHOD_ARG(void, setVerticalOffsetDenNative, jint value)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        nativeSelf->mClap.verticalOffsetD = static_cast<uint32_t>(value);
            }
        JNI_CATCH_RETURN_VOID("CleanApertureProperty.setVerticalOffsetDenNative")
    }
}
