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

#ifndef GROUPSLISTBOX_HPP
#define GROUPSLISTBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"
#include "entitytogroupbox.hpp"

/** @brief Groups List Box class. Extends from Box.
 *  @details 'grpl' box implementation
 */
class GroupsListBox : public Box
{
public:
    GroupsListBox();
    virtual ~GroupsListBox() = default;

    /** @brief Add an EntitytoGroups Box.
     *  @details There may be multiple such boxes for different entity groupings.
     *  @param [in] box An EntityToGroupBox to be added to the GroupsList Box */
    void addEntityToGroupBox(const EntityToGroupBox& box);

    /** @brief Get the list of EntityToGroup boxes preset in the GroupsList Box.
     *  @return  Vector of EntityToGroup Boxes */
    const Vector<EntityToGroupBox>& getEntityToGroupsBoxes() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF compliant file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a GroupsListBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<EntityToGroupBox> mEntityToGroupBoxVector;  ///< Vector of EntityToGroup Boxes.
};

#endif
