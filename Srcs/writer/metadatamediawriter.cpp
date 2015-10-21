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

#include "metadatamediawriter.hpp"
#include "writerconstants.hpp"
#include <fstream>
#include <stdexcept>

MetadataMediaWriter::MetadataMediaWriter(const std::string& fileName, const std::string& handlerType) :
    mFilePath(fileName),
    mHandlerType(handlerType)
{
}

MediaDataBox MetadataMediaWriter::writeMedia()
{
    std::ifstream file;
    file.open(mFilePath, std::ios::in | std::ios::binary);
    if (!file.good())
    {
        throw std::runtime_error("Unable to open exif file '" + mFilePath + "'");
    }

    file.seekg(0, std::ios::end);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Exif metadata is prefixed by exif_tiff_header_offset
    std::streamsize tiffHeaderOffsetLength = 0;
    if (mHandlerType == EXIF_HANDLER)
    {
        /// @todo Add support for exif_tiff_header_offset?
        tiffHeaderOffsetLength = 4; // size of ExifDataBlock.exif_tiff_header_offset
    }
    std::vector<uint8_t> buffer(size + tiffHeaderOffsetLength);
    if (!file.read(reinterpret_cast<char*>(buffer.data() + tiffHeaderOffsetLength), size))
    {
        file.close();
        throw std::runtime_error("Error while reading exif file '" + mFilePath + "'");
    }
    file.close();

    MediaDataBox mdat;
    mdat.addData(buffer);

    return mdat;
}
