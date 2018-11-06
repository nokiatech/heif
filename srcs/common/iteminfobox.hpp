/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef ITEMINFOBOX_HPP
#define ITEMINFOBOX_HPP

#include <cstdint>
#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}
class ItemInfoEntry;
class ItemInfoExtension;

/** @brief Item Information Box class. Extends from FullBox.
 *  @details 'iinf' box contains information about an item's type, encoding and protection scheme */

class ItemInfoBox : public FullBox
{
public:
    ItemInfoBox();
    ItemInfoBox(uint8_t version);
    virtual ~ItemInfoBox() = default;

    /** @brief Clear the contents of the Item Information List */
    void clear();

    /** @return All Item IDs in this ItemInfoBox */
    const Vector<std::uint32_t>& getItemIds() const;

    /** @brief Add a new Item Information Entry to the Item Information List
     * @param [in] infoEntry ItemInfoEntry to be added. */
    void addItemInfoEntry(const ItemInfoEntry& infoEntry);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a ItemInfoBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Get the ItemInfoEntry of particular item type
     * @param [in] itemType Type (4CC) of the item to find.
     * @param [in] index    0-based index of the ItemInformationEntry to be retrieved
     * @return ItemInfoEntry at the desired index */
    ItemInfoEntry* findItemWithType(FourCCInt itemType, unsigned int index = 0);

    /** @brief Return an ItemInfoEntry of an item with a desired itemId
     * @param [in] itemId ID of an Item
     * @return ItemInfoEntry with the desired itemId
     * @throws Runtime Exception if the requested ItemInfoEntry is not found. */
    ItemInfoEntry getItemById(uint32_t itemId) const;

    /** @brief Return an ItemInfoEntry of an item with a desired itemId
     * @param [in] itemId ID of an Item
     * @return ItemInfoEntry with the desired itemId
     * @throws Runtime Exception if the requested ItemInfoEntry is not found. */
    ItemInfoEntry& getItemById(uint32_t itemId);

private:
    Vector<ItemInfoEntry> mItemInfoList;  ///< Vector of the ItemInfoEntry Boxes
    Vector<std::uint32_t> mItemIds;
};

/** @brief Item Information Entry Box. Extends from FullBox.
 *  @details 'infe' box contains information about an item's properties as stored in the meta box. */
class ItemInfoEntry : public FullBox
{
public:
    ItemInfoEntry();
    virtual ~ItemInfoEntry();

    ItemInfoEntry(const ItemInfoEntry&) = default;
    ItemInfoEntry& operator=(const ItemInfoEntry&) = default;

    /** @brief Set an item Id to the ItemInfoEntry
     * @param [in] id ID of an Item */
    void setItemID(std::uint32_t id);

    /** @brief Get the ID of an item
     * @return ID of an Item */
    std::uint32_t getItemID() const;

    /** @brief Set the item protection index of the ItemInfoEntry
     * @param [in] idx the index of the item protection scheme as listed in the item protection box */
    void setItemProtectionIndex(std::uint16_t idx);

    /** @brief Get the item protection index of the ItemInfoEntry
     * @return The index of the item protection scheme as listed in the item protection box */
    std::uint16_t getItemProtectionIndex() const;

    /** @brief Set the item name of the ItemInfoEntry
     * @param [in] name the name of the ItemInfoEntry */
    void setItemName(const String& name);

    /** @brief Get the item name of the ItemInfoEntry
     * @return The name of the ItemInfoEntry */
    const String& getItemName() const;

    /** @brief Set the content type of the ItemInfoEntry
     * @param [in] contentType the content type of the ItemInfoEntry */
    void setContentType(const String& contentType);

    /** @brief Get the content type of the ItemInfoEntry
     * @return The content type of the ItemInfoEntry */
    const String& getContentType() const;

    /** @brief Set the content encoding of the ItemInfoEntry
     * @param [in] contentEncoding The content encoding of the ItemInfoEntry */
    void setContentEncoding(const String& contentEncoding);

    /** @brief Get the content encoding of the ItemInfoEntry
     * @return The content encoding of the ItemInfoEntry */
    const String& getContentEncoding() const;

    /** @brief Set the extension Type of the ItemInfoEntry
     * @param [in] extensionType The extension type of the ItemInfoEntry */
    void setExtensionType(const String& extensionType);

    /** @brief Get the extension Type of the ItemInfoEntry
     * @return The extension type of the ItemInfoEntry */
    const String& getExtensionType() const;

    /** @brief Set the item type of the ItemInfoEntry
     * @param [in] itemType The item type of the ItemInfoEntry */
    void setItemType(FourCCInt itemType);

    /** @brief Get the item type of the ItemInfoEntry
     * @return The item type of the ItemInfoEntry */
    FourCCInt getItemType() const;

    /** @brief Set the item URI type of the ItemInfoEntry
     * @param [in] itemUriType The item URI type of the ItemInfoEntry */
    void setItemUriType(const String& itemUriType);

    /** @brief Get the item URI type of the ItemInfoEntry
     * @return The item URI type of the ItemInfoEntry */
    const String& getItemUriType() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses an ItemInfoEntry bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    // Version 0, 1, 2, 3
    std::uint32_t mItemID;               ///< ID of the item
    std::uint16_t mItemProtectionIndex;  ///< Item protection index
    String mItemName;                    ///< Item name
    String mContentType;                 ///< Content type
    String mContentEncoding;             ///< Content encoding

    // Version 1
    String mExtensionType;  ///< The extension type

    /** @todo Verify if default shallow copy is wanted/needed for ItemInfoExtension */
    std::shared_ptr<ItemInfoExtension> mItemInfoExtension;  ///< Item info extension

    // Version 2
    FourCCInt mItemType;  ///< Item type
    String mItemUriType;  ///< Item UIR type
};

/** @brief Item Information Extension abstract class.
 *  @details Item Information Extension class can be used to extend the item information data strcuture. */
class ItemInfoExtension
{
public:
    ItemInfoExtension()          = default;
    virtual ~ItemInfoExtension() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void write(ISOBMFF::BitStream& bitstr) = 0;

    /** @brief Parses an ItemInfoExtension bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parse(ISOBMFF::BitStream& bitstr) = 0;
};

/** @brief FD Item Information Extension class.
 *  @details Implements Item Information Extension class of type 'fdel' as defined in the ISOBMFF standard. */
class FDItemInfoExtension : public ItemInfoExtension
{
public:
    FDItemInfoExtension();
    virtual ~FDItemInfoExtension() = default;

    /** @brief Set the content location field
     * @param [in] location Content Location field as defined in the ISOBMFF standard. */
    void setContentLocation(const String& location);

    /** @brief Get the content location field
     * @return Content Location field as defined in the ISOBMFF standard. */
    const String& getContentLocation();

    /** @brief Set the content MD5 value.
     * @param [in] md5 MD5 value as defined in the ISOBMFF standard. */
    void setContentMD5(const String& md5);

    /** @brief Get the content MD5 value.
     * @return MD5 value as defined in the ISOBMFF standard. */
    const String& getContentMD5();

    /** @brief Set the content length.
     * @param [in] length content length value as defined in the ISOBMFF standard. */
    void setContentLength(uint64_t length);

    /** @brief Get the content length.
     * @return Content Length value as defined in the ISOBMFF standard. */
    uint64_t getContentLength();

    /** @brief Set the transfer length.
     * @param [in] length transfer length value as defined in the ISOBMFF standard. */
    void setTranferLength(uint64_t length);

    /** @brief Get the content length.
     * @return Content Length value as defined in the ISOBMFF standard. */
    uint64_t getTranferLength();

    /** @brief Set the number of entries in the Group ID list.
     * @param [in] numID Number of entries in the Group ID list. */
    void setNumGroupID(uint8_t numID);

    /** @brief Get the number of entries in the Group ID list.
     * @return Number of entries in the Group ID list. */
    uint8_t getNumGroupID();

    /** @brief Set the Group ID of an index in the list.
     * @param [in] idx Group ID to insert to the Group ID List.
     * @param [in] id Group Id value to be inserted to the Group ID List. */
    void setGroupID(const std::uint32_t idx, const uint32_t id);

    /** @brief Set the Group ID of an index in the list.
     * @param [in] idx Group ID to insert to the Group ID List.
     * @return Group Id value. */
    uint32_t getGroupID(const std::uint32_t idx);

    /** @brief Creates the bitstream that represents the class in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the class data. */
    virtual void write(ISOBMFF::BitStream& bitstr);

    /** @brief Parses an FDItemInfoExtension bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the class data */
    virtual void parse(ISOBMFF::BitStream& bitstr);

private:
    String mContentLocation;    ///< Content location
    String mContentMD5;         ///< MD5 value
    uint64_t mContentLength;    ///< Content length
    uint64_t mTransferLength;   ///< Transfer length
    uint8_t mEntryCount;        ///< Entry count
    Vector<uint32_t> mGroupID;  ///< Vector of Group ID values
};

#endif /* end of include guard: ITEMINFOBOX_HPP */
