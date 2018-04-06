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

#define JNI_METHOD(return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_CleanApertureProperty_##method_name
#define NATIVE_HANDLE \
    HEIFPP::CleanApertureProperty *nativeHandle = (HEIFPP::CleanApertureProperty *) getNativeHandle(env, obj)

extern "C"
{
    jobject createFraction(JNIEnv *env, jobject obj, int numerator, int denominator)
    {
        const jclass javaClass = env->GetObjectClass(obj);

        const jmethodID createMethodId =
            env->GetMethodID(javaClass, "createFraction", "(II)Lcom/nokia/heif/CleanApertureProperty$Fraction;");
        env->DeleteLocalRef(javaClass);
        return env->CallObjectMethod(obj, createMethodId, numerator, denominator);
    }


    JNI_METHOD(jlong, createContextNative)(JNIEnv *env, jobject obj, jobject javaHEIF)
    {
        NATIVE_HEIF(nativeHeif, javaHEIF);
        HEIFPP::CleanApertureProperty *nativeObject = new HEIFPP::CleanApertureProperty(nativeHeif);
        nativeObject->setContext((void *) env->NewGlobalRef(obj));
        return (jlong) nativeObject;
    }

    JNI_METHOD(jobject, getWidthNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CLAP(nativeHandle, obj);
        return createFraction(env, obj, nativeHandle->mClap.widthN, nativeHandle->mClap.widthD);
    }

    JNI_METHOD(void, setWidthNumNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.widthN = value;
    }

    JNI_METHOD(void, setWidthDenNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.widthD = value;
    }

    JNI_METHOD(jobject, getHeightNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CLAP(nativeHandle, obj);
        return createFraction(env, obj, nativeHandle->mClap.heightN, nativeHandle->mClap.heightD);
    }

    JNI_METHOD(void, setHeightNumNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.heightN = value;
    }

    JNI_METHOD(void, setHeightDenNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.heightD = value;
    }

    JNI_METHOD(jobject, getHorizontalOffsetNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CLAP(nativeHandle, obj);
        return createFraction(env, obj, nativeHandle->mClap.horizontalOffsetN, nativeHandle->mClap.horizontalOffsetD);
    }

    JNI_METHOD(void, setHorizontalOffsetNumNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.horizontalOffsetN = value;
    }

    JNI_METHOD(void, setHorizontalOffsetDenNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.horizontalOffsetD = value;
    }

    JNI_METHOD(jobject, getVerticalOffsetNative)(JNIEnv *env, jobject obj)
    {
        NATIVE_CLAP(nativeHandle, obj);
        return createFraction(env, obj, nativeHandle->mClap.verticalOffsetN, nativeHandle->mClap.verticalOffsetD);
    }

    JNI_METHOD(void, setVerticalOffsetNumNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.verticalOffsetN = value;
    }

    JNI_METHOD(void, setVerticalOffsetDenNative)(JNIEnv *env, jobject obj, jint value)
    {
        NATIVE_HANDLE;
        nativeHandle->mClap.verticalOffsetD = value;
    }
}