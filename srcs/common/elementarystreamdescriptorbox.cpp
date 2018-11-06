/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "elementarystreamdescriptorbox.hpp"
#include <algorithm>
#include <stdexcept>
#include "bitstream.hpp"

ElementaryStreamDescriptorBox::ElementaryStreamDescriptorBox()
    : FullBox("esds", 0, 0)
    , mES_Descriptor()
{
}

ElementaryStreamDescriptorBox::ElementaryStreamDescriptorBox(const ElementaryStreamDescriptorBox& box)
    : FullBox(box.getType(), 0, 0)
    , mES_Descriptor(box.mES_Descriptor)
    , mOtherDecSpecificInfo(box.mOtherDecSpecificInfo)
{
}

bool ElementaryStreamDescriptorBox::getOneParameterSet(Vector<uint8_t>& byteStream) const
{
    if (mES_Descriptor.decConfigDescr.decSpecificInfo.DecSpecificInfoTag == 5)
    {
        byteStream = mES_Descriptor.decConfigDescr.decSpecificInfo.DecSpecificInfo;
        return true;
    }
    else
    {
        return false;
    }
}

void ElementaryStreamDescriptorBox::setESDescriptor(ElementaryStreamDescriptorBox::ES_Descriptor& esDescriptor)
{
    mES_Descriptor = esDescriptor;
}

const ElementaryStreamDescriptorBox::ES_Descriptor& ElementaryStreamDescriptorBox::getESDescriptor() const
{
    return mES_Descriptor;
}

namespace
{
    int highestBit(std::uint32_t value)
    {
        int n = 0;
        while (value)
        {
            value >>= 1;
            ++n;
        }
        return n;
    }

    uint64_t writeSize(BitStream& bitstr, std::uint32_t size)
    {
        uint64_t sizeSize = 0;
        int currentBit    = (std::max(0, highestBit(size) - 1)) / 7 * 7;
        bool hasMore;
        do
        {
            hasMore = (size >> (currentBit - 7)) != 0;
            bitstr.write8Bits(((size >> currentBit) & 0x7F) | (hasMore ? 1u << 7 : 0));
            currentBit -= 7;
            ++sizeSize;
        } while (hasMore);
        return sizeSize;
    }

    void writeDecoderSpecificInfo(BitStream& bitstr,
                                  const ElementaryStreamDescriptorBox::DecoderSpecificInfo& decSpecificInfo)
    {
        bitstr.write8Bits(decSpecificInfo.DecSpecificInfoTag);
        writeSize(bitstr, decSpecificInfo.size);
        bitstr.write8BitsArray(decSpecificInfo.DecSpecificInfo, decSpecificInfo.size);
    }
}  // namespace

void ElementaryStreamDescriptorBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write8Bits(mES_Descriptor.ES_DescrTag);

    bool esSizeConverged = false;
    std::uint64_t esSizeSize;
    std::uint32_t esDescriptorSize = mES_Descriptor.size;

    BitStream esBitstr;
    /* Write the whole stuff, then figure out if we wrote the correct
     * size for it (we allos mES_Descriptor to be incorrect); rewrite
     * everything with the correct size. However, this may increase
     * the size due to bigger size having been written and thus moving
     * the remaining of the data forward, so we may need to loop even
     * thrice. */
    while (!esSizeConverged)
    {
        esBitstr.clear();
        esSizeSize = writeSize(esBitstr, esDescriptorSize);
        esBitstr.write16Bits(mES_Descriptor.ES_ID);
        esBitstr.write8Bits(mES_Descriptor.flags);
        if (mES_Descriptor.flags & 0x80)  // streamDependenceFlag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
        {
            esBitstr.write16Bits(mES_Descriptor.dependsOn_ES_ID);
        }

        if (mES_Descriptor.flags & 0x40)  // URL_Flag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
        {
            esBitstr.write8Bits(mES_Descriptor.URLlength);
            if (mES_Descriptor.URLlength)
            {
                esBitstr.writeString(mES_Descriptor.URLstring);
            }
        }

        if (mES_Descriptor.flags & 0x20)  // OCRstreamFlag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
        {
            esBitstr.write16Bits(mES_Descriptor.OCR_ES_Id);
        }

        esBitstr.write8Bits(mES_Descriptor.decConfigDescr.DecoderConfigDescrTag);

        BitStream decConfigBitstr;
        std::uint64_t decConfigSize = mES_Descriptor.decConfigDescr.size;
        std::uint64_t decConfigSizeSize;
        bool decConfigSizeConverged = false;
        while (!decConfigSizeConverged)
        {
            decConfigBitstr.clear();
            decConfigSizeSize = writeSize(decConfigBitstr, static_cast<uint32_t>(decConfigSize));
            decConfigBitstr.write8Bits(mES_Descriptor.decConfigDescr.objectTypeIndication);
            decConfigBitstr.write8Bits((mES_Descriptor.decConfigDescr.streamType << 2) | 0x01);
            decConfigBitstr.write24Bits(mES_Descriptor.decConfigDescr.bufferSizeDB);
            decConfigBitstr.write32Bits(mES_Descriptor.decConfigDescr.maxBitrate);
            decConfigBitstr.write32Bits(mES_Descriptor.decConfigDescr.avgBitrate);

            if (mES_Descriptor.decConfigDescr.decSpecificInfo.DecSpecificInfoTag == 5)
            {
                writeDecoderSpecificInfo(decConfigBitstr, mES_Descriptor.decConfigDescr.decSpecificInfo);
            }

            for (const auto& decSpecificInfo : mOtherDecSpecificInfo)
            {
                writeDecoderSpecificInfo(decConfigBitstr, decSpecificInfo);
            }

            decConfigSizeConverged = decConfigBitstr.getSize() == std::uint64_t(decConfigSize) + decConfigSizeSize;

            if (!decConfigSizeConverged)
            {
                decConfigSize = decConfigBitstr.getSize() - decConfigSizeSize;
            }
        }
        esBitstr.writeBitStream(decConfigBitstr);

        esSizeConverged = esBitstr.getSize() == std::uint64_t(esDescriptorSize) + esSizeSize;

        if (!esSizeConverged)
        {
            esDescriptorSize = std::uint32_t(esBitstr.getSize() - esSizeSize);
        }
    }
    bitstr.writeBitStream(esBitstr);
    updateSize(bitstr);
}

void ElementaryStreamDescriptorBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    //////////////////////////////////////////////
    //      Fill in struct ES_Descriptor        //
    //////////////////////////////////////////////
    mES_Descriptor.ES_DescrTag = bitstr.read8Bits();
    if (mES_Descriptor.ES_DescrTag != 3)  // ES_DescrTag
    {
        throw RuntimeError("ElementaryStreamDescritorBox ES_Descriptor.ES_DescrTag not valid");
    }
    /* Expandable class... need to find out size based on (from ISO/IEC 14496-1)
     * int sizeOfInstance = 0;
     * bit(1) nextByte;
     * bit(7) sizeOfInstance;
     * while(nextByte) {
     *      bit(1) nextByte;
     *      bit(7) sizeByte;
     *      sizeOfInstance = sizeOfInstance<<7 | sizeByte; }
     */
    std::uint8_t readByte = 0;
    std::uint32_t size    = 0;
    do
    {
        readByte              = bitstr.read8Bits();
        std::uint8_t sizeByte = (readByte & 0x7F);
        size                  = (size << 7) | sizeByte;
    } while (readByte & 0x80);

    mES_Descriptor.size  = size;
    mES_Descriptor.ES_ID = bitstr.read16Bits();
    mES_Descriptor.flags = bitstr.read8Bits();

    if (mES_Descriptor.flags & 0x80)  // streamDependenceFlag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
    {
        mES_Descriptor.dependsOn_ES_ID = bitstr.read16Bits();
    }

    if (mES_Descriptor.flags & 0x40)  // URL_Flag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
    {
        mES_Descriptor.URLlength = bitstr.read8Bits();
        if (mES_Descriptor.URLlength)
        {
            bitstr.readStringWithLen(mES_Descriptor.URLstring, mES_Descriptor.URLlength);
        }
    }

    if (mES_Descriptor.flags & 0x20)  // OCRstreamFlag as defined in 7.2.6.5.1 of ISO/IEC 14486-1:2010(E)
    {
        mES_Descriptor.OCR_ES_Id = bitstr.read16Bits();
    }

    //////////////////////////////////////////////////////////////////
    //      Fill in struct ES_Descriptor.DecoderConfigDescriptor    //
    //////////////////////////////////////////////////////////////////
    mES_Descriptor.decConfigDescr.DecoderConfigDescrTag = bitstr.read8Bits();
    if (mES_Descriptor.decConfigDescr.DecoderConfigDescrTag != 4)  // DecoderConfigDescrTag
    {
        throw RuntimeError("ElementaryStreamDescritorBox DecoderConfigDescriptor.DecoderConfigDescrTag not valid");
    }

    readByte = 0;
    size     = 0;
    do
    {
        readByte              = bitstr.read8Bits();
        std::uint8_t sizeByte = (readByte & 0x7f);
        size                  = (size << 7) | sizeByte;
    } while (readByte & 0x80);

    mES_Descriptor.decConfigDescr.size                 = size;
    mES_Descriptor.decConfigDescr.objectTypeIndication = bitstr.read8Bits();
    mES_Descriptor.decConfigDescr.streamType           = (bitstr.read8Bits() >> 2);
    mES_Descriptor.decConfigDescr.bufferSizeDB         = bitstr.read24Bits();
    mES_Descriptor.decConfigDescr.maxBitrate           = bitstr.read32Bits();
    mES_Descriptor.decConfigDescr.avgBitrate           = bitstr.read32Bits();

    /////////////////////////////////////////////////////////////////////////////////////
    //      Fill in struct ES_Descriptor.DecoderConfigDescriptor.DecoderSpecificInfo   //
    /////////////////////////////////////////////////////////////////////////////////////
    while (bitstr.numBytesLeft())  // DecoderSpecificInfo is optional.
    {
        std::uint8_t tag = bitstr.read8Bits();

        readByte = 0;
        size     = 0;
        do
        {
            readByte              = bitstr.read8Bits();
            std::uint8_t sizeByte = (readByte & 0x7f);
            size                  = (size << 7) | sizeByte;
        } while (readByte & 0x80);

        DecoderSpecificInfo decSpecificInfo;

        decSpecificInfo.DecSpecificInfoTag = tag;
        decSpecificInfo.size               = size;
        bitstr.read8BitsArray(decSpecificInfo.DecSpecificInfo, decSpecificInfo.size);

        if (tag == 5)  // DecSpecificInfoTag
        {
            mES_Descriptor.decConfigDescr.decSpecificInfo = std::move(decSpecificInfo);
        }
        else
        {
            mOtherDecSpecificInfo.push_back(std::move(decSpecificInfo));
        }
    }
}
