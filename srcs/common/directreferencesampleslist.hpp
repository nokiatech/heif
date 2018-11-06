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

#ifndef DIRECTREFEREBCESAMPLESLISTENTRY_HPP
#define DIRECTREFEREBCESAMPLESLISTENTRY_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief DirectReferenceSamplesList class. Inherits from SampleGroupDescriptionEntry.
 *  @details Implements DirectReferenceSamplesList as defined in HEIF standard. */
class DirectReferenceSamplesList : public SampleGroupDescriptionEntry
{
public:
    DirectReferenceSamplesList();
    virtual ~DirectReferenceSamplesList() = default;

    /** @brief Set the sample identifier computed for a reference or a non reference sample.
     *  @param [in] sampleId Sample ID */
    void setSampleId(std::uint32_t sampleId);

    /** @brief Get the sample identifier computed for a reference or a non reference sample.
     *  @returns Sample ID */
    std::uint32_t getSampleId() const;

    /** @brief Set identifiers for referenced samples.
     *  @param [in] refSampleId Referenced sample Ids */
    void setDirectReferenceSampleIds(const Vector<std::uint32_t>& refSampleId);

    /** @brief Get identifiers for referenced samples.
     *  @return Identifiers for referenced samples. */
    const Vector<std::uint32_t>& getDirectReferenceSampleIds() const;

    /** @see SampleGroupEntry::getSize() */

    /** @brief Get the serialized byte size of DirectReferenceSamplesList.
     *  @return Byte size of the Entry */
    virtual std::uint32_t getSize() const;

    /** @brief Serialize the DirectReferenceSamplesList data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized DirectReferenceSamplesList data structure */
    virtual void writeEntry(ISOBMFF::BitStream& bitstr);

    /** @brief Parse a serialized DirectReferenceSamplesList data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized DirectReferenceSamplesList data structure */
    virtual void parseEntry(ISOBMFF::BitStream& bitstr);

private:
    std::uint32_t mSampleId;                          ///< Sample Id whose referenced sample Id will be listed
    Vector<std::uint32_t> mDirectReferenceSampleIds;  ///< Vector of direct reference sample Ids
};

#endif /* end of include guard: DIRECTREFEREBCESAMPLESLISTENTRY_HPP */
