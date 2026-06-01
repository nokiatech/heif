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

#pragma once
#include <jni.h>

#include <exception>
#include "jni_md.h"

#define NATIVE_IMAGE_ITEM(handle, object) HEIFPP::ImageItem* handle = (HEIFPP::ImageItem*) getNativeHandle(env, object)
#define NATIVE_ITEM(handle, object) HEIFPP::Item* handle = (HEIFPP::Item*) getNativeHandle(env, object)
#define NATIVE_ITEM_PROPERTY(handle, object) \
    HEIFPP::ItemProperty* handle = (HEIFPP::ItemProperty*) getNativeHandle(env, object)
#define NATIVE_META_ITEM(handle, object) HEIFPP::MetaItem* handle = (HEIFPP::MetaItem*) getNativeHandle(env, object)
#define NATIVE_DECODER_CONFIG(handle, object) \
    HEIFPP::DecoderConfig* handle = (HEIFPP::DecoderConfig*) getNativeHandle(env, object)
#define NATIVE_HEIF(handle, object) HEIFPP::Heif* handle = (HEIFPP::Heif*) getNativeHandle(env, object)


// JNI native-handle and error-check macros. The macro arguments are
// either type-names (used in C-cast context where parenthesization is
// invalid) or identifiers immediately followed by `=` / used as variable
// declarators where parenthesization is also invalid. The macros expand
// into JNI signatures whose grammar is fixed by the Java side.
// NOLINTBEGIN(bugprone-macro-parentheses)
#define NATIVE_SELF HEIFPP::CLASS_NAME* nativeSelf = (HEIFPP::CLASS_NAME*) getNativeHandle(env, self)

#define NATIVE_TRACK(handle, object) HEIFPP::Track* handle = (HEIFPP::Track*) getNativeHandle(env, object)

#define NATIVE_SAMPLE(handle, object) HEIFPP::Sample* handle = (HEIFPP::Sample*) getNativeHandle(env, object)
#define NATIVE_AUDIO_SAMPLE(handle, object) \
    HEIFPP::AudioSample* handle = (HEIFPP::AudioSample*) getNativeHandle(env, object)
#define NATIVE_VIDEO_SAMPLE(handle, object) \
    HEIFPP::VideoSample* handle = (HEIFPP::VideoSample*) getNativeHandle(env, object)

#define CHECK_ERROR(errorCode, errorMessage) checkError(env, errorMessage, (int) errorCode)
#define GET_JAVA_ITEM(nativeHandle) getJavaItem(env, getJavaHEIF(env, self), nativeHandle)
// NOLINTEND(bugprone-macro-parentheses)


#define IMPL_JNI_METHOD(application_identity, return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_##application_identity##_##method_name
#define EVALUATE_JNI_METHOD(application_identity, return_type, method_name) \
    IMPL_JNI_METHOD(application_identity, return_type, method_name)

// `self` is annotated [[maybe_unused]] because some JNI bodies don't reference it (the
// native side reaches state through the native-handle pointer instead). Without the
// attribute every such method emits a -Wunused-parameter warning.
#define JNI_METHOD_ARG(return_type, method_name, ...)                                                                    \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, [[maybe_unused]] jobject self, __VA_ARGS__); \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, [[maybe_unused]] jobject self, __VA_ARGS__)

#define JNI_METHOD(return_type, method_name)                                                                \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, [[maybe_unused]] jobject self); \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, [[maybe_unused]] jobject self)


jobject getDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, void* nativeConfig);

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* item);

jobject getJavaItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* itemProperty);
jobject getJavaTrack(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject);
jobject getJavaSample(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject);
jobject getJavaAlternativeTrackGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject);
jobject getJavaEntityGroup(JNIEnv* env, jobject parentJavaHEIF, void* nativeObject);

template <class type>
jobject createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle);

jlong getNativeHandle(JNIEnv* env, jobject obj);
void setNativeHandle(JNIEnv* env, jobject obj, jlong handle);


void releaseJavaHandles(JNIEnv* env, jobject obj);

jobject getJavaHEIF(JNIEnv* env, jobject obj);

void checkError(JNIEnv* env, const char* message, int errorCode);

/**
 * Translate a C++ exception into a Java exception so it does not propagate
 * across the JNI boundary (which is undefined behavior and typically aborts
 * the process on Android / iOS). Call from a catch block; the corresponding
 * native return value should be the type's default-constructed value.
 *
 *   - std::logic_error           -> java.lang.IllegalStateException
 *                                   (raised by HEIF_ASSERT, internal invariants)
 *   - std::invalid_argument      -> java.lang.IllegalArgumentException
 *   - std::out_of_range          -> java.lang.IndexOutOfBoundsException
 *                                   (e.g. std::vector::at on truncated streams)
 *   - std::bad_alloc             -> java.lang.OutOfMemoryError
 *   - any other std::exception   -> java.lang.RuntimeException
 *   - non-std exception (...)    -> java.lang.RuntimeException("unknown native exception")
 *
 * If a Java exception is already pending (e.g. raised from within the native
 * code via env->ThrowNew), this function leaves it alone.
 */
void translateNativeException(JNIEnv* env, const std::exception* e, const char* context);
void translateUnknownNativeException(JNIEnv* env, const char* context);

/**
 * Wrap a JNI method body to convert any escaping C++ exception into a Java
 * exception. Use as:
 *
 *     JNI_METHOD_ARG(jint, fooNative, jint x)
 *     {
 *         JNI_TRY
 *         {
 *             NATIVE_HEIF(h, self);
 *             return h->doFoo(x);
 *         }
 *         JNI_CATCH_RETURN(0, "fooNative");
 *     }
 *
 * The JNI_CATCH_RETURN(default_value, context_string) macro:
 *   - converts std::exception (and subclasses) to a Java exception
 *   - converts any unknown throw to RuntimeException
 *   - returns `default_value` so that the JNI return-value contract is met
 *     (control returns to the JVM which will see the pending exception and
 *     throw it on the Java side)
 *
 * For void-returning methods use JNI_CATCH_RETURN_VOID(context_string).
 */
#define JNI_TRY try

#define JNI_CATCH_RETURN(default_value, context)                           \
    catch (const std::exception& _e)                                       \
    {                                                                      \
        translateNativeException(env, &_e, context);                       \
        return default_value;                                              \
    }                                                                      \
    catch (...)                                                            \
    {                                                                      \
        translateUnknownNativeException(env, context);                     \
        return default_value;                                              \
    }

#define JNI_CATCH_RETURN_VOID(context)                                     \
    catch (const std::exception& _e)                                       \
    {                                                                      \
        translateNativeException(env, &_e, context);                       \
        return;                                                            \
    }                                                                      \
    catch (...)                                                            \
    {                                                                      \
        translateUnknownNativeException(env, context);                     \
        return;                                                            \
    }

#define UNUSED(x) (void) (x)
