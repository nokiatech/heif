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

#ifndef CREATIONTIMEINFORMATION_HPP
#define CREATIONTIMEINFORMATION_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The Creation time property 'crtt', as defined in the HEIF standard. */
class CreationTimeProperty : public FullBox
{
public:
    CreationTimeProperty();
    ~CreationTimeProperty() override = default;

    /** @brief Sets creation time.
     *  @param [in] creationTime Creation time, in microseconds since midnight, January 1st, 1904, in UTC time. */
    void setCreationTime(std::uint64_t creationTime);

    /** @brief Get creation time.
     *  @returns Creation time, in microseconds since midnight, January 1st, 1904, in UTC time. */
    std::uint64_t getCreationTime() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    std::uint64_t mCreationTime;  ///< creation_time
};

#endif
