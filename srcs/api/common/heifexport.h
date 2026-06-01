/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

// Two-macro split for WebAssembly / JS build support.
//
//   HEIF_DLL_PUBLIC      -- visibility / dllimport-export tag. Legal on
//                          BOTH type declarations (class/struct) and on
//                          functions / non-local variables. Carries
//                          `__attribute__((visibility("default")))` on
//                          ELF, `__declspec(dllexport|dllimport)` on
//                          Windows, and is EMPTY on Emscripten (wasm is
//                          always a static-library build flavour, so
//                          there is nothing to "export" in the ELF /
//                          PE sense -- see HEIF_DLL_FN_EXPORT below).
//
//   HEIF_DLL_FN_EXPORT   -- wasm self-tagging for the C facade. Carries
//                          `EMSCRIPTEN_KEEPALIVE` (= `__attribute__((used))`)
//                          on Emscripten so `wasm-ld` will not strip the
//                          symbol and consumers do NOT have to maintain
//                          an `EXPORTED_FUNCTIONS=[...]` list of ~80
//                          `_heif_*` strings on their emcc link line.
//                          On every non-emcc toolchain this macro is just
//                          an alias for `HEIF_DLL_PUBLIC`, so the existing
//                          dllexport / visibility behavior on Windows
//                          and Linux shared-library builds is preserved.
//
// Why the split? `__attribute__((used))` is only meaningful on functions
// and non-local variables. Clang/emcc emits `-Wignored-attributes` when
// it is applied to a class/struct declaration, and the previous single
// `HEIF_DLL_PUBLIC EMSCRIPTEN_KEEPALIVE` definition triggered that
// warning ~60 times per translation unit on every wasm build. Splitting
// the responsibilities lets each macro stay where it is semantically
// valid: `HEIF_DLL_PUBLIC` on every public *type* and function (for
// visibility), `HEIF_DLL_FN_EXPORT` on every C-facade *function* (for
// wasm self-export). Type declarations under emcc end up with no
// attribute at all, which is exactly what wasm needs -- there is no
// per-type symbol to keep alive in the first place.
//
// clang-format off
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define HEIF_DLL_PUBLIC
    #define HEIF_DLL_LOCAL
    #define HEIF_DLL_FN_EXPORT EMSCRIPTEN_KEEPALIVE
#else
// Retain indented preprocessor directives to keep nested ifs more readable.
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
    // Non-emcc toolchains: function self-export tag is just the regular
    // visibility / dllexport tag. No KEEPALIVE concept exists outside wasm.
    #define HEIF_DLL_FN_EXPORT HEIF_DLL_PUBLIC
#endif  // !__EMSCRIPTEN__
// clang-format on

#endif  // HEIFFILEEXPORT_H
