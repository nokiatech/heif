/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#ifndef METADATAMEDIAWRITER_HPP
#define METADATAMEDIAWRITER_HPP

#include "mediadatabox.hpp"
#include "mediawriter.hpp"
#include <string>

/**
 * @brief MediaWriter for creating a MediaDataBox which contains Exif or XML metadata.
 */
class MetadataMediaWriter: public MediaWriter
{
public:
    /**
     * Constructor of MetadataMediaWriter
     * @param fileName    Name of the metadata (Exif, XML) file
     * @param handlerType Type of the metadata handler ("exif" for Exif)
     */
    MetadataMediaWriter(const std::string& fileName, const std::string& handlerType);
    virtual ~MetadataMediaWriter() = default;

    /**
     * @brief Create a MediaDataBox containing metadata.
     * @throws Runtime Error if it is not possible to open input file.
     * @see MediaWriter::writeMedia()
     */
    virtual MediaDataBox writeMedia();

private:
    std::string mFilePath;    ///< Filename of the input
    std::string mHandlerType; ///< The handler type
};

#endif
