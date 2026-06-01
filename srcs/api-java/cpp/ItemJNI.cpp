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
#include "Item.h"
#include "heifcommondatatypes.h"
#include "jni_md.h"
#include <stdint.h>
#define CLASS_NAME Item
extern "C"
{
    JNI_METHOD(jstring, getTypeNative)
    {
        JNI_TRY
        {
        auto* instance = (HEIFPP::Item*) getNativeHandle(env, self);
        return env->NewStringUTF(instance->getType().value);
            }
        JNI_CATCH_RETURN(nullptr, "Item.getTypeNative")
    }

    JNI_METHOD(void, destroyContextNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        setNativeHandle(env, self, 0);
        delete nativeSelf;
            }
        JNI_CATCH_RETURN_VOID("Item.destroyContextNative")
    }

    JNI_METHOD_ARG(jboolean, isEssentialPropertyNative, jobject property)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(nativeProperty, property);
        return static_cast<jboolean>(nativeSelf->isEssential(nativeProperty));
            }
        JNI_CATCH_RETURN(JNI_FALSE, "Item.isEssentialPropertyNative")
    }

    JNI_METHOD_ARG(void, setEssentialPropertyNative, jobject property, jboolean essential)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        NATIVE_ITEM_PROPERTY(nativeProperty, property);
        nativeSelf->setEssential(nativeProperty, essential != 0);
            }
        JNI_CATCH_RETURN_VOID("Item.setEssentialPropertyNative")
    }

    JNI_METHOD(jint, getGroupCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->getGroupCount());
            }
        JNI_CATCH_RETURN(0, "Item.getGroupCountNative")
    }

    JNI_METHOD_ARG(jobject, getGroupNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaEntityGroup(env, getJavaHEIF(env, self), nativeSelf->getGroup(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Item.getGroupNative")
    }

    JNI_METHOD(jint, getPropertyCountNative)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return static_cast<jint>(nativeSelf->propertyCount());
            }
        JNI_CATCH_RETURN(0, "Item.getPropertyCountNative")
    }

    JNI_METHOD_ARG(jobject, getPropertyNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_SELF;
        return getJavaItemProperty(env, getJavaHEIF(env, self), nativeSelf->getProperty(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "Item.getPropertyNative")
    }
}
