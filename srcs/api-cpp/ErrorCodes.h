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

#pragma once

#include <cstdint>
#include "heifcommondatatypes.h"

namespace HEIFPP
{
    /**
     * Result codes returned by HEIFPP API methods, and propagated to the
     * Java layer via ErrorHandler.throwException as the integer error code
     * inside com.nokia.heif.Exception.
     *
     * Codes 0..9 are STABLE: existing client code (C++ and Java) treats
     * them by name AND by integer value. Do not renumber them.
     *
     * Codes 10..99 are mapped 1:1 from the lower-level HEIF::ErrorCode
     * enum so that callers can distinguish e.g. "file does not exist"
     * (FILE_OPEN_ERROR) from "file is corrupt" (MEDIA_PARSING_ERROR /
     * BYTESTREAM_READ_ERROR / FILE_HEADER_ERROR) from "ID not found in
     * file" (INVALID_ITEM_ID etc). Before this expansion every one of
     * those collapsed to ERROR_UNDEFINED, leaving Java callers with no
     * way to surface a meaningful error message to the user.
     *
     * ERROR_UNDEFINED (999) remains as the catch-all for any future
     * HEIF::ErrorCode value not yet mapped here.
     *
     * Underlying type pinned to std::uint16_t (must hold
     * ERROR_UNDEFINED = 999).
     */
    enum class Result : std::uint16_t
    {
        // --- HEIFPP-level codes (0..9, STABLE, do not renumber) -----------
        OK                    = 0,
        INDEX_OUT_OF_BOUNDS   = 1,
        ALREADY_INITIALIZED   = 2,
        PRIMARY_ITEM_NOT_SET  = 3,
        BRANDS_NOT_SET        = 4,
        HIDDEN_PRIMARY_ITEM   = 5,
        INVALID_HANDLE        = 6,
        ALREADY_IN_GROUP      = 7,
        ALREADY_SET           = 8,
        INVALID_CONFIG        = 9,

        // --- Direct mappings of HEIF::ErrorCode (10..99) ------------------
        // File / IO
        FILE_OPEN_ERROR              = 10,  ///< Could not open the file.
        FILE_READ_ERROR              = 11,  ///< Read from underlying stream failed.
        FILE_HEADER_ERROR            = 12,  ///< File header (ftyp) is malformed.
        BUFFER_SIZE_TOO_SMALL        = 13,  ///< Caller-supplied buffer too small.
        BYTESTREAM_READ_ERROR        = 14,  ///< Bitstream read past end (truncated/malformed).
        MEMORY_TOO_SMALL_BUFFER      = 15,  ///< Internal allocation too small.

        // State
        UNINITIALIZED                = 20,  ///< Reader/Writer used before initialize().
        NOT_APPLICABLE               = 21,  ///< Operation not valid for this file/state.
        FTYP_ALREADY_WRITTEN         = 22,  ///< Writer state error.
        ALLOCATOR_ALREADY_SET        = 23,  ///< Custom allocator already installed.

        // Bad parameters
        INVALID_FUNCTION_PARAMETER   = 30,  ///< Bad arg to API function.
        INVALID_ITEM_ID              = 31,  ///< No item with that id.
        INVALID_GROUP_ID             = 32,  ///< No group with that id.
        INVALID_MEDIADATA_ID         = 33,  ///< No media-data with that id.
        INVALID_METADATAITEM_ID      = 34,  ///< No metadata item with that id.
        INVALID_PROPERTY_INDEX       = 35,  ///< Property index out of range.
        INVALID_REFERENCE_COUNT      = 36,  ///< Reference count mismatch.
        INVALID_SAMPLE_DESCRIPTION_INDEX = 37,
        INVALID_SEGMENT              = 38,
        INVALID_SEQUENCE_ID          = 39,
        INVALID_DECODER_CONFIG_ID    = 40,
        INVALID_SEQUENCE_IMAGE_ID    = 41,

        // Content
        MEDIA_PARSING_ERROR          = 50,  ///< Bitstream content malformed.
        INVALID_MEDIA_FORMAT         = 51,  ///< Unrecognised media format.
        UNSUPPORTED_CODE_TYPE        = 52,  ///< Codec type not supported.
        DECODER_CONFIGURATION_ERROR  = 53,  ///< decoder config parse failure.
        NO_MEDIA                     = 54,  ///< Item has no media data.
        CONTENT_TYPE_NOT_SET         = 55,  ///< Mime item content type missing.
        PROTECTED_ITEM               = 56,  ///< Item is DRM-protected.
        UNPROTECTED_ITEM             = 57,  ///< Item expected to be protected, isn't.

        // Mini / extension
        TOO_MANY_ITEMS               = 60,  ///< (mini) item count over limit.
        MISSING_PROPERTIES           = 61,  ///< (mini) required property missing.
        MULTIPLE_GAINMAPS            = 62,  ///< (mini) more than one gainmap.
        INVALID_EXIF_ORIENTATION_VALUE = 63,
        VVC_DISABLED                 = 64,  ///< Legacy / now unused.

        // Catch-all
        ERROR_UNDEFINED              = 999  ///< Unmapped lower-level error.
    };

    Result convertErrorCode(HEIF::ErrorCode error);
}  // namespace HEIFPP
