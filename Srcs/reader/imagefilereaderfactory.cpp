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

#include "imagefilereaderfactory.hpp"
#include "hevcimagefilereader.hpp"

namespace ImageFileReaderFactory
{

std::unique_ptr<ImageFileReaderInterface> getInstance(const std::string& fileName)
{
    std::unique_ptr<ImageFileReaderInterface> hevcImageFilereader(new HevcImageFileReader);
    hevcImageFilereader->initialize(fileName);
    return hevcImageFilereader;
}

std::unique_ptr<ImageFileReaderInterface> getInstance(std::istream& stream)
{
    std::unique_ptr<ImageFileReaderInterface> hevcImageFilereader(new HevcImageFileReader);
    hevcImageFilereader->initialize(stream);
    return hevcImageFilereader;
}

}
