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

#ifndef RECTANGULARREGIONGROUPENTRY_HPP
#define RECTANGULARREGIONGROUPENTRY_HPP

#include <vector>

#include "bitstream.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief RectangularRegionGroupEntry class. Inherits from abstrct VisualSampleGroupEntry
 * (SampleGroupDescriptionEntry).
 *  @details Implements RectangularRegionGroupEntry as defined in the Carriage of VVC and EVC in ISOBMFF standard. */
class RectangularRegionGroupEntry : public SampleGroupDescriptionEntry
{
public:
    RectangularRegionGroupEntry();
    ~RectangularRegionGroupEntry() override = default;

    std::uint32_t getSize() const override;

    /** @brief Serialize the RectangularRegionGroupEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized RectangularRegionGroupEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized RectangularRegionGroupEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized RectangularRegionGroupEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

    std::uint16_t getGroupId() const;
    void setGroupId(std::uint16_t groupId);

    bool getRectRegionFlag() const;
    void setRectRegionFlag(bool rectRegionFlag);

    std::uint8_t getIndependentIdc() const;
    void setIndependentIdc(std::uint8_t independentIdc);

    bool getFullPicture() const;
    void setFullPicture(bool fullPicture);

    bool getFilteringDisabled() const;
    void setFilteringDisabled(bool filteringDisabled);

    bool getHasDependencyList() const;
    void setHasDependencyList(bool hasDependencyList);

    std::uint16_t getHorizontalOffset() const;
    void setHorizontalOffset(std::uint16_t horizontalOffset);

    std::uint16_t getVerticalOffset() const;
    void setVerticalOffset(std::uint16_t verticalOffset);

    std::uint16_t getRegionWidth() const;
    void setRegionWidth(std::uint16_t regionWidth);

    std::uint16_t getRegionHeight() const;
    void setRegionHeight(std::uint16_t regionHeight);

    std::vector<std::uint16_t> getDependencyRectRegionGroupIds() const;
    void setDependencyRectRegionGroupIds(const std::vector<std::uint16_t>& dependencyRectRegionGroupIds);

private:
    std::uint16_t mGroupId;
    bool mRectRegionFlag;

    std::uint8_t mIndependentIdc;
    bool mFullPicture;
    bool mFilteringDisabled;
    bool mHasDependencyList;

    std::uint16_t mHorizontalOffset;
    std::uint16_t mVerticalOffset;

    std::uint16_t mRegionWidth;
    std::uint16_t mRegionHeight;

    std::vector<std::uint16_t> mDependencyRectRegionGroupIds;
};

#endif /* end of include guard: RECTANGULARREGIONGROUPENTRY_HPP */
