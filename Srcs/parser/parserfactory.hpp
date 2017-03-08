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

#ifndef PARSER_FACTORY_HPP
#define PARSER_FACTORY_HPP

#include "parserinterface.hpp"
#include "avcparser.hpp"
#include "h265parser.hpp"
#include "mediatypedefs.hpp"

#include <memory>
#include <string>

/** Factory to instantiate the correct type of media parser */
namespace ParserFactory
{

inline std::unique_ptr<ParserInterface> getParser(MediaType mediaType)
{
    std::unique_ptr<ParserInterface> mediaParser;

    // Create parser by code type
    switch (mediaType)
    {
        case MediaType::AVC:
        {
            // Use AvcParser
            mediaParser.reset(new AvcParser);
            break;
        }
        case MediaType::HEVC:
        case MediaType::LHEVC:
        {
            // Use H265Parser
            mediaParser.reset(new H265Parser);
            break;
        }
        default:
        {
            // Invalid media type
            throw std::runtime_error("Failed to create bitstream parser for unsupported media type");
        }
    }

    return mediaParser;
}

} // namespace

#endif
