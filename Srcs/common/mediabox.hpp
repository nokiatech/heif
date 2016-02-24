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

#ifndef MEDIABOX_HPP
#define MEDIABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "handlerbox.hpp"
#include "mediaheaderbox.hpp"
#include "mediainformationbox.hpp"

/** @brief Media Box class. Extends from Box.
 *  @details 'mdia' box contains media related information as defined in the ISOBMFF and HEIF standards. **/
class MediaBox : public Box
{
public:
    MediaBox();
    virtual ~MediaBox() = default;

    /** @brief Get Media Header Box.
     *  @returns Reference to the Media Header Box.**/
    MediaHeaderBox& getMediaHeaderBox();

    /** @brief Get Media Handler Box.
     *  @returns Reference to the Media Handler Box.**/
    HandlerBox& getHandlerBox();

    /** @brief Get Media Information Box.
     *  @returns Reference to the Media Information Box.**/
    MediaInformationBox& getMediaInformationBox();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a MediaBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    MediaHeaderBox mMediaHeaderBox; ///< Media Header Box
    HandlerBox mHandlerBox; ///< Media Handler box
    MediaInformationBox mMediaInformationBox; ///< Media Information Box
};

#endif /* end of include guard: MEDIABOX_HPP */
