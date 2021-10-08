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

#include "userdescriptionproperty.hpp"

#include "bitstream.hpp"

UserDescriptionProperty::UserDescriptionProperty()
    : FullBox("udes", 0, 0)
{
}

void UserDescriptionProperty::setLang(const String& lang)
{
    mLang = lang;
}

const String& UserDescriptionProperty::getLang() const
{
    return mLang;
}

void UserDescriptionProperty::setName(const String& name)
{
    mName = name;
}

const String& UserDescriptionProperty::getName() const
{
    return mName;
}

void UserDescriptionProperty::setDescription(const String& description)
{
    mDescription = description;
}

const String& UserDescriptionProperty::getDescription() const
{
    return mDescription;
}

void UserDescriptionProperty::setTags(const String& tags)
{
    mTags = tags;
}

const String& UserDescriptionProperty::getTags() const
{
    return mTags;
}

void UserDescriptionProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.writeZeroTerminatedString(mLang);
    output.writeZeroTerminatedString(mName);
    output.writeZeroTerminatedString(mDescription);
    output.writeZeroTerminatedString(mTags);
    updateSize(output);
}

void UserDescriptionProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    input.readZeroTerminatedString(mLang);
    input.readZeroTerminatedString(mName);
    input.readZeroTerminatedString(mDescription);
    input.readZeroTerminatedString(mTags);
}
