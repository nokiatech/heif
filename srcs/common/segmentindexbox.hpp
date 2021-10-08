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

#ifndef SEGMENTINDEXBOX_HPP
#define SEGMENTINDEXBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief  Segment Index Box class
 * @details 'sidx' box implementation as specified in the ISOBMFF specification.
 */
class SegmentIndexBox : public FullBox
{
public:
    SegmentIndexBox(uint8_t version = 0);
    ~SegmentIndexBox() override = default;

    struct Reference
    {
        bool referenceType;
        uint32_t referencedSize;  // to store 31 bit value
        uint32_t subsegmentDuration;
        bool startsWithSAP;
        uint8_t sapType;        // to store 3 bit value
        uint32_t sapDeltaTime;  // to store 28 bit value
    };

    /** @brief When writing box, put a free-box to the end so that this box could later
     * be expanded to contain up to reserveTotal references
     * @param [in] reserveTotal The amount of space to allocate in total. */
    void setSpaceReserve(size_t reserveTotal);

    /** @brief Set Reference ID of the SegmentIndexBox.
     *  @param [in] uint32_t referenceID **/
    void setReferenceId(const uint32_t referenceID);

    /** @brief Get Reference ID of the SegmentIndexBox.
     *  @return uint32_t Reference ID as specified in 8.16.3.1 of ISO/IEC 14496-12:2015(E)**/
    uint32_t getReferenceId() const;

    /** @brief Set Timescale of the SegmentIndexBox.
     *  @param [in] uint32_t timescale**/
    void setTimescale(const uint32_t timescale);

    /** @brief Get Timescale of the SegmentIndexBox.
     *  @return uint32_t Timescale as specified in 8.16.3.1 of ISO/IEC 14496-12:2015(E)**/
    uint32_t getTimescale() const;

    /** @brief Set Earliest Presentation Time of the SegmentIndexBox.
     *  @param [in] uint64_t earliestPresentationTime **/
    void setEarliestPresentationTime(const uint64_t earliestPresentationTime);

    /** @brief Get Earliest Presentation Time of the SegmentIndexBox.
     *  @return uint64_t Earliest Presentation Time as specified in 8.16.3.1 of ISO/IEC 14496-12:2015(E)**/
    uint64_t getEarliestPresentationTime() const;

    /** @brief Set First Offset of the SegmentIndexBox.
     *  @param [in] uint64_t firstOffset **/
    void setFirstOffset(const uint64_t firstOffset);

    /** @brief Get First Offset of the SegmentIndexBox.
     *  @return uint64_t First Offset as specified in 8.16.3.1 of ISO/IEC 14496-12:2015(E)**/
    uint64_t getFirstOffset() const;

    /** @brief Add Reference to the SegmentIndexBox.
     *  @param [in] SegmentIndexBox::Reference reference**/
    void addReference(const Reference& reference);

    /** @brief Get References of the SegmentIndexBox.
     *  @return Vector<SegmentIndexBox::Reference> References as specified in 8.16.3.1 of ISO/IEC 14496-12:2015(E)**/
    Vector<Reference> getReferences() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    uint32_t mReferenceID;
    uint32_t mTimescale;
    uint64_t mEarliestPresentationTime;
    uint64_t mFirstOffset;
    Vector<Reference> mReferences;  // contains ReferenceCount amount of References.
    size_t mReserveTotal;
};

#endif /* end of include guard: SEGMENTINDEXBOX_HPP */
