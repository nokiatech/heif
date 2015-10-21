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

#ifndef IMAGEMEDIAWRITER_HPP
#define IMAGEMEDIAWRITER_HPP

#include "mediawriter.hpp"
#include <string>

class MediaDataBox;

/**
 * @brief MediaWriter for creating a MediaDataBox which contains image data.
 * @details This class is used for writing image bitstream data to MediaDataBoxes by both TrackBox and MetaBox writers.
 * The same class is used both for master and thumbnail image writing.
 * @todo Refactoring could be considered, as now same bitstream is parsed both in MediaWriter and a corresponding
 * Track or MetaWriter, as they need to extract information in any case.
 */
class ImageMediaWriter : public MediaWriter
{
public:
    /** @param fileName Name of the bitstream file */
    ImageMediaWriter(const std::string& fileName);
    virtual ~ImageMediaWriter() = default;

    /**
     * @brief Create a MediaDataBox containing image bitstream data.
     * @throws Runtime Error if it is not possible to open input bitstream file.
     * @see MediaWriter::writeMedia()
     */
    MediaDataBox writeMedia();

private:
    std::string mFilePath; ///< Filename of the input
};

#endif
