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

#ifndef ITEMINFOBOX_HPP
#define ITEMINFOBOX_HPP

#include "fullbox.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class BitStream;
class ItemInfoEntry;
class ItemInfoExtension;

/** @brief Item Information Box class. Extends from FullBox.
 *  @details 'iinf' box contains information about an item's type, encoding and protection scheme **/

class ItemInfoBox : public FullBox
{
public:
    ItemInfoBox();
    ItemInfoBox(uint8_t version);
    virtual ~ItemInfoBox() = default;

    /** @brief Clear the contents of the Item Information List */
    void clear();

    /** @brief Get the number of entries in the Item Information List
     * @return number of entries in the Item Information List */
    std::uint32_t getEntryCount() const;

    /** @return All Item IDs in this ItemInfoBox */
    std::vector<std::uint32_t> getItemIds() const;

    /** @brief Add a new Item Information Entry to the Item Information List
     * @param [in] infoEntry ItemInfoEntry to be added. */
    void addItemInfoEntry(const ItemInfoEntry& infoEntry);

    /** @brief Get the ItemInfoEntry at a 0-based index in the list.
     * @param [in] idx 0-based index of the ItemInformationEntry to be retrieved
     * @return ItemInformationEntry at the desired index */
    const ItemInfoEntry& getItemInfoEntry(int idx) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a ItemInfoBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

    /** @brief Get the ItemInfoEntry of particular item type
     * @param [in] itemType Type (4CC) of the item to find.
     * @param [in] index    0-based index of the ItemInformationEntry to be retrieved
     * @return ItemInfoEntry at the desired index */
    ItemInfoEntry* findItemWithType(const std::string& itemType, unsigned int index = 0);

    /** @brief Return item and its index for the specified itemType and itemID
     * @param [in] itemID ID of the item to be found
     * @param [in] itemType Type of the item to be found
     * @param [out] index 0-based index of the ItemInformationEntry to be retrieved
     * @return ItemInfoEntry at the desired index */
    ItemInfoEntry* findItemWithTypeAndID(const std::string& itemType, unsigned int itemID, unsigned int& index);

    /** @brief Return the number of items of a particular item type in the ItemInfoEntry Box
     * @param [in] itemType Type of the item to be found
     * @return Number of items matching the criteria */
    unsigned int countNumberOfItems(const std::string& itemType);

    /** @brief Return a vector of items of a particular item type in the ItemInfoEntry Box
     * @param [in] itemType Type of the item to be found
     * @return Vector of items matching the criteria */
    std::vector<ItemInfoEntry> getItemsByType(const std::string& itemType) const;

    /** @brief Return an ItemInfoEntry of an item with a desired itemId
     * @param [in] itemId ID of an Item
     * @return ItemInfoEntry with the desired itemId
     * @throws Runtime Exception if the requested ItemInfoEntry is not found. **/
    ItemInfoEntry getItemById(uint32_t itemId) const;

private:
    std::map<std::uint32_t, ItemInfoEntry> mItemInfos; ///< ItemInfoEntry Boxes
};


/** @brief Item Information Entry Box. Extends from FullBox.
 *  @details 'infe' box contains information about an item's properties as stored in the meta box.**/
class ItemInfoEntry : public FullBox
{
public:
    ItemInfoEntry();
    virtual ~ItemInfoEntry();

    ItemInfoEntry(const ItemInfoEntry& itemInfoEntry) = default;
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
    void setItemName(const std::string& name);

    /** @brief Get the item name of the ItemInfoEntry
     * @return The name of the ItemInfoEntry */
    const std::string& getItemName() const;

    /** @brief Set the content type of the ItemInfoEntry
     * @param [in] contentType the content type of the ItemInfoEntry */
    void setContentType(const std::string& contentType);

    /** @brief Get the content type of the ItemInfoEntry
     * @return The content type of the ItemInfoEntry */
    const std::string& getContentType() const;

    /** @brief Set the content encoding of the ItemInfoEntry
     * @param [in] contentEncoding The content encoding of the ItemInfoEntry */
    void setContentEncoding(const std::string& contentEncoding);

    /** @brief Get the content encoding of the ItemInfoEntry
     * @return The content encoding of the ItemInfoEntry */
    const std::string& getContentEncoding() const;

    /** @brief Set the extension Type of the ItemInfoEntry
     * @param [in] extensionType The extension type of the ItemInfoEntry */
    void setExtensionType(const std::string& extensionType);

    /** @brief Get the extension Type of the ItemInfoEntry
     * @return The extension type of the ItemInfoEntry */
    const std::string& getExtensionType() const;

    /** @brief Set the item type of the ItemInfoEntry
     * @param [in] itemType The item type of the ItemInfoEntry */
    void setItemType(const std::string& itemType);

    /** @brief Get the item type of the ItemInfoEntry
     * @return The item type of the ItemInfoEntry */
    const std::string& getItemType() const;

    /** @brief Set the item URI type of the ItemInfoEntry
     * @param [in] itemUriType The item URI type of the ItemInfoEntry */
    void setItemUriType(const std::string& itemUriType);

    /** @brief Get the item URI type of the ItemInfoEntry
     * @return The item URI type of the ItemInfoEntry */
    const std::string& getItemUriType() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses an ItemInfoEntry bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    // Version 0, 1, 2, 3
    std::uint32_t mItemID;              ///< ID of the item
    std::uint16_t mItemProtectionIndex; ///< Item protection index
    std::string mItemName;              ///< Item name
    std::string mContentType;           ///< Content type
    std::string mContentEncoding;       ///< Content encoding

    // Version 1
    std::string mExtensionType; ///< The extension type

    /** @todo Verify if default shallow copy is wanted/needed for ItemInfoExtension **/
    ItemInfoExtension* mItemInfoExtension; ///< Item info extension

    // Version 2
    std::string mItemType;    ///< Item type
    std::string mItemUriType; ///< Item UIR type
};


/** @brief Item Information Extension abstract class.
 *  @details Item Information Extension class can be used to extend the item information data strcuture.**/
class ItemInfoExtension
{
public:
    ItemInfoExtension() = default;
    virtual ~ItemInfoExtension() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void write(BitStream& bitstr) = 0;

    /** @brief Parses an ItemInfoExtension bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parse(BitStream& bitstr) = 0;
};


/** @brief FD Item Information Extension class.
 *  @details Implements Item Information Extension class of type 'fdel' as defined in the ISOBMFF standard.**/
class FDItemInfoExtension : public ItemInfoExtension
{
public:
    FDItemInfoExtension();
    virtual ~FDItemInfoExtension() = default;

    /** @brief Set the content location field
     * @param [in] location Content Location field as defined in the ISOBMFF standard. **/
    void setContentLocation(const std::string& location);

    /** @brief Get the content location field
     * @return Content Location field as defined in the ISOBMFF standard. **/
    const std::string& getContentLocation();

    /** @brief Set the content MD5 value.
     * @param [in] md5 MD5 value as defined in the ISOBMFF standard. **/
    void setContentMD5(const std::string& md5);

    /** @brief Get the content MD5 value.
     * @return MD5 value as defined in the ISOBMFF standard. **/
    const std::string& getContentMD5();

    /** @brief Set the content length.
     * @param [in] length content length value as defined in the ISOBMFF standard. **/
    void setContentLength(uint64_t length);

    /** @brief Get the content length.
     * @return Content Length value as defined in the ISOBMFF standard. **/
    uint64_t getContentLength();

    /** @brief Set the transfer length.
     * @param [in] length transfer length value as defined in the ISOBMFF standard. **/
    void setTranferLength(uint64_t length);

    /** @brief Get the content length.
     * @return Content Length value as defined in the ISOBMFF standard. **/
    uint64_t getTranferLength();

    /** @brief Set the number of entries in the Group ID list.
     * @param [in] numID Number of entries in the Group ID list. **/
    void setNumGroupID(uint8_t numID);

    /** @brief Get the number of entries in the Group ID list.
     * @return Number of entries in the Group ID list. **/
    uint8_t getNumGroupID();

    /** @brief Set the Group ID of an index in the list.
     * @param [in] idx Group ID to insert to the Group ID List.
     * @param [in] id Group Id value to be inserted to the Group ID List. **/
    void setGroupID(int idx, uint32_t id);

    /** @brief Set the Group ID of an index in the list.
     * @param [in] idx Group ID to insert to the Group ID List.
     * @return Group Id value. **/
    uint32_t getGroupID(int idx);

    /** @brief Creates the bitstream that represents the class in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the class data. */
    virtual void write(BitStream& bitstr);

    /** @brief Parses an FDItemInfoExtension bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the class data */
    virtual void parse(BitStream& bitstr);

private:
    std::string mContentLocation;   ///< Content location
    std::string mContentMD5;        ///< MD5 value
    uint64_t mContentLength;        ///< Content length
    uint64_t mTransferLength;       ///< Transfer length
    uint8_t mEntryCount;            ///< Entry count
    std::vector<uint32_t> mGroupID; ///< Vector of Group ID values
};

#endif /* end of include guard: ITEMINFOBOX_HPP */
