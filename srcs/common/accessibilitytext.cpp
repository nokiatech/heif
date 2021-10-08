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

#include "accessibilitytext.hpp"

#include "bitstream.hpp"

AccessibilityTextProperty::AccessibilityTextProperty()
    : FullBox("altt", 0, 0)
{
}

void AccessibilityTextProperty::setText(const String& text)
{
    mAltText = text;
}

const String& AccessibilityTextProperty::getText() const
{
    return mAltText;
}

void AccessibilityTextProperty::setLang(const String& lang)
{
    mAltLang = lang;
}

const String& AccessibilityTextProperty::getLang() const
{
    return mAltLang;
}

void AccessibilityTextProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.writeZeroTerminatedString(mAltText);
    output.writeZeroTerminatedString(mAltLang);
    updateSize(output);
}

void AccessibilityTextProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    input.readZeroTerminatedString(mAltText);
    input.readZeroTerminatedString(mAltLang);
}
