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

#ifndef IMAGEFILEREADERFACTORY_HPP
#define IMAGEFILEREADERFACTORY_HPP

#include <string>
#include <memory>

class ImageFileReaderInterface;

namespace ImageFileReaderFactory
{
/** @param [in] fileName Reference to a string which is the full path to the image file.
 *  @return A pointer to a derived instance of ImageFileReaderInterface */
std::unique_ptr<ImageFileReaderInterface> getInstance(const std::string& fileName);

/** @param [in] stream Image file stream.
 *  @return A pointer to a derived instance of ImageFileReaderInterface */
std::unique_ptr<ImageFileReaderInterface> getInstance(std::istream& stream);
};

#endif
