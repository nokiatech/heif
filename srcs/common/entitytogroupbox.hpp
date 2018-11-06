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

#ifndef ENTITYTOGROUPBOX_HPP
#define ENTITYTOGROUPBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/**
 * @brief Entity to Group box implementation as specified in the HEIF specification.
 */
class EntityToGroupBox : public FullBox
{
public:
    EntityToGroupBox();
    EntityToGroupBox(FourCCInt type);
    virtual ~EntityToGroupBox() = default;

    typedef Vector<std::uint32_t> EntityIds;  ///< Convenience type for handling entity IDs

    /**
     * @brief setEntityIds Set entity id values of the box.
     * @param entityIds New entity id values of the box. Previous ids, if present, are replaced.
     */
    void setEntityIds(const EntityIds& entityIds);

    /**
     * @brief getEntityIds Get entity ids of the grouping.
     * @return Entity ids of the grouping.
     */
    const EntityIds& getEntityIds() const;

    /**
     * @brief setGroupId Set group id.
     * @param groupId Group id, a non-negative integer.
     */
    void setGroupId(std::uint32_t groupId);

    /**
     * @brief getGroupId Get group id.
     * @return Group id, a non-negative integer.
     */
    std::uint32_t getGroupId() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint32_t mGroupId;  ///< group_id as in HEIF specification
    EntityIds mEntityIds;    ///< entity_ids of this group (track/item ids)
};

#endif
