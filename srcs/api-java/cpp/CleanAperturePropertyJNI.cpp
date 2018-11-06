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
#include "TransformativeProperty.h"

#define CLASS_NAME CleanApertureProperty

extern "C"
{
    jobject createFraction(JNIEnv *env, jobject obj, int numerator, int denominator);
    jobject createFraction(JNIEnv *env, jobject obj, int numerator, int denominator)
    {
        const jclass javaClass = env->GetObjectClass(obj);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createFraction", "(II)Lcom/nokia/heif/CleanApertureProperty$Fraction;");
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(obj, createMethodId, numerator, denominator);
    }


    JNI_METHOD_ARG(jlong, createContextNative, jobject javaHEIF)
    {
        UNUSED(self);
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::CleanApertureProperty *nativeObject = new HEIFPP::CleanApertureProperty(nativeHeif);
        return reinterpret_cast<jlong>(nativeObject);
    }

    JNI_METHOD(jobject, getWidthNative)
    {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.widthN),
                              static_cast<jint>(nativeSelf->mClap.widthD));
    }

    JNI_METHOD_ARG(void, setWidthNumNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.widthN = static_cast<uint32_t>(value);
    }

    JNI_METHOD_ARG(void, setWidthDenNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.widthD = static_cast<uint32_t>(value);
    }

    JNI_METHOD(jobject, getHeightNative)
    {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.heightN),
                              static_cast<jint>(nativeSelf->mClap.heightD));
    }

    JNI_METHOD_ARG(void, setHeightNumNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.heightN = static_cast<uint32_t>(value);
    }

    JNI_METHOD_ARG(void, setHeightDenNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.heightD = static_cast<uint32_t>(value);
    }

    JNI_METHOD(jobject, getHorizontalOffsetNative)
    {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.horizontalOffsetN),
                              static_cast<jint>(nativeSelf->mClap.horizontalOffsetD));
    }

    JNI_METHOD_ARG(void, setHorizontalOffsetNumNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.horizontalOffsetN = static_cast<uint32_t>(value);
    }

    JNI_METHOD_ARG(void, setHorizontalOffsetDenNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.horizontalOffsetD = static_cast<uint32_t>(value);
    }

    JNI_METHOD(jobject, getVerticalOffsetNative)
    {
        NATIVE_SELF;
        return createFraction(env, self, static_cast<jint>(nativeSelf->mClap.verticalOffsetN),
                              static_cast<jint>(nativeSelf->mClap.verticalOffsetD));
    }

    JNI_METHOD_ARG(void, setVerticalOffsetNumNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.verticalOffsetN = static_cast<uint32_t>(value);
    }

    JNI_METHOD_ARG(void, setVerticalOffsetDenNative, jint value)
    {
        NATIVE_SELF;
        nativeSelf->mClap.verticalOffsetD = static_cast<uint32_t>(value);
    }
}
