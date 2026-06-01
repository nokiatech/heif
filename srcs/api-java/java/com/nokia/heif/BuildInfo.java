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
 */
package com.nokia.heif;

/**
 * Compile-time version of the HEIF Java API.
 * <p>
 * Mirror of {@code BuildInfo::Version} on the C++ side and the
 * {@code HEIF_VERSION_MAJOR / MINOR / PATCH} preprocessor macros.
 * Single source of truth for both: the {@code project(heif VERSION ...)}
 * call in {@code srcs/CMakeLists.txt}. This file is hand-maintained
 * because the Java tier intentionally has no preprocessor or build-time
 * codegen; bump the four constants below in lockstep with a CMake
 * version bump.
 * <p>
 * Embedders use this for compile-time / runtime version guards:
 * <pre>{@code
 * if (BuildInfo.atLeast(4, 0, 0)) {
 *     ErrorHandler.setErrorSink(ErrorHandler.STDERR_SINK);
 * }
 * }</pre>
 *
 * @since 4.0.0
 */
public final class BuildInfo
{
    /** Major version (e.g. {@code 4} for version 4.0.0). */
    public static final int MAJOR = 4;

    /** Minor version (e.g. {@code 0} for version 4.0.0). */
    public static final int MINOR = 0;

    /** Patch version (e.g. {@code 0} for version 4.0.0). */
    public static final int PATCH = 0;

    /**
     * Human-readable version string in {@code "M.m.p"} form
     * (e.g. {@code "4.0.0"}). Convenient for log lines and
     * {@code About} dialogs.
     */
    public static final String VERSION = MAJOR + "." + MINOR + "." + PATCH;

    /**
     * Encoded numeric version: {@code MAJOR<<16 | MINOR<<8 | PATCH}.
     * Mirrors the C macro {@code HEIF_VERSION_NUMBER}, suitable for
     * single-integer comparisons.
     */
    public static final int VERSION_NUMBER = (MAJOR << 16) | (MINOR << 8) | PATCH;

    /**
     * Compile-time / runtime version guard.
     * Returns {@code true} if the running Java API is at least the
     * supplied {@code MAJOR.MINOR.PATCH}, mirroring the C macro
     * {@code HEIF_VERSION_AT_LEAST}.
     */
    public static boolean atLeast(int major, int minor, int patch)
    {
        return VERSION_NUMBER >= ((major << 16) | (minor << 8) | patch);
    }

    /** Not instantiable. */
    private BuildInfo() {}
}
