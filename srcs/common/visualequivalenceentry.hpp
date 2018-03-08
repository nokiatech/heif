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

#ifndef VISUALEQUIVALENCEENTRY_HPP
#define VISUALEQUIVALENCEENTRY_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief VisualEquivalenceEntry class. Inherits from SampleGroupDescriptionEntry.
*  @details Implements VisualEquivalenceEntry as defined in HEIF standard. */
class VisualEquivalenceEntry : public SampleGroupDescriptionEntry
{
public:
    VisualEquivalenceEntry();
    virtual ~VisualEquivalenceEntry() = default;

    /** @brief Set the difference of the particular time related to the image item(s) 
    *  and the composition time of the sample associated with the sample group description entry. 
    *  @param [in] timeOffset Time offset */
    void setTimeOffset(std::int16_t timeOffset);

    /** @brief Get the difference of the particular time related to the image item(s) 
    *  and the composition time of the sample associated with the sample group description entry. 
    *  @returns Time offset */
    std::int16_t getTimeOffset() const;

    /** @brief Set timescale multiplier that specifies the timescale, as a multiplier to the media timescale of the track.
    *  @param [in] multiplier Timescale multiplier  */
    void setTimescaleMultiplier(uint16_t multiplier);

    /** @brief Get  timescale multiplier that specifies the timescale, as a multiplier to the media timescale of the track.
    *  @return std::uint16_t Timescale multiplier*/
    std::uint16_t getTimescaleMultiplier() const;

    /** @see SampleGroupEntry::getSize() */
    /** @brief Get the serialized byte size of VisualEquivalenceEntry.
    *  @return Byte size of the Entry*/
    virtual std::uint32_t getSize() const;

    /** @brief Serialize the VisualEquivalenceEntry data structure.
    *  @details Implemented by the extending class.
    *  @param [out] bitstr Bitstream containing the serialized VisualEquivalenceEntry data structure */
    virtual void writeEntry(ISOBMFF::BitStream& bitstr);

    /** @brief Parse a serialized VisualEquivalenceEntry data structure.
    *  @details Implemented by the extending class.
    *  @param [in] bitstr Bitstream containing the serialized VisualEquivalenceEntry data structure */
    virtual void parseEntry(ISOBMFF::BitStream& bitstr);

private:
    std::int16_t mTimeOffset;            ///< Difference of the time related to the image item(s) and the composition time of sample(s).
    std::uint16_t mTimescaleMultiplier;  ///< Timescale as a multiplier to the media timescale of the track.
};

#endif /* end of include guard: VISUALEQUIVALENCEENTRY_HPP */
