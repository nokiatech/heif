/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

#define NATIVE_DERIVED_IMAGE_ITEM(handle, object) \
    HEIFPP::DerivedImageItem* handle = (HEIFPP::DerivedImageItem*) getNativeHandle(env, object)
#define NATIVE_CODED_IMAGE_ITEM(handle, object) \
    HEIFPP::CodedImageItem* handle = (HEIFPP::CodedImageItem*) getNativeHandle(env, object)
#define NATIVE_GRID_IMAGE_ITEM(handle, object) HEIFPP::GridImageItem* handle = (HEIFPP::GridImageItem*) getNativeHandle(env, object)
#define NATIVE_OVERLAY_IMAGE_ITEM(handle, object) \
    HEIFPP::Overlay* handle = (HEIFPP::Overlay*) getNativeHandle(env, object)
#define NATIVE_IDENTITY_IMAGE_ITEM(handle, object) \
    HEIFPP::Identity* handle = (HEIFPP::Identity*) getNativeHandle(env, object)
#define NATIVE_IMAGE_ITEM(handle, object) HEIFPP::ImageItem* handle = (HEIFPP::ImageItem*) getNativeHandle(env, object)
#define NATIVE_ITEM(handle, object) HEIFPP::Item* handle = (HEIFPP::Item*) getNativeHandle(env, object)
#define NATIVE_ITEM_PROPERTY(handle, object) \
    HEIFPP::ItemProperty* handle = (HEIFPP::ItemProperty*) getNativeHandle(env, object)
#define NATIVE_ROTATE_PROPERTY(handle, object) \
    HEIFPP::RotateProperty* handle = (HEIFPP::RotateProperty*) getNativeHandle(env, object)
#define NATIVE_MIRROR_PROPERTY(handle, object) \
    HEIFPP::MirrorProperty* handle = (HEIFPP::MirrorProperty*) getNativeHandle(env, object)
#define NATIVE_CLAP(handle, object) \
    HEIFPP::CleanApertureProperty* nativeHandle = (HEIFPP::CleanApertureProperty*) getNativeHandle(env, object)
#define NATIVE_PIXEL_INFORMATION_PROPERTY(handle, object) \
    HEIFPP::PixelInformationProperty* handle = (HEIFPP::PixelInformationProperty*) getNativeHandle(env, object)
#define NATIVE_PIXEL_ASPECT_RATIO_PROPERTY(handle, object) \
    HEIFPP::PixelAspectRatioProperty* handle = (HEIFPP::PixelAspectRatioProperty*) getNativeHandle(env, object)
#define NATIVE_COLOUR_INFORMATION_PROPERTY(handle, object) \
    HEIFPP::ColourInformationProperty* handle = (HEIFPP::ColourInformationProperty*) getNativeHandle(env, object)
#define NATIVE_RELATIVE_LOCATION_PROPERTY(handle, object) \
    HEIFPP::RelativeLocationProperty* handle = (HEIFPP::RelativeLocationProperty*) getNativeHandle(env, object)
#define NATIVE_AUXILIARY_PROPERTY(handle, object) \
    HEIFPP::AuxProperty* handle = (HEIFPP::AuxProperty*) getNativeHandle(env, object)
#define NATIVE_META_ITEM(handle, object) HEIFPP::MetaItem* handle = (HEIFPP::MetaItem*) getNativeHandle(env, object)
#define NATIVE_MIME_ITEM(handle, object) HEIFPP::MimeItem* handle = (HEIFPP::MimeItem*) getNativeHandle(env, object)
#define NATIVE_EXIF_ITEM(handle, object) HEIFPP::ExifItem* handle = (HEIFPP::ExifItem*) getNativeHandle(env, object)
#define NATIVE_DECODER_CONFIG(handle, object) \
    HEIFPP::DecoderConfiguration* handle = (HEIFPP::DecoderConfiguration*) getNativeHandle(env, object)
#define NATIVE_HEIF(handle, object) HEIFPP::Heif* handle = (HEIFPP::Heif*) getNativeHandle(env, object)


#define NATIVE_SELF HEIFPP::CLASS_NAME* nativeSelf = (HEIFPP::CLASS_NAME*) getNativeHandle(env, self)

#define NATIVE_TRACK(handle, object) HEIFPP::Track* handle = (HEIFPP::Track*) getNativeHandle(env, object)

#define NATIVE_AUDIO_TRACK(handle, object) \
    HEIFPP::AudioTrack* handle = (HEIFPP::AudioTrack*) getNativeHandle(env, object)
#define NATIVE_VIDEO_TRACK(handle, object) \
    HEIFPP::VideoTrack* handle = (HEIFPP::VideoTrack*) getNativeHandle(env, object)
#define NATIVE_ALTERNATIVE_TRACK_GROUP(handle, object) \
    HEIFPP::AlternativeTrackGroup* handle = (HEIFPP::AlternativeTrackGroup*) getNativeHandle(env, object)

#define NATIVE_SAMPLE(handle, object) HEIFPP::Sample* handle = (HEIFPP::Sample*) getNativeHandle(env, object)
#define NATIVE_AUDIO_SAMPLE(handle, object) \
    HEIFPP::AudioSample* handle = (HEIFPP::AudioSample*) getNativeHandle(env, object)
#define NATIVE_VIDEO_SAMPLE(handle, object) \
    HEIFPP::VideoSample* handle = (HEIFPP::VideoSample*) getNativeHandle(env, object)

#define NATIVE_ENTITY_GROUP(handle, object) \
    HEIFPP::EntityGroup* handle = (HEIFPP::EntityGroup*) getNativeHandle(env, object)
#define NATIVE_EQUIVALENCE_GROUP(handle, object) \
    HEIFPP::EqivGroup* handle = (HEIFPP::EqivGroup*) getNativeHandle(env, object)

#define CHECK_ERROR(errorCode, errorMessage) checkError(env, errorMessage, (int) errorCode)
#define GET_JAVA_ITEM(nativeHandle) getJavaItem(env, getJavaHEIF(env, self), nativeHandle)


#define IMPL_JNI_METHOD(application_identity, return_type, method_name) \
    JNIEXPORT return_type JNICALL Java_com_nokia_heif_##application_identity##_##method_name
#define EVALUATE_JNI_METHOD(application_identity, return_type, method_name) \
    IMPL_JNI_METHOD(application_identity, return_type, method_name)

#define JNI_METHOD_ARG(return_type, method_name, ...)                                                   \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, jobject self, __VA_ARGS__); \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, jobject self, __VA_ARGS__)

#define JNI_METHOD(return_type, method_name)                                               \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, jobject self); \
    EVALUATE_JNI_METHOD(CLASS_NAME, return_type, method_name)(JNIEnv * env, jobject self)

#define GET_JAVA_OBJECT(nativeHandle) reinterpret_cast<jobject>(const_cast<void*>(nativeHandle->getContext()))

jobject getDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, void* nativeConfig);

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* item);


template <class type>
jobject getJavaObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle);

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