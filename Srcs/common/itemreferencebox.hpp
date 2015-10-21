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

#ifndef ITEMREFERENCEBOX_HPP
#define ITEMREFERENCEBOX_HPP

#include "bbox.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <list>
#include <vector>

class BitStream;

/** @brief Single Item Reference Box class. Extends from Box.
 *  @details The type related semantics (i.e. 4CC and meaning) of this box is defined by the standard being implemented. **/

class SingleItemTypeReferenceBox : public Box
{
public:
    SingleItemTypeReferenceBox();
    SingleItemTypeReferenceBox(const std::string& referenceType);
    virtual ~SingleItemTypeReferenceBox() = default;

    /** @brief Set "from-item" item Id value
     *  @param [in] itemID Item Id value*/
    void setFromItemID(std::uint16_t itemID);

    /** @brief Get "from-item" item Id value
     *  @return Item Id value*/
    std::uint16_t getFromItemID() const;

    /** @brief Adds a  "To-item" item Id value
     *  @param [in] itemID Item Id value to be added.*/
    void addToItemID(std::uint16_t itemID);

    /** @brief Gets the list of "To-item" item Ids
     *  @return Vector of Item Id values */
    std::vector<std::uint16_t> getToItemIds() const;

    /** @brief clears the list of "To-item" item Ids */
    void clearToItemIDs();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a SingleItemTypeReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    std::uint16_t mFromItemId; /// "From-Item" item Id value
    std::vector<std::uint16_t> mToItemIds; /// vector of "To-Item" item Id values
};

/** @brief Item Reference Box class. Extends from FullBox.
 *  @details 'iref' box contains item references of different referencing types as defined in the HEIF and ISOBMFF standards. **/

class ItemReferenceBox : public FullBox
{
public:
    ItemReferenceBox();
    virtual ~ItemReferenceBox() = default;

    /** @brief Adds an item reference of a perticular type, from-id and to-id values.
     *  @param [in] type Type of the item reference
     *  @param [in] fromId "From-Id" field value of the item reference data structure
     *  @param [in] toId "To-Id" field value of the item reference data structure */
    void add(const std::string& type, std::uint16_t fromId, std::uint16_t toId);

    /** @brief Returns the vector of item references of a particular reference type.
     *  @param [in] type Type of the item reference
     *  @return vector of item references with the requested reference type */
    std::vector<SingleItemTypeReferenceBox> getReferencesOfType(const std::string& type) const;

    /** @brief Parses an ItemReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

private:
    void addItemRef(const SingleItemTypeReferenceBox& ref); /// add an item reference to the ItemReferenceBox

    std::list<SingleItemTypeReferenceBox> mReferenceList; /// list of item references of SingleItemTypeReferenceBox data structure
};

#endif /* end of include guard: ITEMREFERENCEBOX_HPP */
