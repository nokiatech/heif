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

#ifndef PROJECTIONFORMATBOX_HPP
#define PROJECTIONFORMATBOX_HPP

#include <cstdint>
#include "fullbox.hpp"

/** @brief Fullbox about projection format of podv scheme
 *  @details Defined in the OMAF standard. **/
class ProjectionFormatBox : public FullBox
{
public:
    ProjectionFormatBox();
    virtual ~ProjectionFormatBox() = default;

    enum class ProjectionType : std::uint8_t
    {
        EQUIRECTANGULAR = 0,
        CUBEMAP
    };

    ProjectionType getProjectionType() const;

    /** @brief Sets projection type 5bit value. Extra bits are masked out.
     */
    void setProjectionType(ProjectionType);

    /** @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint8_t mProjectionType;  // for masking it is easier to keep this as uint8 internally
};

#endif
