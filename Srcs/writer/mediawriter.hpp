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

#ifndef MEDIAWRITER_HPP
#define MEDIAWRITER_HPP

#include "mediadatabox.hpp"

/**
 * @brief Abstract base class for media data writers
 * @details MediaWriter based classes handle writing image data, while a corresponding TrackWriter or MetaWriter
 * class handles writing related track or metadata information. */
class MediaWriter
{
public:
    MediaWriter() = default;
    virtual ~MediaWriter() = default;

    /**
     * Create a MediaDataBox containing the media data, based on the input configuration of the derived class.
     * @return A filled MediaDataBox.
     */
    virtual MediaDataBox writeMedia() = 0;
};

#endif /* end of include guard: MEDIAWRITER_HPP */
