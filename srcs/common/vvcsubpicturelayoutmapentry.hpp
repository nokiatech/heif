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

#ifndef VVCSUBPICTURELAYOUTMAPENTRY_HPP
#define VVCSUBPICTURELAYOUTMAPENTRY_HPP

#include <vector>

#include "bitstream.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief VvcSubpictureLayoutMapEntry class. Inherits from abstract VisualSampleGroupEntry
 * (SampleGroupDescriptionEntry).
 *  @details Implements VvcSubpictureLayoutMapEntry as defined in the Carriage of VVC and EVC in ISOBMFF standard. */
class VvcSubpictureLayoutMapEntry : public SampleGroupDescriptionEntry
{
public:
    VvcSubpictureLayoutMapEntry();
    ~VvcSubpictureLayoutMapEntry() override = default;

    /** @brief Get the size of the Sample Group Entry.
     *  @returns Serialized byte size of the sample group entry */
    std::uint32_t getSize() const override;

    /** @brief Serialize the VvcSubpictureLayoutMapEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized VvcSubpictureLayoutMapEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized VvcSubpictureLayoutMapEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized VvcSubpictureLayoutMapEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

    /**
     * @return groupIDs of the entry.
     */
    std::vector<std::uint16_t> getGroupIds() const;

    /**
     * @param groupIds New groupIDs of the entry. Possible previous entries are discarded.
     */
    void setGroupIds(const std::vector<std::uint16_t>& groupIds);

    /**
     * @return Get groupID_info_4cc value.
     */
    FourCCInt getGroupIdInfo4CC() const;

    /**
     * @param groupID_info_4cc Set groupID_info_4cc value.
     */
    void setGroupIdInfo4CC(FourCCInt groupID_info_4cc);

private:
    FourCCInt mGroupIdInfo;
    std::vector<std::uint16_t> mGroupdIds;
};

#endif
