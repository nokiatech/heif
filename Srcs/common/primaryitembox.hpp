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

#ifndef PRIMARYITEMBOX_HPP
#define PRIMARYITEMBOX_HPP

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
     * @brief Serialize box data to the BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(BitStream& bitstr);

    /**
     * @brief Deserialize box data from the BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint32_t mItemId; ///< The identifier of the primary item
};

#endif /* end of include guard: PRIMARYITEMBOX_HPP */
