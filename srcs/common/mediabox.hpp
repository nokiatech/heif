/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef MEDIABOX_HPP
#define MEDIABOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "handlerbox.hpp"
#include "mediaheaderbox.hpp"
#include "mediainformationbox.hpp"

/** @brief Media Box class. Extends from Box.
 *  @details 'mdia' box contains media related information as defined in the ISOBMFF standard. */
class MediaBox : public Box
{
public:
    MediaBox();
    virtual ~MediaBox() = default;

    /** @brief Get Media Header Box.
     *  @returns Reference to the Media Header Box. */
    MediaHeaderBox& getMediaHeaderBox();

    /** @brief Get Media Header Box.
     *  @returns Reference to the Media Header Box. */
    const MediaHeaderBox& getMediaHeaderBox() const;

    /** @brief Get Media Handler Box.
     *  @returns Reference to the Media Handler Box. */
    HandlerBox& getHandlerBox();

    /** @brief Get Media Handler Box.
     *  @returns Reference to the Media Handler Box. */
    const HandlerBox& getHandlerBox() const;

    /** @brief Get Media Information Box.
     *  @returns Reference to the Media Information Box. */
    MediaInformationBox& getMediaInformationBox();

    /** @brief Get Media Information Box.
     *  @returns Reference to the Media Information Box. */
    const MediaInformationBox& getMediaInformationBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a MediaBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    MediaHeaderBox mMediaHeaderBox;            ///< Media Header Box
    HandlerBox mHandlerBox;                    ///< Media Handler box
    MediaInformationBox mMediaInformationBox;  ///< Media Information Box
};

#endif /* end of include guard: MEDIABOX_HPP */
