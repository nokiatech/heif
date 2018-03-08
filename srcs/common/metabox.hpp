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

#ifndef METABOX_HPP
#define METABOX_HPP

#include "customallocator.hpp"
#include "datainformationbox.hpp"
#include "fullbox.hpp"
#include "groupslistbox.hpp"
#include "handlerbox.hpp"
#include "itemdatabox.hpp"
#include "iteminfobox.hpp"
#include "itemlocationbox.hpp"
#include "itempropertiesbox.hpp"
#include "itemprotectionbox.hpp"
#include "itemreferencebox.hpp"
#include "primaryitembox.hpp"

/**
 * Meta Box class
 * @details 'meta' box implementation as specified in ISOBMFF specification.
 */
class MetaBox : public FullBox
{
public:
    MetaBox();
    virtual ~MetaBox() = default;

    /**
     * @return Handler type from the contained HandlerBox.
     */
    FourCCInt getHandlerType() const;

    /**
     * Set handler type in the contained HandlerBox.
     * @param handler Handler type to declare the format of the MetaBox contents.
     */
    void setHandlerType(FourCCInt handler);

    /**
     * @return Reference to the contained PrimaryItemBox. If a parsed file did not contain a PrimaryItemBox, the
     *         referenced box is an empty one.
     */
    const PrimaryItemBox& getPrimaryItemBox() const;

    /**
     * Set Primary item identifier in the contained PrimaryItemBox
     * @param itemId Primary item identifier
     */
    void setPrimaryItem(std::uint32_t itemId);

    /**
     * @return Reference to the contained ItemInfoBox (Item Information Box. If a parsed file did not contain
     *         an ItemInfoBox, the box is an empty one.
     */
    const ItemInfoBox& getItemInfoBox() const;

    /**
     * Set contained ItemInfoBox
     * @param itemInfoBox ItemInfoBox to copy to MetaBox
     */
    void setItemInfoBox(const ItemInfoBox& itemInfoBox);

    /**
     * @return Reference to the contained ItemLocationBox. If a parsed file did not contain
     *         an ItemLocationBox, the box is an empty one.
     */
    const ItemLocationBox& getItemLocationBox() const;

    /**
     * @return Reference to the contained ItemReferenceBox. If a parsed file did not contain
     *         an ItemReferenceBox, the box is an empty one.
     */
    const ItemReferenceBox& getItemReferenceBox() const;

    /**
     * @return Reference to the contained ItemPropertiesBox. If a parsed file did not contain
     *         an ItemPropertiesBox, the box is an empty one.
     */
    const ItemPropertiesBox& getItemPropertiesBox() const;

    /**
     * Add a new Property or FullProperty, and associate itemIds to it.
     * @param box       The new Property or FullProperty.
     * @param itemIds   Item IDs to associate with the property.
     * @param essential True if the property is essential for items, meaning that a reader is required to process it.
     * @return 1-based index of the added property in the ItemPropertyContainer.
     */
    std::uint16_t addProperty(std::shared_ptr<Box> box, const Vector<std::uint32_t>& itemIds, bool essential);

    /**
     * Associate itemIds to an existing Property or FullProperty.
     * @param index     Index of an existing item in the ItemPropertyContainer of the ItemPropertiesBox contained by MetaBox.
     * @param itemIds   Item IDs to associate with the property.
     * @param essential True if the property is essential for items, meaning that a reader is required to process it.
     */
    void addProperty(std::uint16_t index, const Vector<std::uint32_t>& itemIds, bool essential);

    /**
     * Get a reference to a ProtectionSchemeInfoBox inside of the contained ItemProtectionBox.
     * @param index Index of the ProtectionSchemeInfoBox inside the ItemProtectionBox.
     * @return Reference to the contained ProtectionSchemeInfoBox.
     */
    const ProtectionSchemeInfoBox& getProtectionSchemeInfoBox(std::uint16_t index) const;

    /**
     * @return Reference to the contained GroupsListBox. If a parsed file did not contain
     *         a GroupsListBox, the box is an empty one.
     */
    const GroupsListBox& getGroupsListBox() const;

    /**
     * Add a new EntityToGroupBox of to the contained GroupsListBox.
     * @param type    Grouping type identifier.
     * @param groupId ID of the entity group, a non-negative integer assigned to the particular group.
     * @param itemIds Item IDs in the group
     */
    void addEntityGrouping(FourCCInt type, std::uint32_t groupId, const Vector<std::uint32_t>& itemIds);

    /**
     * @return Reference to the contained DataInformationBox. If a parsed file did not contain
     *         a DataInformationBox, the box is an empty one.
     */
    const DataInformationBox& getDataInformationBox() const;

    /**
     * @return Reference to the contained ItemDataBox. If a parsed file did not contain
     *         an ItemDataBox, the box is an empty one.
     */
    const ItemDataBox& getItemDataBox() const;

    /**
     * @brief Add an ItemLocation entry with an ItemLocationExtent to the contained ItemLocationBox
     * @details file_offset is used as the construction method. This method should be used when the item data
     * is located in a MediaDataBox.
     * @param itemId     Item ID for which the ItemLocation is added
     * @param offset     Data offset (from baseOffset)
     * @param length     Length of the data
     * @param baseOffset Base offset of the data. This can be used during dual-pass file writing, to give location of the MediaDataBox on the second pass.
     */
    void addIloc(std::uint32_t itemId, std::uint64_t offset, std::uint64_t length, std::uint64_t baseOffset);

    /**
     * Add an ItemInfoEntry to the contained ItemInfoBox
     * @details ItemInfoEntry version 2 is used.
     * @param itemId Item ID for which the ItemInfoEntry is added
     * @param type   item_type field in ItemInfoEntry. Typically 4 printable characters, like 'hvc1'.
     * @param name   item_name field in ItemInfoEntry, a symbolic name of the item.
     * @param hidden True indicates that the item is hidden.
     */
    void addItem(std::uint32_t itemId, FourCCInt type, const String& name, bool hidden = false);

    /**
     * Add an ItemInfoEntry to the contained ItemInfoBox
     * @param infe ItemInfoEntry to add.
     */
    void addItem(const ItemInfoEntry& infe);

    /**
     * Add an item reference to the contained ItemReferenceBox
     * @param type   An indication of the type of the reference, typically 4 printable characters.
     * @param fromId ID of the item that refers to other item
     * @param toId   ID of the item referred to
     */
    void addItemReference(FourCCInt type, std::uint32_t fromId, std::uint32_t toId);

    /**
     * Add a new item data to the contained ItemDataBox. The ItemLocation and ItemInfoEntry are added.
     * @param itemId ID of the new item
     * @param type   item_type field in ItemInfoEntry. Typically 4 printable characters, like 'hvc1'.
     * @param name   item_name field in ItemInfoEntry, a symbolic name of the item.
     * @param data   Data of the item which will be added to the contained ItemDataBox.
     */
    void addIdatItem(std::uint32_t itemId, FourCCInt type, const String& name,
                     const Vector<uint8_t>& data);

    /**
     * Add data to ItemDataBox and the related ItemLocationExtent for an already existing item.
     * @param itemId ID for which the ItemLocationExtent is added.
     * @param data   Data to ItemDataBox
     */
    void addItemIdatExtent(std::uint32_t itemId, const Vector<uint8_t>& data);

    /**
     * @brief Add ItemInfoEntry and ItemLocation entries for an item which is located in MediaDataBox.
     * @details This does not add any extents to the item. One or multiple ItemLocationExtent must be added using method
     *         addItemExtent()
     * @param itemId     ID of the new item
     * @param type       item_type field in ItemInfoEntry. Typically 4 printable characters, like 'hvc1'.
     * @param name       item_name field in ItemInfoEntry, a symbolic name of the item.
     * @param baseOffset Base offset of the data. This can be used during dual-pass file writing, to give location of
     *                   the MediaDataBox on the second pass.
     */
    void addMdatItem(std::uint32_t itemId, FourCCInt type, const String& name,
                     std::uint64_t baseOffset);

    /**
     * Add an extent to an existing item which is located in ItemDataBox.
     * @param itemId ID of the already existing item for which the extent is added.
     * @param offset Offset of the data
     * @param length Length of the data
     */
    void addItemExtent(std::uint32_t itemId, std::uint64_t offset, std::uint64_t length);

    /**
     * @brief setItemFileOffsetBase Set base offset for items which have file offset construction method.
     * @param baseOffset            Base offset in bytes. This could be e.g. start location of the 'mdat' box.
     * @param itemIds               Item ids which base offset to change.
     */
    void setItemFileOffsetBase(std::uint64_t baseOffset);

    /**
     * @brief setImageHidden Set image hidden.
     * @param itemId         ID of the image.
     * @param isHidden       True if the image is set hidden, false if not.
     */
    void setImageHidden(std::uint32_t itemId, bool isHidden);

    /**
     * @brief Serialize box data to the BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    HandlerBox mHandlerBox;                  ///< Corresponds to ISOBMFF specification MetaBox field theHandler
    PrimaryItemBox mPrimaryItemBox;          ///< Corresponds to ISOBMFF specification MetaBox field primary_resource
    DataInformationBox mDataInformationBox;  ///< Corresponds to ISOBMFF specification MetaBox field file_locations
    ItemLocationBox mItemLocationBox;        ///< Corresponds to ISOBMFF specification MetaBox field item_locations
    ItemProtectionBox mItemProtectionBox;    ///< Corresponds to ISOBMFF specification MetaBox field protections
    ItemInfoBox mItemInfoBox;                ///< Corresponds to ISOBMFF specification MetaBox field item_infos
    ItemReferenceBox mItemReferenceBox;      ///< Corresponds to ISOBMFF specification MetaBox field item_refs
    ItemDataBox mItemDataBox;                ///< Corresponds to ISOBMFF specification MetaBox field item_data
    // ISOBMFF optional box IPMPControlBox IPMP_control is not implemented.

    ItemPropertiesBox mItemPropertiesBox;  ///< From HEIF specification
    GroupsListBox mGroupsListBox;          ///< From HEIF specification
};

#endif /* end of include guard: METABOX_HPP */
