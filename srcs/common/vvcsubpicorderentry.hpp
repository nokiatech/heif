/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef VVCSUBPICORDERENTRY_HPP
#define VVCSUBPICORDERENTRY_HPP

#include <vector>

#include "samplegroupdescriptionentry.hpp"
#include <cstdint>

/** VvcSubpicOrderEntry class. Inherits from abstrct VisualSampleGroupEntry (SampleGroupDescriptionEntry). */
class VvcSubpicOrderEntry : public SampleGroupDescriptionEntry
{
public:
    VvcSubpicOrderEntry();
    ~VvcSubpicOrderEntry() override = default;

    bool getSubpicIdInfoFlag() const;
    void setSubpicIdInfoFlag(bool subpicIdInfoFlag);

    std::vector<std::uint16_t> getSubpicRefIdx() const;
    void setSubpicRefIdx(const std::vector<std::uint16_t>& subpicRefIdx);

    std::uint16_t getSubpicIdLenMinus1() const;
    void setSubpicIdLenMinus1(std::uint16_t subpicIdLenMinus1);

    std::uint16_t getSubpicIdBitPos() const;
    void setSubpicIdBitPos(std::uint16_t subpicIdBitPos);

    bool getStartCodeEmulFlag() const;
    void setStartCodeEmulFlag(bool startCodeEmulFlag);

    bool getPpsSpsSubpicIdFlag() const;
    void setPpsSpsSubpicIdFlag(bool ppsSpsSubpicIdFlag);

    std::uint8_t getPpsId() const;
    void setPpsId(std::uint8_t ppsId);

    std::uint8_t getSpsId() const;
    void setSpsId(std::uint8_t spsId);

    std::uint32_t getSize() const override;

    /** @brief Serialize the VvcSubpicOrderEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized VvcSubpicOrderEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized VvcSubpicOrderEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized VvcSubpicOrderEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

private:
    bool mSubpicIdInfoFlag{false};
    std::vector<std::uint16_t> mSubpicRefIdx;
    std::uint16_t mSubpicIdLenMinus1{0};
    std::uint16_t mSubpicIdBitPos{0};
    bool mStartCodeEmulFlag{false};
    bool mPpsSpsSubpicIdFlag{false};
    std::uint8_t mPpsId{0};
    std::uint8_t mSpsId{0};
};

#endif /* end of include guard: VVCSUBPICORDERENTRY_HPP */
