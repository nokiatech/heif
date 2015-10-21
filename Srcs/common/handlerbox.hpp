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

#ifndef HANDLERBOX_HPP
#define HANDLERBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <string>

/** @brief Handler Box class. Extends from FullBox.
 *  @details 'hdlr' box implementation as specified in the ISOBMFF specification
 */
class HandlerBox : public FullBox
{
public:
    HandlerBox();
    virtual ~HandlerBox() = default;

    /** @brief Sets the media handler type.
     *  @param [in] handlerType 4CC code of the media handler type as a string value. **/
    void setHandlerType(const std::string& handlerType);

    /** @brief Gets the media handler type.
     *  @return The media handler type as a string value. **/
    const std::string& getHandlerType() const;

    /** @brief Sets the name field of the media handler.
     *  @param [in] name field as a string value  **/
    void setName(const std::string& name);

    /** @brief Get the name field of the media handler.
     *  @return name field as a string value  **/
    const std::string& getName();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a HandlerBox bitstream and fills in the necessary member variables
     *  @details If the media handler type is different than 'pict' then a warning is logged
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    std::string mHandlerType; ///< type of the media handler
    std::string mName;   ///< designated name of the media handler
};

#endif /* end of include guard: HANDLERBOX_HPP */
