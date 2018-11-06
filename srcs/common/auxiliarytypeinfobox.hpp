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

#ifndef AUXILIARYTYPEPINFOBOX_HPP
#define AUXILIARYTYPEPINFOBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"


namespace ISOBMFF
{
    class BitStream;
}

/// @brief Auxiliary Type Info Box class
/// @details 'auxi' implementation. Image Sequences can have auxiliary tracks which type is indicated by this box.
class AuxiliaryTypeInfoBox : public FullBox
{
public:
    AuxiliaryTypeInfoBox();
    virtual ~AuxiliaryTypeInfoBox() = default;

    /// @param [in] type UTF-8 character string used to identify the type of the associated auxiliary image sequence.
    void setAuxType(const String& type);

    /// @return Auxiliary type. @see setAuxType() for details about the string.
    const String& getAuxType() const;

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    String mAuxType;  ///< UTF-8 character string of the Uniform Resource Name (URN).
};

#endif
