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

#ifndef VVCNALUONFIGBOX_HPP
#define VVCNALUONFIGBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

/**
 * VVC NAL unit configuration box 'vvnC'
 */
class VvcNaluConfigBox : public FullBox
{
public:
    VvcNaluConfigBox();
    ~VvcNaluConfigBox() override = default;

    void writeBox(ISOBMFF::BitStream& bitstr) const override;
    void parseBox(ISOBMFF::BitStream& bitstr) override;

    std::uint8_t getLengthSizeMinusOne() const;
    void setLengthSizeMinusOne(std::uint8_t lengthSizeMinusOne);

private:
    std::uint8_t mLengthSizeMinusOne;
};

#endif
