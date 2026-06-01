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

#include "ErrorCodes.h"
#include "heifcommondatatypes.h"

namespace HEIFPP
{
    // Map a low-level HEIF::ErrorCode into the HEIFPP::Result that gets
    // exposed to client code (C++) and ultimately to Java callers via
    // com.nokia.heif.Exception.getErrorCode().
    //
    // Historically this returned ERROR_UNDEFINED for everything except a
    // tiny handful of values, which made every parser/IO failure look
    // identical to embedders ("UNDEFINED_ERROR 999"). The expanded
    // mapping below preserves the small set of pre-existing codes (0..9)
    // unchanged and adds direct 1:1 mappings for the rest, so a Java
    // caller can distinguish e.g. "file not found" from "file corrupt"
    // from "ID does not exist".
    Result convertErrorCode(HEIF::ErrorCode error)
    {
        switch (error)
        {
            // OK
            case HEIF::ErrorCode::OK:                              return Result::OK;

            // Pre-existing mappings (kept for binary/source compatibility)
            case HEIF::ErrorCode::ALREADY_INITIALIZED:             return Result::ALREADY_INITIALIZED;
            case HEIF::ErrorCode::PRIMARY_ITEM_NOT_SET:            return Result::PRIMARY_ITEM_NOT_SET;
            case HEIF::ErrorCode::BRANDS_NOT_SET:                  return Result::BRANDS_NOT_SET;
            case HEIF::ErrorCode::HIDDEN_PRIMARY_ITEM:             return Result::HIDDEN_PRIMARY_ITEM;

            // File / IO
            case HEIF::ErrorCode::FILE_OPEN_ERROR:                 return Result::FILE_OPEN_ERROR;
            case HEIF::ErrorCode::FILE_READ_ERROR:                 return Result::FILE_READ_ERROR;
            case HEIF::ErrorCode::FILE_HEADER_ERROR:               return Result::FILE_HEADER_ERROR;
            case HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL:           return Result::BUFFER_SIZE_TOO_SMALL;
            case HEIF::ErrorCode::BYTESTREAM_READ_ERROR:           return Result::BYTESTREAM_READ_ERROR;
            case HEIF::ErrorCode::MEMORY_TOO_SMALL_BUFFER:         return Result::MEMORY_TOO_SMALL_BUFFER;

            // State
            case HEIF::ErrorCode::UNINITIALIZED:                   return Result::UNINITIALIZED;
            case HEIF::ErrorCode::NOT_APPLICABLE:                  return Result::NOT_APPLICABLE;
            case HEIF::ErrorCode::FTYP_ALREADY_WRITTEN:            return Result::FTYP_ALREADY_WRITTEN;
            case HEIF::ErrorCode::ALLOCATOR_ALREADY_SET:           return Result::ALLOCATOR_ALREADY_SET;

            // Bad parameters / IDs
            case HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER:      return Result::INVALID_FUNCTION_PARAMETER;
            case HEIF::ErrorCode::INVALID_ITEM_ID:                 return Result::INVALID_ITEM_ID;
            case HEIF::ErrorCode::INVALID_GROUP_ID:                return Result::INVALID_GROUP_ID;
            case HEIF::ErrorCode::INVALID_MEDIADATA_ID:            return Result::INVALID_MEDIADATA_ID;
            case HEIF::ErrorCode::INVALID_METADATAITEM_ID:         return Result::INVALID_METADATAITEM_ID;
            case HEIF::ErrorCode::INVALID_PROPERTY_INDEX:          return Result::INVALID_PROPERTY_INDEX;
            case HEIF::ErrorCode::INVALID_REFERENCE_COUNT:         return Result::INVALID_REFERENCE_COUNT;
            case HEIF::ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX:return Result::INVALID_SAMPLE_DESCRIPTION_INDEX;
            case HEIF::ErrorCode::INVALID_SEGMENT:                 return Result::INVALID_SEGMENT;
            case HEIF::ErrorCode::INVALID_SEQUENCE_ID:             return Result::INVALID_SEQUENCE_ID;
            case HEIF::ErrorCode::INVALID_DECODER_CONFIG_ID:       return Result::INVALID_DECODER_CONFIG_ID;
            case HEIF::ErrorCode::INVALID_SEQUENCE_IMAGE_ID:       return Result::INVALID_SEQUENCE_IMAGE_ID;

            // Content
            case HEIF::ErrorCode::MEDIA_PARSING_ERROR:             return Result::MEDIA_PARSING_ERROR;
            case HEIF::ErrorCode::INVALID_MEDIA_FORMAT:            return Result::INVALID_MEDIA_FORMAT;
            case HEIF::ErrorCode::UNSUPPORTED_CODE_TYPE:           return Result::UNSUPPORTED_CODE_TYPE;
            case HEIF::ErrorCode::DECODER_CONFIGURATION_ERROR:     return Result::DECODER_CONFIGURATION_ERROR;
            case HEIF::ErrorCode::NO_MEDIA:                        return Result::NO_MEDIA;
            case HEIF::ErrorCode::CONTENT_TYPE_NOT_SET:            return Result::CONTENT_TYPE_NOT_SET;
            case HEIF::ErrorCode::PROTECTED_ITEM:                  return Result::PROTECTED_ITEM;
            case HEIF::ErrorCode::UNPROTECTED_ITEM:                return Result::UNPROTECTED_ITEM;

            // Mini / extension
            case HEIF::ErrorCode::TMI:                             return Result::TOO_MANY_ITEMS;
            case HEIF::ErrorCode::MISSING_PROPERTIES:              return Result::MISSING_PROPERTIES;
            case HEIF::ErrorCode::MULTIPLE_GAINMAPS:               return Result::MULTIPLE_GAINMAPS;
            case HEIF::ErrorCode::INVALID_EXIF_ORIENTATION_VALUE:  return Result::INVALID_EXIF_ORIENTATION_VALUE;
            case HEIF::ErrorCode::VVC_DISABLED:                    return Result::VVC_DISABLED;

            // Truly unknown / future codes fall through to ERROR_UNDEFINED.
            case HEIF::ErrorCode::UNKNOWN_ERROR:
            default:                                               return Result::ERROR_UNDEFINED;
        }
    }
}  // namespace HEIFPP
