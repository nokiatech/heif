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

#ifndef CODINGCONSTRAINTSBOX_HPP
#define CODINGCONSTRAINTSBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief Coding Constraints box class
 *  @details 'ccst' box implementation. It includes fields specifying that certain constraints are obeyed by the
 *           samples of the track. */
class CodingConstraintsBox : public FullBox
{
public:
    CodingConstraintsBox();
    CodingConstraintsBox(const CodingConstraintsBox& other);
    virtual ~CodingConstraintsBox() = default;

    /** @param [in] flag Set to true to indicate that samples that are not sync samples, if any,
     *                   are predicted only from sync samples. */
    void setAllRefPicsIntra(bool flag);

    /** @return Get all_ref_pics_intra value. @see setAllRefPicsIntra() for more detail. */
    bool getAllRefPicsIntra() const;

    /** @param [in] flag True indicates that intra prediction may or may not be used.
     *                   False indicates that intra prediction is not used in the inter predicted images. */
    void setIntraPredUsed(bool flag);

    /** @return Get intra_pred_used value. @see setIntraPredUsed() for more detail. */
    bool getIntraPredUsed() const;

    /** @param [in] maxRefPics The maximum number of reference images that may be used for decoding any single image
     *                         within an image sequence. The field size is 4-bits, and the value 15 indicates
     *                         that any number of reference images may be present. maxRefPics greater than 15 will
     *                         result the value to be set to 15. */
    void setMaxRefPicUsed(std::uint8_t maxRefPics);

    /** @return Get max_ref_per_pic  value. @see setMaxRefPicUsed() for more detail. */
    std::uint8_t getMaxRefPicUsed() const;

    /** @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    bool mAllRefPicsIntra;        ///< CodingConstraintsBox field all_ref_pics_intra
    bool mIntraPredUsed;          ///< CodingConstraintsBox field intra_pred_used
    std::uint8_t mMaxRefPicUsed;  ///< CodingConstraintsBox field max_ref_per_pic
};

#endif /* end of include guard: CODINGCONSTRAINTSBOX_HPP */
