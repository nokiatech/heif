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

#ifndef ENTITYTOGROUPBOX_HPP
#define ENTITYTOGROUPBOX_HPP

#include "fullbox.hpp"
#include <vector>

/**
 * @brief Entity to Group box of 'altr' type
 * @details Box implementation as specified in the HEIF specification.
 *
 * Note that currently only 'altr' type EntityToGroupBoxes are supported.
 */
class EntityToGroupBox : public FullBox
{
public:
    EntityToGroupBox();
    virtual ~EntityToGroupBox() = default;

    typedef std::vector<std::uint32_t> EntityIds; ///< Convenience type for handling entity IDs

    void setEntityIds(const EntityIds& entityIds);
    EntityIds getEntityIds() const;

    /**
     * @brief Serialize box data to the BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(BitStream& bitstr);

    /**
     * @brief Deserialize box data from the BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint32_t mGroupId; ///< group_id as in HEIF specification
    EntityIds mEntityIds;   ///< entity_ids of this group (track/item ids)
};

#endif
