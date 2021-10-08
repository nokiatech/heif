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

#ifndef ROTATIONBOX_HPP
#define ROTATIONBOX_HPP

#include <cstdint>
#include <vector>
#include "commontypes.hpp"
#include "fullbox.hpp"

/** @brief his box provides information on the rotation of this track
 *  @details Defined in the OMAF standard. **/
class RotationBox : public FullBox
{
public:
    RotationBox();
    RotationBox(const RotationBox&);
    virtual ~RotationBox() = default;

    struct Rotation
    {
        std::int32_t yaw;
        std::int32_t pitch;
        std::int32_t roll;
    };

    Rotation getRotation() const;
    void setRotation(Rotation rot);

    /** @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Rotation mRotation;
};

#endif
