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

#ifndef VVCSUBPICIDENTRY_HPP
#define VVCSUBPICIDENTRY_HPP

#include <vector>

#include "bitstream.hpp"
#include "samplegroupdescriptionentry.hpp"

/** VvcSubpicIdEntry class. Inherits from abstrct VisualSampleGroupEntry (SampleGroupDescriptionEntry). */
class VvcSubpicIdEntry : public SampleGroupDescriptionEntry
{
public:
    VvcSubpicIdEntry();
    ~VvcSubpicIdEntry() override = default;

    /** @brief Get the size of the Sample Group Entry.
     *  @returns Serialized byte size of the sample group entry */
    std::uint32_t getSize() const override;

    /** @brief Serialize the VvcSubpicIdEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized VvcSubpicIdEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized VvcSubpicIdEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized VvcSubpicIdEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

    /**
     * @return subpic_ids of the entry.
     */
    std::vector<std::uint16_t> getSubPicIds() const;

    /**
     * @param subPicIds New subpic_ids of the entry. Possible previous entries are discarded.
     */
    void setSubPicIds(const std::vector<std::uint16_t>& subPicIds);

    /**
     * @return groupIDs of the entry.
     */
    std::vector<std::uint16_t> getGroupIds() const;

    /**
     * @param groupIds New groupIDs of the entry. Possible previous entries are discarded.
     */
    void setGroupIds(const std::vector<std::uint16_t>& groupIds);

    /**
     * @return Get subpicture_id_continuous_flag value. When true, there should be only 1 subpic_id set.
     */
    bool getSubPicIdContinuousFlag() const;

    /**
     * @param subPicIdContinuousFlag Set subpicture_id_continuous_flag value. When true, there should be only 1
     * subpic_id set.
     */
    void setSubPicIdContinuousFlag(bool subPicIdContinuousFlag);

    /**
     * @return Get rect_region_flag value.
     */
    bool getRectRegionFlag() const;

    /**
     * @param rectRegioFlag Set rect_region_flag value.
     */
    void setRectRegionFlag(bool rectRegioFlag);

private:
    bool mRectRegionFlag;
    bool mSubPicIdContinuousFlag;
    std::vector<std::uint16_t> mSubPicIds;
    std::vector<std::uint16_t> mGroupdIds;
};

#endif /* end of include guard: VVCSUBPICIDENTRY_HPP */
