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

#ifndef TRACKREFERENCEBOX_HPP
#define TRACKREFERENCEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "trackreferencetypebox.hpp"

#include <string>
#include <vector>

/** @brief TrackBox class. Extends from Box.
 *  @details 'tref' box contains information related to the track's references, if any, as defined in the ISOBMFF and HEIF standards.
 *  @details There may be different reference types present in the box. **/
class TrackReferenceBox : public Box
{
public:
    TrackReferenceBox();
    virtual ~TrackReferenceBox() = default;

    /** @brief Clear the box from all references. **/
    void clearTrefBoxes();

    /** @brief Add a new track reference of a particular type.
     *  @param [in] trefTypeBox A new track reference type box data structure **/
    void addTrefTypeBox(TrackReferenceTypeBox& trefTypeBox);

    /** @brief Get the track references.
     *  @returns vector of track refeences of type TrackReferenceTypeBox.**/
    const std::vector<TrackReferenceTypeBox>& getTrefTypeBoxes() const;

    /** @brief Check for the presence of a particular reference type.
     *  @param [in] type Track reference type to be checked.
     *  @returns TRUE if present, FALSE if not found. **/
    bool isReferenceTypePresent(const std::string& type) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a TrackReferenceBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<TrackReferenceTypeBox> mTrefTypeBoxes; /// vector of track reference type boxes present in the track reference box
};

#endif /* end of include guard: TRACKREFERENCEBOX_HPP */
