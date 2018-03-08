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

#ifndef JPEGCONFIGURATIONBOX_HPP
#define JPEGCONFIGURATIONBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"

/** @brief JPEG Configuration item property class
 *  @details 'jpgC' box implementation. This is used by
 *           JPEG items as a decoder configuration property.
 *           Data can include e.g. quantization and other tables
 *           common to several JPEG image items.
 */
class JpegConfigurationBox : public Box
{
public:
    JpegConfigurationBox();
    virtual ~JpegConfigurationBox() = default;

    /**
     * @brief getPrefix Get JPEG prefix data of the property.
     * @return JPEG prefix data.
     */
    Vector<uint8_t> getPrefix() const;

    /**
     * @brief setPrefix Set JPEG prefix data of the property.
     * @param data JPEG prefix data.
     */
    void setPrefix(const Vector<uint8_t>& data);

    /** @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<uint8_t> mJpegPrefix;  ///< JPEG prefix data.
};

#endif /* end of include guard: JPEGCONFIGURATIONBOX_HPP */
