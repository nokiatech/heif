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

#ifndef USERDESCRIPTIONPROPERTY_HPP
#define USERDESCRIPTIONPROPERTY_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The User description property 'udes' as defined in the ISOBMFF standard. */
class UserDescriptionProperty : public FullBox
{
public:
    UserDescriptionProperty();
    ~UserDescriptionProperty() override = default;

    /// @param [in] lang UTF-8 character string RFC 5646 compliant language tag string ("en-US"), or empty if
    /// unknown/undefined.
    void setLang(const String& lang);

    /// @return Lang. @see setLang() for details about the string.
    const String& getLang() const;

    /// @param [in] name UTF-8 character string for name for the item or group of entities.
    void setName(const String& name);

    /// @return Name. @see setName() for details about the string.
    const String& getName() const;

    /// @param [in] description UTF-8 character string for description of the item or group of entities.
    void setDescription(const String& description);

    /// @return Description. @see setDescription() for details about the string.
    const String& getDescription() const;

    /// @param [in] tags UTF-8 character string for comman-separated user-defined tags.
    void setTags(const String& tags);

    /// @return Tags. @see setTags() for details about the string.
    const String& getTags() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    String mLang;         ///< UTF-8 character string for language tag
    String mName;         ///< UTF-8 character string for name for the item or group of entities.
    String mDescription;  ///< UTF-8 character string for description of the item or group of entities.
    String mTags;         ///< UTF-8 character string for comman-separated user-defined tags.
};

#endif
