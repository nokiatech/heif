/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef STEREOVIDEOBOX_HPP
#define STEREOVIDEOBOX_HPP

#include <cstdint>
#include "fullbox.hpp"

/** @brief omaf extended Stereo video box stvi
 *  @details Defined in the OMAF standard and in isobmf **/
class StereoVideoBox : public FullBox
{
public:
    StereoVideoBox();
    virtual ~StereoVideoBox() = default;

    enum class SingleViewAllowedType : std::uint8_t
    {
        None    = 0,
        Right   = 1,
        Left    = 2,
        Invalid = 3  // only values 0-2 are allowed
    };

    enum class StereoSchemeType : std::uint8_t
    {
        Iso14496 = 1,
        Iso13818 = 2,
        Iso23000 = 3,
        Povd     = 4

    };

    /// Table D‐8 of ISO/IEC 14496‐10 (frame_packing_arrangement_type)
    enum class Iso14496FramePackingArrangementType : std::uint32_t
    {
        CheckerboardInterleaving = 0,
        ColumnInterleaving       = 1,
        RowInterleaving          = 2,
        SideBySidePacking        = 3,
        TopBottomPacking         = 4,
        TemporalInterleaving     = 5
    };

    /// Table L.1 of ISO/IEC 13818‐2 (arrangement_type)
    enum class Iso13818ArrangementType : std::uint32_t
    {
        StereoSideBySide   = 0b0000000000000011,
        StereoTopAndBottom = 0b0000000000000100,
        TwoDimensionVideo  = 0b0000000000001000
    };

    /// Table 4 of ISO / IEC 23000‐11:2009 (Stereoscopic Composition Type)
    enum class Iso23000StereoscopicCompositionType : std::uint8_t
    {
        SideBysideHalf         = 0x00,
        VerticalLineInterleave = 0x01,
        FrameSequential        = 0x02,
        LeftRightViewSequence  = 0x03,
        TopBottomHalf          = 0x04,
        SideBysideFull         = 0x05,
        TopBottomFull          = 0x06
    };

    struct Iso23000ArrangementType
    {
        Iso23000StereoscopicCompositionType compositionType;
        bool isLeftFirst;
    };

    // OMAF specs 7.6.1.2
    enum class PovdFrameCompositionType : std::uint8_t
    {
        TopBottomPacking     = 3,
        SideBySidePacking    = 4,
        TemporalInterleaving = 5
    };

    struct PovdArrangementType
    {
        PovdFrameCompositionType compositionType;
        bool useQuincunxSampling;
    };

    union StereoIndicationType {
        uint32_t valAsUint32;                          // common getter for full data
        Iso14496FramePackingArrangementType Iso14496;  // 4 bytes
        Iso13818ArrangementType Iso13818;              // 4 bytes
        Iso23000ArrangementType Iso23000;              // 2 bytes
        PovdArrangementType Povd;                      // 2 bytes
    };

    void setSingleViewAllowed(SingleViewAllowedType allowed);
    SingleViewAllowedType getSingleViewAllowed() const;

    void setStereoScheme(StereoSchemeType scheme);
    StereoSchemeType getStereoScheme() const;

    void setStereoIndicationType(StereoIndicationType indicationType);
    StereoIndicationType getStereoIndicationType() const;

    /** @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    SingleViewAllowedType mSingleViewAllowed;
    StereoSchemeType mStereoScheme;
    StereoIndicationType mStereoIndicationType;
};

#endif
