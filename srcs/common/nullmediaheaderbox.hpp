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

#ifndef NULLMEDIAHEADERBOX_HPP
#define NULLMEDIAHEADERBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Null Media Header Box class
 * @details 'nmhd' box implementation as specified in the ISOBMFF specification.
 */
class NullMediaHeaderBox : public FullBox
{
public:
    NullMediaHeaderBox();
    virtual ~NullMediaHeaderBox() = default;

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
};

#endif /* end of include guard: NULLMEDIAHEADERBOX_HPP */
