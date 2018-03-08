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

#ifndef SOUNDMEDIAHEADERBOX_HPP
#define SOUNDMEDIAHEADERBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Sound Media Header Box class
 * @details 'smhd' box implementation as specified in the ISOBMFF specification.
 */
class SoundMediaHeaderBox : public FullBox
{
public:
    SoundMediaHeaderBox();
    virtual ~SoundMediaHeaderBox() = default;

    /** @brief Sets balance as defined in ISOBMFF
     *  @param [in] groupID track's alternate group id */
    void setBalance(std::uint16_t balance);

    /** @brief Get balance as defined in ISOBMFF
     *  @returns track's alternate group id */
    std::uint16_t getBalance() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint16_t mBalance;
};

#endif /* end of include guard: SOUNDMEDIAHEADERBOX_HPP */
