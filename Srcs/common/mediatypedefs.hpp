/* Copyright (c) 2017, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#ifndef MEDIATYPEDEFS_HPP
#define MEDIATYPEDEFS_HPP

#include <map>
#include <stdexcept>
#include <string>

/** @brief Supported bitstream media types */
enum class MediaType
{
    AVC,
    HEVC,
    LHEVC,
    INVALID
};

/** Helpers for media type - code type - bitstream type conversions */
namespace MediaTypeTool
{
    const std::map<std::string, MediaType> mediaTypeNames =
    {
        { "avc1", MediaType::AVC },
        { "hvc1", MediaType::HEVC },
        { "lhv1", MediaType::LHEVC }
    };

    /** Get MediaType for code type string */
    inline MediaType getMediaTypeByCodeType(const std::string codeType,
                                            const std::string fileNameForErrorMsg)
    {
        const auto type = mediaTypeNames.find(codeType);
        if (type != mediaTypeNames.cend())
        {
            return type->second;
        }

        // Unsupported code type
        std::string fileInfo = (fileNameForErrorMsg.empty()) ? "" : " (" + fileNameForErrorMsg + ")";
        if (codeType.empty())
        {
            throw std::runtime_error("Failed to define media type, code_type not set" + fileInfo);
        }
        else
        {
            throw std::runtime_error("Failed to define media type for unsupported code_type '" +
                                     codeType + "'" + fileInfo);
        }
    }

    /** Get "friendly" bitstream type name for MediaType, used only for logging (can be changed!) */
    inline const std::string getBitStreamTypeName(MediaType mediaType)
    {
        for (auto iter = mediaTypeNames.cbegin(); iter != mediaTypeNames.cend(); ++iter)
        {
            if (iter->second == mediaType)
            {
                return iter->first;
            }
        }

        return "INVALID";
    }
}

#endif
