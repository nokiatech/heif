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

#ifndef COVERAGEINFORMATIONBOX_HPP
#define COVERAGEINFORMATIONBOX_HPP

#include <cstdint>
#include <vector>

#include "commontypes.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** @brief his box provides information on the content coverage of this track
 *  @details Defined in the OMAF standard. **/
class CoverageInformationBox : public FullBox
{
public:
    CoverageInformationBox();
    CoverageInformationBox(const CoverageInformationBox&);
    virtual ~CoverageInformationBox() = default;

    enum class CoverageShapeType : std::uint8_t
    {
        FOUR_GREAT_CIRCLES = 0,
        TWO_AZIMUTH_AND_TWO_ELEVATION_CIRCLES
    };

    /** @brief SphereRegion with an additional view_idc flag if
     */
    struct CoverageSphereRegion
    {
        ViewIdcType viewIdc;  // only valid if mViewIdcPresenceFlag is set
        SphereRegion region;
    };

    CoverageShapeType getCoverageShapeType() const;
    void setCoverageShapeType(CoverageShapeType);

    bool getViewIdcPresenceFlag() const;
    void setViewIdcPresenceFlag(bool);

    ViewIdcType getDefaultViewIdc() const;
    void setDefaultViewIdc(ViewIdcType);

    Vector<CoverageSphereRegion*> getSphereRegions() const;
    void addSphereRegion(UniquePtr<CoverageSphereRegion>);

    /** @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Dump box data to logInfo() in human readable format.
     */
    void dump() const;

private:
    CoverageShapeType mCoverageShapeType;
    bool mViewIdcPresenceFlag;
    ViewIdcType mDefaultViewIdc;
    Vector<UniquePtr<CoverageSphereRegion>> mSphereRegions;
};

#endif
