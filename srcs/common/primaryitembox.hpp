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

#ifndef PRIMARYITEMBOX_HPP
#define PRIMARYITEMBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Primary Item Box class
 * @details 'pitm' box implementation as specified in the ISOBMFF specification.
 */
class PrimaryItemBox : public FullBox
{
public:
    PrimaryItemBox();
    virtual ~PrimaryItemBox() = default;

    void setItemId(std::uint32_t itemId);
    std::uint32_t getItemId() const;

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
    std::uint32_t mItemId;  ///< The identifier of the primary item
};

#endif /* end of include guard: PRIMARYITEMBOX_HPP */
