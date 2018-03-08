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

#ifndef SAMPLEGROUPENTRY_HPP
#define SAMPLEGROUPENTRY_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"

/** @brief SampleGroupDescriptionEntry class.
 *  @details Provides abstract methods to generate a sample group description entry.
 *  @details Other classes can inherit from SampleGroupDescriptionEntry to implement different sample groupings */
class SampleGroupDescriptionEntry
{
public:
    /**
     * @param type Grouping type which identifies the associated SampleToGroupBox.
     */
    SampleGroupDescriptionEntry(const FourCCInt& type);
    virtual ~SampleGroupDescriptionEntry() = default;

    /** @brief Get the type of the Sample Group Entry.
     *  @returns FourCCInt type of the sample group entry */
    FourCCInt getGroupingType() const;

    /** @brief Get the size of the Sample Group Entry.
     *  @details Abstract class to be implemented by the extending class.
     *  @returns Serialized byte size of the sample group entry */
    virtual std::uint32_t getSize() const = 0;

    /** @brief Serialize the sample group entry data structure.
     *  @details Abstract class to be implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized sample group entry data structure */
    virtual void writeEntry(ISOBMFF::BitStream& bitstr) = 0;

    /** @brief Parse a serialized sample group entry data structure.
     *  @details Abstract class to be implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized sample group entry data structure */
    virtual void parseEntry(ISOBMFF::BitStream& bitstr) = 0;

private:
    FourCCInt mGroupingType;  ///< Type used for class pointer type safety when casting from baseclass.
};

#endif /* end of include guard: SAMPLEGROUPENTRY_HPP */
