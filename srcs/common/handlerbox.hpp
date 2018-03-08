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

#ifndef HANDLERBOX_HPP
#define HANDLERBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"


/** @brief Handler Box class. Extends from FullBox.
 *  @details 'hdlr' box implementation as specified in the ISOBMFF specification
 */
class HandlerBox : public FullBox
{
public:
    HandlerBox();
    virtual ~HandlerBox() = default;

    /** @brief Sets the media handler type.
     *  @param [in] handlerType 4CC code of the media handler type as a string value. */
    void setHandlerType(FourCCInt handlerType);

    /** @brief Gets the media handler type.
     *  @return The media handler type as a string value. */
    FourCCInt getHandlerType() const;

    /** @brief Sets the name field of the media handler.
     *  @param [in] name field as a string value  */
    void setName(const String& name);

    /** @brief Get the name field of the media handler.
     *  @return name field as a string value  */
    const String& getName() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a HandlerBox bitstream and fills in the necessary member variables
     *  @details If the media handler type is different than 'pict' then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    FourCCInt mHandlerType;  ///< type of the media handler
    String mName;            ///< designated name of the media handler
};

#endif /* end of include guard: HANDLERBOX_HPP */
