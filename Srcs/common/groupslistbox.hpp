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

#ifndef GROUPSLISTBOX_HPP
#define GROUPSLISTBOX_HPP

#include "bbox.hpp"
#include "entitytogroupbox.hpp"
#include <vector>

/** @brief Groups List Box class. Extends from Box.
 *  @details 'grpl' box implementation as specified in the HEIF specification
 */
class GroupsListBox : public Box
{
public:
    GroupsListBox();
    virtual ~GroupsListBox() = default;

    /** @brief Add an EntitytoGroups Box.
     *  @details There may be multiple such boxes for different entity groupings.
     *  @param [in] box An EntityToGroupBox to be added to the GroupsList Box **/
    void addEntityToGroupBox(const EntityToGroupBox& box);

    /** @brief Get the list of EntityToGroup boxes preset in the GroupsList Box.
     *  @return  Vector of EntityToGroup Boxes **/
    const std::vector<EntityToGroupBox>& getEntityToGroupsBoxes() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF compliant file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a GroupsListBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<EntityToGroupBox> mEntityToGroupBoxVector; ///< Vector of EntityToGroup Boxes.
};

#endif
