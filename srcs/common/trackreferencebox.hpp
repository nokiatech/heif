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

#ifndef TRACKREFERENCEBOX_HPP
#define TRACKREFERENCEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "trackreferencetypebox.hpp"


/** @brief TrackBox class. Extends from Box.
 *  @details 'tref' box contains information related to the track's references, if any, as defined in the ISOBMFF standard.
 *  @details There may be different reference types present in the box. */
class TrackReferenceBox : public Box
{
public:
    TrackReferenceBox();
    virtual ~TrackReferenceBox() = default;

    /** @brief Clear the box from all references. */
    void clearTrefBoxes();

    /** @brief Add a new track reference of a particular type.
     *  @param [in] trefTypeBox A new track reference type box data structure */
    void addTrefTypeBox(TrackReferenceTypeBox& trefTypeBox);

    /** @brief Get the track references.
     *  @returns vector of track refeences of type TrackReferenceTypeBox. */
    const Vector<TrackReferenceTypeBox>& getTrefTypeBoxes() const;

    /** @brief Check for the presence of a particular reference type.
     *  @param [in] type Track reference type to be checked.
     *  @returns TRUE if present, FALSE if not found. */
    bool isReferenceTypePresent(FourCCInt type) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a TrackReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<TrackReferenceTypeBox> mTrefTypeBoxes;  ///< Vector of track reference type boxes present in the track reference box
};

#endif /* end of include guard: TRACKREFERENCEBOX_HPP */
