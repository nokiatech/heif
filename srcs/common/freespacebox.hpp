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

#ifndef FREESPACEBOX_HPP
#define FREESPACEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

class FreeSpaceBox : public Box
{
public:
    FreeSpaceBox();
    virtual ~FreeSpaceBox() = default;

    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;
    virtual void parseBox(ISOBMFF::BitStream& bitstr);
};

#endif
