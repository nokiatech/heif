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

#include "Heif.h"
#include "Helpers.h"
#include "InputStream.h"
#include "OutputStream.h"

#include <new>        // std::bad_alloc - test triggers
#include <stdexcept>  // std::invalid_argument / out_of_range / runtime_error - test triggers
#include "heifcommondatatypes.h"
#include "helpers.h"
#include "jni_md.h"
#include <stdint.h>

#define CLASS_NAME HEIF
extern "C"
{
    JNI_METHOD(void, createInstanceNative)
    {
        JNI_TRY
        {
            auto* instance = new HEIFPP::Heif();
            setNativeHandle(env, self, (jlong) instance);
        }
        JNI_CATCH_RETURN_VOID("HEIF.createInstanceNative");
    }

    JNI_METHOD(void, destroyInstanceNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        delete nativeHandle;
        setNativeHandle(env, self, 0);
            }
        JNI_CATCH_RETURN_VOID("HEIF.destroyInstanceNative")
    }

    JNI_METHOD_ARG(void, loadNative, jstring filename, jint preloadMode)
    {
        JNI_TRY
        {
            NATIVE_HEIF(nativeHandle, self);
            const char* nativeFilename = env->GetStringUTFChars(filename, nullptr);

            HEIFPP::Result error = nativeHandle->load(nativeFilename, static_cast<HEIFPP::Heif::PreloadMode>(preloadMode));
            env->ReleaseStringUTFChars(filename, nativeFilename);
            CHECK_ERROR(error, "Loading failed");
        }
        JNI_CATCH_RETURN_VOID("HEIF.loadNative");
    }

    JNI_METHOD_ARG(void, loadStreamNative, jobject stream, jint preloadMode)
    {
        JNI_TRY
        {
            NATIVE_HEIF(nativeHandle, self);
            auto* inputStream    = new InputStream(env, stream);
            HEIFPP::Result error = nativeHandle->load(inputStream, static_cast<HEIFPP::Heif::PreloadMode>(preloadMode));
            delete inputStream;
            CHECK_ERROR(error, "Loading failed");
        }
        JNI_CATCH_RETURN_VOID("HEIF.loadStreamNative");
    }

    JNI_METHOD_ARG(void, saveNative, jstring filename)
    {
        JNI_TRY
        {
            NATIVE_HEIF(nativeHandle, self);
            const char* nativeFilename = env->GetStringUTFChars(filename, nullptr);

            HEIFPP::Result error = nativeHandle->save(nativeFilename);

            env->ReleaseStringUTFChars(filename, nativeFilename);
            CHECK_ERROR(error, "Saving failed");
        }
        JNI_CATCH_RETURN_VOID("HEIF.saveNative");
    }

    JNI_METHOD_ARG(void, saveStreamNative, jobject stream)
    {
        JNI_TRY
        {
            NATIVE_HEIF(nativeHandle, self);
            auto* outputStream   = new OutputStream(env, stream);
            HEIFPP::Result error = nativeHandle->save(outputStream);
            delete outputStream;
            CHECK_ERROR(error, "Saving failed");
        }
        JNI_CATCH_RETURN_VOID("HEIF.saveStreamNative");
    }

    JNI_METHOD(jint, getItemCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getItemCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getItemCountNative")
    }

    JNI_METHOD_ARG(jobject, getItemNative, jint itemIndex)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::Item* item = nativeHandle->getItem(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getItemNative")
    }

    JNI_METHOD(jint, getImageCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getImageCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getImageCountNative")
    }

    JNI_METHOD_ARG(jobject, getImageNative, jint itemIndex)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::ImageItem* item = nativeHandle->getImage(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getImageNative")
    }

    JNI_METHOD(jint, getMasterImageCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getMasterImageCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getMasterImageCountNative")
    }

    JNI_METHOD_ARG(jobject, getMasterImageNative, jint itemIndex)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        HEIFPP::ImageItem* item = nativeHandle->getMasterImage(static_cast<uint32_t>(itemIndex));
        return getJavaItem(env, self, item);
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getMasterImageNative")
    }

    JNI_METHOD_ARG(jint, getItemsOfTypeCountNative, jstring type)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        const char* nativeString = env->GetStringUTFChars(type, nullptr);

        jint itemCount = static_cast<jint>(nativeHandle->getItemsOfTypeCount(HEIF::FourCC(nativeString)));

        env->ReleaseStringUTFChars(type, nativeString);
        return static_cast<jint>(itemCount);
            }
        JNI_CATCH_RETURN(0, "HEIF.getItemsOfTypeCountNative")
    }

    JNI_METHOD_ARG(jobject, getItemOfTypeNative, jstring type, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        const char* nativeString = env->GetStringUTFChars(type, nullptr);
        jobject javaObject       = getJavaItem(
            env, self, nativeHandle->getItemOfType(HEIF::FourCC(nativeString), static_cast<uint32_t>(index)));
        env->ReleaseStringUTFChars(type, nativeString);
        return javaObject;
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getItemOfTypeNative")
    }

    JNI_METHOD(jobject, getPrimaryItemNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaItem(env, self, nativeHandle->getPrimaryItem());
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getPrimaryItemNative")
    }

    JNI_METHOD_ARG(void, setPrimaryItemNative, jobject item)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        NATIVE_IMAGE_ITEM(nativeItem, item);
        nativeHandle->setPrimaryItem(nativeItem);
            }
        JNI_CATCH_RETURN_VOID("HEIF.setPrimaryItemNative")
    }

    JNI_METHOD(jint, getCompatibleBrandsCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->compatibleBrands());
            }
        JNI_CATCH_RETURN(0, "HEIF.getCompatibleBrandsCountNative")
    }

    JNI_METHOD_ARG(jstring, getCompatibleBrandNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        HEIF::FourCC brand = nativeHandle->getCompatibleBrand(static_cast<uint32_t>(index));
        return env->NewStringUTF(brand.value);
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getCompatibleBrandNative")
    }

    JNI_METHOD_ARG(void, addCompatibleBrandNative, jstring brand)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        const char* nativeBrand = env->GetStringUTFChars(brand, nullptr);
        nativeHandle->addCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
            }
        JNI_CATCH_RETURN_VOID("HEIF.addCompatibleBrandNative")
    }

    JNI_METHOD_ARG(void, removeCompatibleBrandNative, jstring brand)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        const char* nativeBrand = env->GetStringUTFChars(brand, nullptr);
        nativeHandle->removeCompatibleBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
            }
        JNI_CATCH_RETURN_VOID("HEIF.removeCompatibleBrandNative")
    }

    JNI_METHOD(jstring, getMajorBrandNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        HEIF::FourCC brand = nativeHandle->getMajorBrand();
        return env->NewStringUTF(brand.value);
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getMajorBrandNative")
    }

    JNI_METHOD_ARG(void, setMajorBrandNative, jstring brand)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        const char* nativeBrand = env->GetStringUTFChars(brand, nullptr);
        nativeHandle->setMajorBrand(HEIF::FourCC(nativeBrand));
        env->ReleaseStringUTFChars(brand, nativeBrand);
            }
        JNI_CATCH_RETURN_VOID("HEIF.setMajorBrandNative")
    }

    JNI_METHOD(jint, getPropertyCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getPropertyCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getPropertyCountNative")
    }

    JNI_METHOD_ARG(jobject, getPropertyNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaItemProperty(env, self, nativeHandle->getProperty(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getPropertyNative")
    }

    JNI_METHOD(jint, getTrackCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return static_cast<jint>(nativeHandle->getTrackCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getTrackCountNative")
    }

    JNI_METHOD_ARG(jobject, getTrackNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeHandle, self);
        return getJavaTrack(env, self, nativeHandle->getTrack(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getTrackNative")
    }

    JNI_METHOD(jint, getAlternativeTrackGroupCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeSelf, self);
        return static_cast<jint>(nativeSelf->getAlternativeTrackGroupCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getAlternativeTrackGroupCountNative")
    }

    JNI_METHOD_ARG(jobject, getAlternativeTrackGroupNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeSelf, self);
        return getJavaAlternativeTrackGroup(env, self,
                                            nativeSelf->getAlternativeTrackGroup(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getAlternativeTrackGroupNative")
    }

    JNI_METHOD(jint, getEntityGroupCountNative)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeSelf, self);
        return static_cast<jint>(nativeSelf->getGroupCount());
            }
        JNI_CATCH_RETURN(0, "HEIF.getEntityGroupCountNative")
    }

    JNI_METHOD_ARG(jobject, getEntityGroupNative, jint index)
    {
        JNI_TRY
        {
        NATIVE_HEIF(nativeSelf, self);
        return getJavaEntityGroup(env, self, nativeSelf->getGroup(static_cast<uint32_t>(index)));
            }
        JNI_CATCH_RETURN(nullptr, "HEIF.getEntityGroupNative")
    }

    /*
     * Diagnostic hook. Trips a HEIF_ASSERT unconditionally so the JNI
     * exception-translation firewall can be verified. The HEIFPP layer
     * throws std::logic_error from HEIF_ASSERT, which the translator
     * maps to java.lang.IllegalStateException.
     *
     * Note: declared "static native" on the Java side, so JNI passes a
     * jclass in place of the usual jobject; the layout is ABI-identical
     * (both are pointer-to-_jobject) so the JNI_METHOD macro works.
     */
    JNI_METHOD(void, triggerHeifAssertForTesting)
    {
        JNI_TRY
        {
            HEIF_ASSERT(false);
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerHeifAssertForTesting")
    }

    /*
     * Diagnostic hooks for the JNI exception-translation table. Each one
     * deliberately throws one std::exception subclass to verify the
     * mapping in translateNativeException (Helpers.cpp):
     *
     *   std::invalid_argument  -> java.lang.IllegalArgumentException
     *   std::out_of_range      -> java.lang.IndexOutOfBoundsException
     *   std::bad_alloc         -> java.lang.OutOfMemoryError
     *   std::runtime_error     -> java.lang.RuntimeException (catch-all
     *                             for "any other std::exception")
     *   non-std throw  (...)   -> java.lang.RuntimeException("unknown ...")
     *
     * Do not call from production code; these are deliberate tripwires.
     */
    JNI_METHOD(void, triggerInvalidArgumentForTesting)
    {
        JNI_TRY
        {
            throw std::invalid_argument("test: invalid_argument tripwire");
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerInvalidArgumentForTesting")
    }

    JNI_METHOD(void, triggerOutOfRangeForTesting)
    {
        JNI_TRY
        {
            throw std::out_of_range("test: out_of_range tripwire");
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerOutOfRangeForTesting")
    }

    JNI_METHOD(void, triggerBadAllocForTesting)
    {
        JNI_TRY
        {
            throw std::bad_alloc();
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerBadAllocForTesting")
    }

    JNI_METHOD(void, triggerRuntimeErrorForTesting)
    {
        JNI_TRY
        {
            throw std::runtime_error("test: runtime_error tripwire");
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerRuntimeErrorForTesting")
    }

    JNI_METHOD(void, triggerUnknownThrowForTesting)
    {
        JNI_TRY
        {
            throw 42;  // non-std type, must hit catch(...)
        }
        JNI_CATCH_RETURN_VOID("HEIF.triggerUnknownThrowForTesting")
    }
}
