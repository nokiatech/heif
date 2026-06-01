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

#include "Helpers.h"

#include <MimeItem.h>

#include <cstdio>
#include <stdexcept>

#include "DecoderConfiguration.h"
#include "DescriptiveProperty.h"
#include "EntityGroup.h"
#include "ErrorContext.h"
#include "Item.h"
#include "ItemProperty.h"
#include "Sample.h"
#include "Track.h"
#include "heifcommondatatypes.h"
#include "heifreaderdatatypes.h"
#include "jni.h"
#include <new>
#include <string>

static const char* ALTERNATIVE_TRACK_GROUP_CLASS_NAME = "com/nokia/heif/AlternativeTrackGroup";

static const char* CREATE_ITEM_METHOD           = "createItem";
static const char* CREATE_ITEM_PROPERTY_METHOD  = "createItemProperty";
static const char* CREATE_DECODER_CONFIG_METHOD = "createDecoderConfig";
static const char* CREATE_SAMPLE_METHOD         = "createSample";
static const char* CREATE_ENTITY_GROUP_METHOD   = "createEntityGroup";
static const char* CREATE_TRACK_METHOD          = "createTrack";

template <class type>
jobject
createJavaBaseObject(JNIEnv* env, jobject parentHeif, type nativeHandle, const char* methodName, const char* fourCC)
{
    jclass classType = env->GetObjectClass(parentHeif);
    const jmethodID createItemMethod =
        env->GetMethodID(classType, methodName, "(Ljava/lang/String;J)Lcom/nokia/heif/Base;");
    if (createItemMethod == nullptr)
    {
        return nullptr;
    }
    env->DeleteLocalRef(classType);
    jstring fourCCString = env->NewStringUTF(fourCC);
    jobject javaItem     = env->CallObjectMethod(parentHeif, createItemMethod, fourCCString, (jlong) nativeHandle);
    return javaItem;
}

template <class type>
jobject createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle)
{
    const jclass itemClass      = env->FindClass(className);
    const jmethodID constructor = env->GetMethodID(itemClass, "<init>", "(Lcom/nokia/heif/HEIF;J)V");
    if (constructor == nullptr)
    {
        return nullptr;
    }

    jobject newObject = env->NewObject(itemClass, constructor, parentJavaHEIF, (jlong) nativeHandle);
    // Delete local refs
    env->DeleteLocalRef(itemClass);

    return newObject;
}

jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItem(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    auto* heifItem              = static_cast<HEIFPP::Item*>(item);
    HEIF::FourCC itemType       = heifItem->getType();
    const char* mimeForCreation = "invl";

    if (itemType == HEIF::FourCC("hvc1") || itemType == HEIF::FourCC("avc1") || itemType == HEIF::FourCC("jpeg") ||
        itemType == HEIF::FourCC("iden") || itemType == HEIF::FourCC("iovl") || itemType == HEIF::FourCC("grid") ||
        itemType == HEIF::FourCC("Exif") || itemType == HEIF::FourCC("vvc1"))
    {
        mimeForCreation = itemType.value;
    }

    else if (itemType == HEIF::FourCC("mime"))
    {
        auto mimeItem = dynamic_cast<HEIFPP::MimeItem*>(heifItem);
        if (heifItem->isMPEG7Item())
        {
            mimeForCreation = "mpg7";
        }
        else if (heifItem->isXMPItem())
        {
            mimeForCreation = "xmp1";
        }
        else if (mimeItem && mimeItem->getContentType() == "image/jpeg")
        {
            mimeForCreation = "jpeg";
        }
    }
    return createJavaBaseObject(env, parentJavaHEIF, heifItem, CREATE_ITEM_METHOD, mimeForCreation);
}

jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item);
jobject createItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* item)
{
    auto* itemProperty                 = static_cast<HEIFPP::ItemProperty*>(item);
    const HEIF::ItemPropertyType& type = itemProperty->getType();

    const char* mimeForCreation = "invl";

    if (type == HEIF::ItemPropertyType::COLR)
    {
        auto* colourInformationProperty = (HEIFPP::ColourInformationProperty*) itemProperty;
        if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("nclx"))
        {
            mimeForCreation = "nclx";
        }
        else if (colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("rICC") ||
                 colourInformationProperty->mColourInformation.colourType == HEIF::FourCC("prof"))
        {
            mimeForCreation = "rICC";
        }
    }
    else
    {
        mimeForCreation = itemProperty->rawType().value;
    }

    return createJavaBaseObject(env, parentJavaHEIF, itemProperty, CREATE_ITEM_PROPERTY_METHOD, mimeForCreation);
}

jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfig* nativeConfig);
jobject createDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::DecoderConfig* nativeConfig)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeConfig, CREATE_DECODER_CONFIG_METHOD,
                                nativeConfig->getMediaType().value);
}

jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack);
jobject createTrack(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Track* nativeTrack)
{
    const char* mimeForCreation = "invl";
    if (nativeTrack->isMasterImageSequence() || nativeTrack->isAuxiliaryImageSequence() ||
        nativeTrack->isThumbnailImageSequence())
    {
        mimeForCreation = "pict";
    }
    else if (nativeTrack->isVideoTrack())
    {
        mimeForCreation = "vide";
    }
    else if (nativeTrack->isAudioTrack())
    {
        mimeForCreation = "soun";
    }
    return createJavaBaseObject(env, parentJavaHEIF, nativeTrack, CREATE_TRACK_METHOD, mimeForCreation);
}

jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample);
jobject createSample(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::Sample* nativeSample)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeSample, CREATE_SAMPLE_METHOD, nativeSample->getType().value);
}

jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup);
jobject createEntityGroup(JNIEnv* env, jobject parentJavaHEIF, HEIFPP::EntityGroup* nativeEntityGroup)
{
    return createJavaBaseObject(env, parentJavaHEIF, nativeEntityGroup, CREATE_ENTITY_GROUP_METHOD,
                                nativeEntityGroup->getType().value);
}

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != nullptr)
    {
        auto* nativeItem = static_cast<HEIFPP::Item*>(nativeObject);
        return createItem(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return nullptr;
    }
}

jobject getJavaAlternativeTrackGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != nullptr)
    {
        auto* nativeGroup = static_cast<HEIFPP::AlternativeTrackGroup*>(nativeObject);
        return createBaseObject(env, parentJavaHEIF, ALTERNATIVE_TRACK_GROUP_CLASS_NAME, nativeGroup);
    }
    else
    {
        return nullptr;
    }
}

jobject getJavaEntityGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != nullptr)
    {
        auto* nativeGroup = static_cast<HEIFPP::EntityGroup*>(nativeObject);
        return createEntityGroup(env, parentJavaHEIF, nativeGroup);
    }
    else
    {
        return nullptr;
    }
}

jobject getJavaTrack(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != nullptr)
    {
        auto* nativeItem = static_cast<HEIFPP::Track*>(nativeObject);
        return createTrack(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return nullptr;
    }
}

jobject getJavaSample(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject)
{
    if (nativeObject != nullptr)
    {
        auto* nativeItem = static_cast<HEIFPP::Sample*>(nativeObject);
        return createSample(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return nullptr;
    }
}

jobject getDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, void* nativeConfig)
{
    if (nativeConfig != nullptr)
    {
        auto* nativeItem = static_cast<HEIFPP::DecoderConfig*>(nativeConfig);
        return createDecoderConfig(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return nullptr;
    }
}


jobject getJavaItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* propertyItem)
{
    if (propertyItem != nullptr)
    {
        auto* nativeItem = static_cast<HEIFPP::ItemProperty*>(propertyItem);
        return createItemProperty(env, parentJavaHEIF, nativeItem);
    }
    else
    {
        return nullptr;
    }
}

jobject getJavaHEIF(JNIEnv* env, jobject obj)
{
    jclass classType              = env->GetObjectClass(obj);
    const jmethodID getHeifMethod = env->GetMethodID(classType, "getParentHEIF", "()Lcom/nokia/heif/HEIF;");
    env->DeleteLocalRef(classType);
    return env->CallObjectMethod(obj, getHeifMethod);
}

jlong getNativeHandle(JNIEnv* env, jobject obj)
{
    if (obj == nullptr)
    {
        return 0;
    }
    jclass classType = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(classType, "mNativeHandle", "J");
    env->DeleteLocalRef(classType);
    return env->GetLongField(obj, fieldID);
}

void setNativeHandle(JNIEnv* env, jobject obj, jlong handle)
{
    jclass classType = env->GetObjectClass(obj);
    jfieldID fieldID = env->GetFieldID(classType, "mNativeHandle", "J");
    env->DeleteLocalRef(classType);
    env->SetLongField(obj, fieldID, handle);
}

void releaseJavaHandles(JNIEnv* env, jobject obj)
{
    jclass classType               = env->GetObjectClass(obj);
    const jmethodID createMethodId = env->GetMethodID(classType, "releaseHandles", "()V");
    env->DeleteLocalRef(classType);
    env->CallVoidMethod(obj, createMethodId);
}

void checkError(JNIEnv* env, const char* message, int errorCode)
{
    if (errorCode != 0)
    {
        // If a Java exception is already pending (e.g. one raised by a
        // Java InputStream/OutputStream callback that we propagated up
        // via early-return + ExceptionCheck), do NOT overwrite it with
        // a generic com.nokia.heif.Exception. Preserving the original
        // exception keeps the diagnostic context the Java caller needs.
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            return;
        }

        // Drain (and consume) any rich diagnostic context the HEIFPP layer
        // recorded for this thread alongside the integer error code.
        // The thread-local is read before we touch any other JNI machinery
        // so the context can never be corrupted by a JNI side-effect, and
        // it is cleared immediately after to avoid surfacing stale info on
        // the next checkError call from the same thread.
        //
        // IMPORTANT: copy the context BY VALUE -- `HEIFPP::getLastError()`
        // returns a `const ErrorContext&` that references the per-thread
        // slot directly, so binding it to a reference and then calling
        // `clearLastError()` would observably reset the very fields we
        // are about to read.  Without the copy, source would read
        // "Unknown" instead of "Api_State" / "Reader_*" because the slot
        // would be zeroed before the rich `throwException` call could
        // marshal it across the JNI boundary.
        const ::HEIFPP::ErrorContext ctx = ::HEIFPP::getLastError();
        ::HEIFPP::clearLastError();

        const jclass errorHandlerClass = env->FindClass("com/nokia/heif/ErrorHandler");
        if (errorHandlerClass == nullptr)
        {
            return;  // pending ClassNotFoundError will surface to Java
        }

        // We try to use the rich 6-arg overload when context is valid;
        // otherwise we fall back to the legacy 2-arg form so this works
        // unchanged with older Java-only test runners that haven't picked
        // up the new ErrorHandler.java.
        if (ctx.valid)
        {
            const jmethodID richMethod = env->GetStaticMethodID(
                errorHandlerClass, "throwException",
                "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;)V");
            if (richMethod != nullptr)
            {
                // Combine the JNI call-site message with the HEIFPP-level
                // context message so both layers' framing is visible. The
                // JNI string tells the user "which API call", the HEIFPP
                // string tells them "which validation/state issue".
                char combined[768];
                std::snprintf(combined, sizeof(combined), "%s: %s",
                              message ? message : "(no context)",
                              ctx.message.c_str());

                jstring jInfo   = env->NewStringUTF(combined);
                jstring jSource = env->NewStringUTF(::HEIFPP::errorSourceName(ctx.source));
                jstring jFile   = env->NewStringUTF(ctx.file ? ctx.file : "");
                jstring jCause  = env->NewStringUTF(ctx.causeWhat.c_str());

                env->CallStaticVoidMethod(errorHandlerClass, richMethod, errorCode,
                                          jInfo, jSource, jFile, static_cast<jint>(ctx.line), jCause);

                env->DeleteLocalRef(jInfo);
                env->DeleteLocalRef(jSource);
                env->DeleteLocalRef(jFile);
                env->DeleteLocalRef(jCause);
                env->DeleteLocalRef(errorHandlerClass);
                return;
            }
            // Rich method missing - clear the NoSuchMethodError JNI raised
            // and fall through to the legacy path so we don't lose the error.
            if (env->ExceptionCheck() == JNI_TRUE)
            {
                env->ExceptionClear();
            }
        }

        const jmethodID checkErrorID =
            env->GetStaticMethodID(errorHandlerClass, "throwException", "(ILjava/lang/String;)V");
        jstring errorMessage = env->NewStringUTF(message);
        env->CallStaticVoidMethod(errorHandlerClass, checkErrorID, errorCode, errorMessage);
        env->DeleteLocalRef(errorMessage);
        env->DeleteLocalRef(errorHandlerClass);
    }
}

namespace
{
    void throwJava(JNIEnv* env, const char* className, const char* message)
    {
        // Do not overwrite a pending Java exception (e.g. one that bubbled up
        // from a Java callback such as InputStream.read()).
        if (env->ExceptionCheck() == JNI_TRUE)
        {
            return;
        }
        jclass cls = env->FindClass(className);
        if (cls != nullptr)
        {
            env->ThrowNew(cls, message != nullptr ? message : "");
            env->DeleteLocalRef(cls);
        }
    }
}

void translateNativeException(JNIEnv* env, const std::exception* e, const char* context)
{
    char buffer[512];
    const char* what = (e != nullptr && e->what() != nullptr) ? e->what() : "";
    std::snprintf(buffer, sizeof(buffer), "%s: %s", context != nullptr ? context : "(no context)", what);

    if (dynamic_cast<const std::invalid_argument*>(e) != nullptr)
    {
        throwJava(env, "java/lang/IllegalArgumentException", buffer);
    }
    else if (dynamic_cast<const std::out_of_range*>(e) != nullptr)
    {
        throwJava(env, "java/lang/IndexOutOfBoundsException", buffer);
    }
    else if (dynamic_cast<const std::logic_error*>(e) != nullptr)
    {
        // Includes HEIF_ASSERT failures (std::logic_error).
        throwJava(env, "java/lang/IllegalStateException", buffer);
    }
    else if (dynamic_cast<const std::bad_alloc*>(e) != nullptr)
    {
        throwJava(env, "java/lang/OutOfMemoryError", buffer);
    }
    else
    {
        throwJava(env, "java/lang/RuntimeException", buffer);
    }
}

void translateUnknownNativeException(JNIEnv* env, const char* context)
{
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), "%s: unknown native exception",
                  context != nullptr ? context : "(no context)");
    throwJava(env, "java/lang/RuntimeException", buffer);
}
