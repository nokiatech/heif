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

#ifndef FREESPACEBOX_HPP
#define FREESPACEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

class FreeSpaceBox : public Box
{
public:
    FreeSpaceBox();
    ~FreeSpaceBox() override = default;

    /**
     * @brief setSize Set FreeSpaceBox size.
     * @param size FreeSpaceBox size in bytes. Must be 8 or more because of the box header size.
     * @return True if size was set successfully (size >= 8), false otherwise.
     */
    bool setSize(std::uint32_t size);

    void writeBox(ISOBMFF::BitStream& bitstr) const override;
    void parseBox(ISOBMFF::BitStream& bitstr) override;
};

#endif
