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

#ifndef ACCESSIBILITYTEXT_HPP
#define ACCESSIBILITYTEXT_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The AccessibilityText 'altt' as defined in the HEIF standard. */
class AccessibilityTextProperty : public FullBox
{
public:
    AccessibilityTextProperty();
    ~AccessibilityTextProperty() override = default;

    /** @param [in] Alternate text as UTF-8 character string. */
    void setText(const String& text);

    /** @return Alternate text as UTF-8 character string. */
    const String& getText() const;

    /** @param [in] lang UTF-8 character string RFC 5646 compliant language tag string ("en-US"), or empty if
     * unknown/undefined. */
    void setLang(const String& lang);

    /** @return Lang. @see setLang() for details about the string. */
    const String& getLang() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    String mAltText;  ///< alt_text UTF-8 string
    String mAltLang;  ///< alt_lang UTF-8 string
};

#endif
