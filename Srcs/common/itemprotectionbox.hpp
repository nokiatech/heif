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

#ifndef ITEMPROTECTIONBOX_HPP
#define ITEMPROTECTIONBOX_HPP

#include "fullbox.hpp"
#include "protectionschemeinfobox.hpp"
#include <vector>

/** Item Protection Box class
 * @details 'ipro' box implementation as specified in the ISOBMFF specification. */
class ItemProtectionBox : public FullBox
{
public:
    ItemProtectionBox();
    virtual ~ItemProtectionBox() = default;

    /** @return Number of contained Protection Scheme Info Boxes */
    std::uint16_t getSize() const;

    /** Get a ProtectionSchemeInfoBox box.
     *  @param [in] index 0-based index of the box */
    const ProtectionSchemeInfoBox& getEntry(std::uint16_t index) const;

    /** Add a new ProtectionSchemeInfoBox box.
     *  @param [in] sinf The ProtectionSchemeInfoBox to add
     *  @return 0-based index of the added box */
    std::uint16_t addEntry(const ProtectionSchemeInfoBox& sinf);

    /** Write box to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& bitstream);

    /** Read box from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& bitstream);

private:
    std::vector<ProtectionSchemeInfoBox> mProtectionInformation; ///< 'sinf' boxes
};

#endif /* ITEMPROTECTIONBOX_HPP */
