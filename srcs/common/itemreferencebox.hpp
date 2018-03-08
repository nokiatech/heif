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

#ifndef ITEMREFERENCEBOX_HPP
#define ITEMREFERENCEBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <list>

namespace ISOBMFF
{
    class BitStream;
}

/** @brief Single Item Reference Box class. Extends from Box.
 *  @details The type related semantics (i.e. 4CC and meaning) of this box is defined by the standard being implemented.
 *           Member variable mIsLarge determines if the object is parsed and written as a SingleItemTypeReferenceBox or
 *           a SingleItemTypeReferenceBoxLarge. */
class SingleItemTypeReferenceBox : public Box
{
public:
    /**
     * @brief Constructor
     * @param isLarge True if the box is parsed and serialized as a SingleItemTypeReferenceBoxLarge.
     */
    SingleItemTypeReferenceBox(bool isLarge = false);
    virtual ~SingleItemTypeReferenceBox() = default;

    /** @brief Set 4CC reference type
     *  @param Type of this reference */
    void setReferenceType(FourCCInt referenceType);

    /** @brief Set "from-item" item Id value
     *  @param [in] itemID Item Id value*/
    void setFromItemID(std::uint32_t itemID);

    /** @brief Get "from-item" item Id value
     *  @return Item Id value*/
    std::uint32_t getFromItemID() const;

    /** @brief Adds a  "To-item" item Id value
     *  @param [in] itemID Item Id value to be added. */
    void addToItemID(std::uint32_t itemID);

    /** @brief Gets the list of "To-item" item Ids
     *  @return Vector of Item Id values */
    Vector<std::uint32_t> getToItemIds() const;

    /** @brief clears the list of "To-item" item Ids */
    void clearToItemIDs();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SingleItemTypeReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint32_t mFromItemId;         ///< "From-Item" item Id value
    Vector<std::uint32_t> mToItemIds;  ///< Vector of "To-Item" item Id values
    bool mIsLarge;                     ///< True if this is a SingleItemTypeReferenceBoxLarge
};

/** @brief Item Reference Box class. Extends from FullBox.
 *  @details 'iref' box contains item references of different referencing types as defined in the ISOBMFF standards. */

class ItemReferenceBox : public FullBox
{
public:
    ItemReferenceBox();
    virtual ~ItemReferenceBox() = default;

    /** @brief Adds an item reference of a particular type, from-id and to-id values.
     *  @param [in] type Type of the item reference
     *  @param [in] fromId "From-Id" field value of the item reference data structure
     *  @param [in] toId "To-Id" field value of the item reference data structure */
    void add(FourCCInt type, std::uint32_t fromId, std::uint32_t toId);

    /** @brief Returns the vector of item references of a particular reference type.
     *  @param [in] type Type of the item reference
     *  @return vector of item references with the requested reference type */
    Vector<SingleItemTypeReferenceBox> getReferencesOfType(FourCCInt type) const;

    /** @brief Parses an ItemReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

private:
    void addItemRef(const SingleItemTypeReferenceBox& ref);  ///< Add an item reference to the ItemReferenceBox

    List<SingleItemTypeReferenceBox> mReferenceList;  ///< List of item references of SingleItemTypeReferenceBox data structure
};

#endif /* end of include guard: ITEMREFERENCEBOX_HPP */
