/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

// https://gcc.gnu.org/wiki/Visibility

#ifndef HEIFFILEEXPORT_H
#define HEIFFILEEXPORT_H

#if (defined(HEIF_BUILDING_LIB) && !defined(HEIF_BUILDING_DLL)) || defined(HEIF_USE_STATIC_LIB)
    #define HEIF_DLL_PUBLIC
    #define HEIF_DLL_LOCAL
#else
    #if defined _WIN32 || defined __CYGWIN__
        #ifdef HEIF_BUILDING_DLL
            #ifdef __GNUC__
            #define HEIF_DLL_PUBLIC __attribute__((dllexport))
            #else
            #define HEIF_DLL_PUBLIC __declspec(dllexport)  // Note: actually gcc seems to also supports this syntax.
            #endif
        #else
            #ifdef __GNUC__
                #define HEIF_DLL_PUBLIC __attribute__((dllimport))
            #else
                #define HEIF_DLL_PUBLIC __declspec(dllimport)  // Note: actually gcc seems to also supports this syntax.
            #endif
        #endif
        #define HEIF_DLL_LOCAL
    #else
        #if __GNUC__ >= 4
            #define HEIF_DLL_PUBLIC __attribute__((visibility("default")))
            #define HEIF_DLL_LOCAL __attribute__((visibility("hidden")))
        #else
            #define HEIF_DLL_PUBLIC
            #define HEIF_DLL_LOCAL
        #endif
    #endif
#endif

#endif  // HEIFFILEEXPORT_H
