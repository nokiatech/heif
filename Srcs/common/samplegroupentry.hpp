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

#ifndef SAMPLEGROUPENTRY_HPP
#define SAMPLEGROUPENTRY_HPP

#include "bitstream.hpp"
#include <vector>

/** @brief SampleGroupEntry class.
 *  @details Provides abstract methods to generate a sample group entry.
 *  @details Other classes can inherit from SampleGroupEntry to implement different sample groupings **/
class SampleGroupEntry
{
public:
    SampleGroupEntry() = default;
    virtual ~SampleGroupEntry() = default;

    /** @brief Get the size of the Sample Group Entry.
     *  @details Abstract class to be implemented by the extending class.
     *  @returns Serialized byte size of the sample group entry **/
    virtual std::uint32_t getSize() const = 0;

    /** @brief Serialize the sample group entry data structure.
     *  @details Abstract class to be implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized sample group entry data structure **/
    virtual void writeEntry(BitStream& bitstr) = 0;

    /** @brief Parse a serialized sample group entry data structure.
     *  @details Abstract class to be implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized sample group entry data structure **/
    virtual void parseEntry(BitStream& bitstr) = 0;
};


/** @brief DirectReferenceSampleListEntry class. Inherits from SampleGroupEntry.
 *  @details Implements DirectReferenceSampleListEntry as defined in HEIF and ISOBMFF standards.**/
class DirectReferenceSampleListEntry : public SampleGroupEntry
{
public:
    DirectReferenceSampleListEntry();
    virtual ~DirectReferenceSampleListEntry() = default;

    /** @brief Set the sample identifier computed for a reference or a non reference sample.
     *  @param [in] sampleId Sample ID **/
    void setSampleId(std::uint32_t sampleId);

    /** @brief Get the sample identifier computed for a reference or a non reference sample.
     *  @returns Sample ID **/
    std::uint32_t getSampleId() const;

    /** @brief Set identifiers for referenced samples.
     *  @param [in] refSampleId Referenced sample Ids **/
    void setDirectReferenceSampleIds(const std::vector<std::uint32_t>& refSampleId);

    /** @brief Get identifiers for referenced samples.
     *  @return Identifiers for referenced samples.**/
    std::vector<std::uint32_t> getDirectReferenceSampleIds() const;

    /** @see SampleGroupEntry::getSize() */

    /** @brief Get the serialized byte size of DirectReferenceSampleListEntry.
     *  @return Byte size of the Entry**/
    virtual std::uint32_t getSize() const;

    /** @brief Serialize the DirectReferenceSampleListEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized DirectReferenceSampleListEntry data structure **/
    virtual void writeEntry(BitStream& bitstr);

    /** @brief Parse a serialized DirectReferenceSampleListEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized DirectReferenceSampleListEntry data structure **/
    virtual void parseEntry(BitStream& bitstr);

private:
    std::uint32_t mSampleId; ///< Sample Id whose referenced sample Id will be listed
    std::vector<std::uint32_t> mDirectReferenceSampleIds; ///< Vector of direct reference sample Ids
};

#endif /* end of include guard: SAMPLEGROUPENTRY_HPP */

