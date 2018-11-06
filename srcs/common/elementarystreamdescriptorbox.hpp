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

#ifndef ELEMENTARYSTREAMDESCRIPTORBOX_HPP
#define ELEMENTARYSTREAMDESCRIPTORBOX_HPP

#include <vector>
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/// @brief Elementary Stream Descriptor box
/// @details 'esds' box containing the ES_Descriptor as specified in ISO/IEC FDIS 14496-14 specification
class ElementaryStreamDescriptorBox : public FullBox
{
public:
    ElementaryStreamDescriptorBox();
    ElementaryStreamDescriptorBox(const ElementaryStreamDescriptorBox& box);
    virtual ~ElementaryStreamDescriptorBox() = default;

    struct DecoderSpecificInfo  // 7.2.6.7.1 in ISO/IEC 14496-1:2010(E)
    {
        std::uint8_t DecSpecificInfoTag = 0;
        std::uint32_t size              = 0;
        Vector<uint8_t> DecSpecificInfo;
    };

    struct DecoderConfigDescriptor  // 7.2.6.6.1 in ISO/IEC 14496-1:2010(E)
    {
        std::uint8_t DecoderConfigDescrTag = 0;
        std::uint32_t size                 = 0;
        std::uint8_t objectTypeIndication  = 0;
        std::uint8_t streamType            = 0;
        std::uint32_t bufferSizeDB         = 0;
        std::uint32_t maxBitrate           = 0;
        std::uint32_t avgBitrate           = 0;
        DecoderSpecificInfo decSpecificInfo;
    };

    struct ES_Descriptor  // 7.2.6.5.1 in ISO/IEC 14496-1:2010(E)
    {
        std::uint8_t ES_DescrTag      = 0;
        std::uint32_t size            = 0;
        std::uint16_t ES_ID           = 0;
        std::uint8_t flags            = 0;
        std::uint16_t dependsOn_ES_ID = 0;
        std::uint8_t URLlength        = 0;
        String URLstring;
        std::uint16_t OCR_ES_Id = 0;
        DecoderConfigDescriptor decConfigDescr;
    };

    /// @param [out] byteStream return argument for DecSpecificInfo. @see 7.2.6.7.1 ISO/IEC 14496-1:2010(E) for details.
    /// @return true if the parameter set is available
    bool getOneParameterSet(Vector<uint8_t>& byteStream) const;

    /// @brief set the ES_Descriptor. @see ISO/IEC 14496-1 for details.
    void setESDescriptor(ElementaryStreamDescriptorBox::ES_Descriptor& esDescriptor);

    /// @return ES_Descriptor. @see ISO/IEC 14496-1 for details.
    const ES_Descriptor& getESDescriptor() const;

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    ES_Descriptor mES_Descriptor;  /// ES_Descriptor as specified in ISO/IEC 14496-1
    List<DecoderSpecificInfo> mOtherDecSpecificInfo;
};

#endif
