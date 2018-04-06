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

#pragma once
#include <jni.h>

#define NATIVE_DERIVED_IMAGE_ITEM(handle, object) \
    HEIFPP::DerivedImageItem* handle = (HEIFPP::DerivedImageItem*) getNativeHandle(env, object)
#define NATIVE_CODED_IMAGE_ITEM(handle, object) \
    HEIFPP::CodedImageItem* handle = (HEIFPP::CodedImageItem*) getNativeHandle(env, object)
#define NATIVE_GRID_IMAGE_ITEM(handle, object) HEIFPP::Grid* handle = (HEIFPP::Grid*) getNativeHandle(env, object)
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
    HEIFPP::CleanApertureProperty* nativeHandle = (HEIFPP::CleanApertureProperty*) getNativeHandle(env, obj)
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
#define CHECK_ERROR(errorCode, errorMessage) checkError(env, errorMessage, (int) errorCode)
#define GET_JAVA_ITEM(nativeHandle) getJavaItem(env, getJavaHEIF(env, obj), nativeHandle)

static const char* HEVC_DECODER_CONFIG_CLASS_NAME = "com/nokia/heif/HEVCDecoderConfig";
static const char* AVC_DECODER_CONFIG_CLASS_NAME  = "com/nokia/heif/AVCDecoderConfig";

static const char* HEVC_IMAGE_ITEM_CLASS_NAME     = "com/nokia/heif/HEVCImageItem";
static const char* AVC_IMAGE_ITEM_CLASS_NAME      = "com/nokia/heif/AVCImageItem";
static const char* CODED_IMAGE_ITEM_CLASS_NAME    = "com/nokia/heif/CodedImageItem";
static const char* GRID_IMAGE_ITEM_CLASS_NAME     = "com/nokia/heif/GridImageItem";
static const char* IDENTITY_IMAGE_ITEM_CLASS_NAME = "com/nokia/heif/IdentityImageItem";
static const char* OVERLAY_IMAGE_ITEM_CLASS_NAME  = "com/nokia/heif/OverlayImageItem";
static const char* EXIF_ITEM_CLASS_NAME           = "com/nokia/heif/ExifItem";
static const char* MPEG7_ITEM_CLASS_NAME          = "com/nokia/heif/MPEG7Item";
static const char* XMP_ITEM_CLASS_NAME            = "com/nokia/heif/XMPItem";


static const char* AUXILIARY_PROPERTY_CLASS_NAME          = "com/nokia/heif/AuxiliaryProperty";
static const char* CLEAN_APERTURE_PROPERTY_CLASS_NAME     = "com/nokia/heif/CleanApertureProperty";
static const char* ICC_COLOUR_PROPERTY_CLASS_NAME         = "com/nokia/heif/ICCColourProperty";
static const char* NCLX_COLOUR_PROPERTY_CLASS_NAME        = "com/nokia/heif/NCLXColourProperty";
static const char* MIRROR_PROPERTY_CLASS_NAME             = "com/nokia/heif/MirrorProperty";
static const char* ROTATE_PROPERTY_CLASS_NAME             = "com/nokia/heif/RotateProperty";
static const char* PIXEL_ASPECT_RATIO_PROPERTY_CLASS_NAME = "com/nokia/heif/PixelAspectRatioProperty";
static const char* PIXEL_INFORMATION_PROPERTY_CLASS_NAME  = "com/nokia/heif/PixelInformationProperty";
static const char* RELATIVE_LOCATION_PROPERTY_CLASS_NAME  = "com/nokia/heif/RelativeLocationProperty";


jobject getDecoderConfig(JNIEnv* env, jobject parentJavaHEIF, void* nativeConfig);

jobject getJavaItem(JNIEnv* env, jobject parentJavaHEIF, void* item);


template <class type>
jobject getJavaObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle);

jobject getJavaItemProperty(JNIEnv* env, jobject parentJavaHEIF, void* itemProperty);

template <class type>
jobject createBaseObject(JNIEnv* env, jobject parentJavaHEIF, const char* className, type nativeHandle);

jlong getNativeHandle(JNIEnv* env, jobject obj);
void setNativeHandle(JNIEnv* env, jobject obj, jlong handle);


void releaseJavaHandles(JNIEnv* env, jobject obj);

jobject getJavaHEIF(JNIEnv* env, jobject obj);

void checkError(JNIEnv* env, const char* message, int errorCode);