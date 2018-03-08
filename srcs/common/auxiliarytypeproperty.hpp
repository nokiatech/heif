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

#ifndef AUXILIARYTYPEPROPERTY_HPP
#define AUXILIARYTYPEPROPERTY_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"


namespace ISOBMFF
{
    class BitStream;
}

/// @brief Auxiliary Type Property class
/// @details 'auxC' property implementation. Every auxiliary images is associated with one AuxiliaryTypeProperty.
class AuxiliaryTypeProperty : public FullBox
{
public:
    AuxiliaryTypeProperty();
    virtual ~AuxiliaryTypeProperty() = default;

    /// @param [in] type UTF-8 character string used to identify the type of the associated auxiliary image item.
    void setAuxType(const String& type);

    /// @return Auxiliary type. @see setAuxType() for details about the string.
    String getAuxType() const;

    /// @param [in] subtype Zero or more bytes. The semantics depend on auxType.
    void setAuxSubType(const Vector<std::uint8_t>& subtype);

    /// @return Auxiliary subtype. @see setAuxSubType() for details about the data.
    Vector<std::uint8_t> getAuxSubType() const;

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    String mAuxType;                   ///< UTF-8 character string of the Uniform Resource Name (URN).
    Vector<std::uint8_t> mAuxSubType;  ///< Subtype data, semantics depends on aux_type.
};

#endif
