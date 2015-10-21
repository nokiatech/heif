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

#ifndef AUXILIARYTYPEPROPERTY_HPP
#define AUXILIARYTYPEPROPERTY_HPP

#include "fullbox.hpp"

#include <string>
#include <vector>

class BitStream;

/// @brief Auxiliary Type Property class
/// @details 'auxC' property implementation. Every auxiliary images is associated with one AuxiliaryTypeProperty.
class AuxiliaryTypeProperty : public FullBox
{
public:
    AuxiliaryTypeProperty();
    virtual ~AuxiliaryTypeProperty() = default;

    /// @param [in] type UTF-8 character string used to identify the type of the associated auxiliary image item.
    void setAuxType(const std::string& type);

    /// @return Auxiliary type. @see setAuxType() for details about the string.
    std::string getAuxType() const;

    /// @param [in] subtype Zero or more bytes. The semantics depend on auxType.
    void setAuxSubType(const std::vector<std::uint8_t>& subtype);

    /// @return Auxiliary subtype. @see setAuxSubType() for details about the data.
    std::vector<std::uint8_t> getAuxSubType() const;

    /// @see Box::writeBox()
    virtual void writeBox(BitStream& output);

    /// @see Box::parseBox()
    virtual void parseBox(BitStream& input);

private:
    std::string mAuxType; ///< UTF-8 character string of the Uniform Resource Name (URN).
    std::vector<std::uint8_t> mAuxSubType; ///< Subtype data, semantics depends on aux_type.
};

#endif
