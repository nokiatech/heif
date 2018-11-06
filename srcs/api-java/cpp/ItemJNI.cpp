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
#include "Item.h"
#define CLASS_NAME Item
extern "C"
{
    JNI_METHOD(jstring, getTypeNative)
    {
        HEIFPP::Item *instance = (HEIFPP::Item *) getNativeHandle(env, self);
        return env->NewStringUTF(instance->getType().value);
    }

    JNI_METHOD(void, destroyContextNative)
    {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
    }

    JNI_METHOD_ARG(jboolean, isEssentialPropertyNative, jobject property)
    {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(nativeProperty, property);
        return static_cast<jboolean>(nativeSelf->isEssential(nativeProperty));
    }

    JNI_METHOD_ARG(void, setEssentialPropertyNative, jobject property, jboolean essential)
    {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(nativeProperty, property);
        nativeSelf->setEssential(nativeProperty, essential != 0);
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self), nativeSelf->getGroup(static_cast<uint32_t>(index)));
    }

    JNI_METHOD(jint, getPropertyCountNative)
    {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->propertyCount());
    }

    JNI_METHOD_ARG(jobject, getPropertyNative, jint index)
    {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->getProperty(static_cast<uint32_t>(index)));
    }

}
