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

#ifndef EDITBOX_HPP
#define EDITBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief Edit List Box class. Extends from FullBox.
 *  @details 'elst' box implementation as specified in the ISOBMFF specification
 *  @details Both version 0 and 1 list entry formats are supported.
 */
class EditListBox : public FullBox
{
public:
    /// Edit List Entry Format version 0
    struct EntryVersion0
    {
        std::uint32_t mSegmentDuration;
        std::int32_t mMediaTime;
        std::uint16_t mMediaRateInteger;
        std::uint16_t mMediaRateFraction;
    };

    /// Edit List Entry Format version 1
    struct EntryVersion1
    {
        std::uint64_t mSegmentDuration;
        std::int64_t mMediaTime;
        std::uint16_t mMediaRateInteger;
        std::uint16_t mMediaRateFraction;
    };

    EditListBox();
    virtual ~EditListBox() = default;

    /** @brief Add an edit list entry of version 0
     *  @param [in] entry edit list entry of version 0 */
    void addEntry(const EntryVersion0& entry);

    /** @brief Add an edit list entry of version 1
     *  @param [in] entry edit list entry of version 1 */
    void addEntry(const EntryVersion1& entry);

    /** Return the number of entries in the Edit List */
    std::uint32_t numEntry() const;

    /** @brief Get an edit list entry of type either version 0 or version 1
     * @param [in] index edit list entry index (0-based)
     * @return Edit List entry of type either version 0 or version 1 */
    template <typename T>
    const T& getEntry(const std::uint32_t index) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses an Edit List Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<EntryVersion0> mEntryVersion0;  ///< vector of edit list entries of type verison 0
    Vector<EntryVersion1> mEntryVersion1;  ///< vector of edit list entries of type verison 1
};

/** @brief Implementation of getEntry(). Returns the entry in the Edit List at given 0-based index.
 *  @details Template is used in order to support entries of type version 0 and version 1.
 *  @returns Edit List Entry at 0-based index [index]
 *  @throws Run-time Error if a wrong version edit list entry is attempted
 */
template <typename T>
const T& EditListBox::getEntry(const std::uint32_t index) const
{
    if (std::is_same<T, EditListBox::EntryVersion0>::value)
    {
        return (const T&) mEntryVersion0.at(index);
    }

    if (std::is_same<T, EditListBox::EntryVersion1>::value)
    {
        return (const T&) mEntryVersion1.at(index);
    }
}

/** @brief Edit box class. Extends from Box.
 *  @details 'edts' box as specified in ISOBMFF specification.
 *  @details This box contains an edit list.
 */
class EditBox : public Box
{
public:
    EditBox();
    virtual ~EditBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;
    /** @brief Parses an Edit Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Sets the Edit List Box to be used.
     *  @param [in]  editListBox Pointer of the Edit List Box */
    void setEditListBox(std::shared_ptr<EditListBox> editListBox);

    /** @brief Gets the Edit List Box used.
     *  @return  Pointer of the Edit List Box */
    const EditListBox* getEditListBox() const;

private:
    std::shared_ptr<EditListBox> mEditListBox;  // Edit List Box pointer
};

#endif /* end of include guard: EDITBOX_HPP */
