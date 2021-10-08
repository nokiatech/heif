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

#ifndef MODIFICATIONTIMEINFORMATION_HPP
#define MODIFICATIONTIMEINFORMATION_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The Modification time property 'mdft' as defined in the HEIF standard. */
class ModificationTimeProperty : public FullBox
{
public:
    ModificationTimeProperty();
    ~ModificationTimeProperty() override = default;

    /** @brief Sets modification time.
     *  @param [in] modificationTime Modification time, in microseconds since midnight, January 1st, 1904, in UTC time.
     */
    void setModificationTime(std::uint64_t modificationTime);

    /** @brief Get modification time.
     *  @returns Modification time, in microseconds since midnight, January 1st, 1904, in UTC time. */
    std::uint64_t getModificationTime() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    std::uint64_t mModificationTime;
};

#endif
