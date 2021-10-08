/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef MOVIEFRAGMENTHEADERBOX_HPP
#define MOVIEFRAGMENTHEADERBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Movie Fragment Header Box class
 * @details 'mfhd' box implementation as specified in the ISOBMFF specification.
 */
class MovieFragmentHeaderBox : public FullBox
{
public:
    MovieFragmentHeaderBox();
    ~MovieFragmentHeaderBox() override = default;

    /** @brief Set Sequence Number of the MovieFragmentHeaderBox.
     *  @param [in] uint32_t sequenceNumber */
    void setSequenceNumber(const uint32_t sequenceNumber);

    /** @brief Get Sequence Number of the MovieFragmentHeaderBox.
     *  @return uint32_t as specified in 8.8.5.1 of ISO/IEC 14496-12:2015(E) */
    uint32_t getSequenceNumber() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    uint32_t mSequenceNumber;
};

#endif /* end of include guard: MOVIEFRAGMENTHEADERBOX_HPP */
