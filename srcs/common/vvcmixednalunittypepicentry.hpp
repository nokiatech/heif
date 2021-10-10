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

#ifndef VVCMIXEDNALUNITTYPEPICENTRY_HPP
#define VVCMIXEDNALUNITTYPEPICENTRY_HPP

#include <vector>

#include "bitstream.hpp"
#include "samplegroupdescriptionentry.hpp"

/** @brief VvcMixedNalUnitTypePicEntry class. Inherits from abstrct VisualSampleGroupEntry
 * (SampleGroupDescriptionEntry).
 *  @details Implements VvcMixedNALUnitTypePicEntry as defined in the Carriage of VVC and EVC in ISOBMFF standard. */
class VvcMixedNalUnitTypePicEntry : public SampleGroupDescriptionEntry
{
public:
    VvcMixedNalUnitTypePicEntry();
    ~VvcMixedNalUnitTypePicEntry() override = default;

    /** @brief Get the size of the Sample Group Entry.
     *  @returns Serialized byte size of the sample group entry */
    std::uint32_t getSize() const override;

    /** @brief Serialize the VvcMixedNalUnitTypePicEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [out] bitstr Bitstream containing the serialized VvcMixedNalUnitTypePicEntry data structure */
    void writeEntry(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parse a serialized VvcMixedNalUnitTypePicEntry data structure.
     *  @details Implemented by the extending class.
     *  @param [in] bitstr Bitstream containing the serialized VvcMixedNalUnitTypePicEntry data structure */
    void parseEntry(ISOBMFF::BitStream& bitstr) override;

    typedef std::pair<std::uint32_t, std::uint32_t>
        MixSubpTrackIdxs;  ///< mix_subp_track_idx1[i], mix_subp_track_idx2[i];

    std::uint16_t getPpsMixNaluTypesInPicBitPos() const;
    void setPpsMixNaluTypesInPicBitPos(std::uint16_t ppsMixNaluTypesInPicBitPos);

    std::uint8_t getPpsId() const;
    void setPpsId(std::uint8_t ppsId);

    Vector<MixSubpTrackIdxs> getMixSubpTrackIds() const;
    void setMixSubpTrackIds(const Vector<MixSubpTrackIdxs>& mixSubpTrackIds);

private:
    Vector<MixSubpTrackIdxs> mMixSubpTrackIdxs;
    std::uint16_t mPpsMixNaluTypesInPicBitPos;
    std::uint8_t mPpsId;
};

#endif /* end of include guard: VVCMIXEDNALUNITTYPEPICENTRY_HPP */
