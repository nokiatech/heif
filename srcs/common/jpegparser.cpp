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

#include "jpegparser.hpp"
#include "log.hpp"

#include <map>

JpegParser::JpegParser()
    : mData(nullptr)
    , mSize(0)
    , mIndex(0)
{
}

JpegParser::JpegInfo JpegParser::parse(uint8_t* data, const unsigned int size)
{
    JpegInfo info;
    Marker marker;
    mData  = data;
    mSize  = size;
    mIndex = 0;

    if ((data == nullptr) || (size == 0))
    {
        return info;
    }

    unsigned int segmentSize = 0;
    while (readNextSegment(marker, segmentSize))
    {
        printName(marker);

        if ((marker >= Marker::SOF0) && (marker <= Marker::SOF15) && (marker != Marker::DHT))
        {
            try
            {
                info.imageHeight = readUint16(mIndex + 3);
                info.imageWidth  = readUint16(mIndex + 5);
            }
            catch (...)
            {
                break;
            }

            logInfo() << "JpegParser: SOFn marker found. Read image dimensions (WxH):" << info.imageWidth << " x "
                      << info.imageHeight << std::endl;

            if (info.imageHeight == 0)
            {
                logWarning() << "JpegParser: Image height extraction from frame data is not supported." << std::endl;
                // Height should be extracted from frame data, but it is not supported yet.
                break;
            }
        }
        else if (marker == Marker::APP0)
        {
            logInfo() << "JpegParser: APP0 segment found. This could imply a JFIF file." << std::endl;
        }
        else if (marker == Marker::APP1)
        {
            logInfo() << "JpegParser: APP1 segment found. This could imply an Exif file." << std::endl;
        }
        else if (marker == Marker::EOI)
        {
            info.parsingOk = true;
            break;
        }

        mIndex += segmentSize;
    }

    mData = nullptr;
    return info;
}

bool JpegParser::readNextSegment(Marker& marker, unsigned int& size)
{
    logInfo() << "JpegParser: Reading a segment from offset: " << mIndex << std::endl;
    if (!readNextMarker(marker))
    {
        return false;
    }
    if ((marker == Marker::SOI) || (marker == Marker::EOI))
    {
        size = 0;  // These segments contain no other data.
        return true;
    }
    else if (marker == Marker::SOS)
    {
        // SOS segment does not start with length, end needs to be searched.
        if (!getSosSegmentSize(size))
        {
            logWarning() << "JpegParser: Failure while reading SOS segment." << std::endl;
            return false;
        }
    }
    else
    {
        // Most segments start with a 16-bit length field.
        try
        {
            size = readUint16(mIndex);
        }
        catch (...)
        {
            return false;
        }
    }

    return true;
}

bool JpegParser::readNextMarker(Marker& marker)
{
    if (mIndex + 1 >= mSize)
    {
        return false;
    }

    // Skip one or more 0xff bytes.
    bool foundFF = false;
    while ((mIndex < mSize) && (mData[mIndex] == 0xff))
    {
        foundFF = true;
        ++mIndex;
    }
    if (!foundFF)
    {
        logInfo() << "JpegParser: Not found 0xff byte when looking for marker." << std::endl;
        return false;
    }

    marker = Marker(mData[mIndex]);
    ++mIndex;

    return true;
}

bool JpegParser::getSosSegmentSize(unsigned int& size) const
{
    auto index = mIndex;
    while ((index + 1) < mSize)
    {
        if ((mData[index] == 0xff) && (mData[index + 1] == static_cast<uint8_t>(Marker::EOI)))
        {
            size = index - mIndex;
            return true;
        }
        ++index;
    }
    return false;
}


std::uint16_t JpegParser::readUint16(const unsigned int index) const
{
    if (index + 1 >= mSize)
    {
        throw RuntimeError("JpegParser: Failure while reading an uint16 from JPEG file bitstream.");
    }
    return ((mData[index] & 0xff) << 8) | (mData[index + 1] & 0xff);
}

void JpegParser::printName(const Marker marker) const
{
    const std::map<Marker, std::string> markerNames = {
        {Marker::SOF0, "SOF0"},   {Marker::SOF1, "SOF1"},   {Marker::SOF2, "SOF2"},   {Marker::DHT, "DHT"},
        {Marker::DQT, "DQT"},     {Marker::DRI, "DRI"},     {Marker::SOF3, "SOF3"},   {Marker::SOF5, "SOF5"},
        {Marker::SOF6, "SOF6"},   {Marker::SOF7, "SOF7"},   {Marker::SOF9, "SOF9"},   {Marker::SOF10, "SOF10"},
        {Marker::SOF11, "SOF11"}, {Marker::SOF13, "SOF13"}, {Marker::SOF14, "SOF14"}, {Marker::SOF15, "SOF15"},
        {Marker::SOI, "SOI"},     {Marker::EOI, "EOI"},     {Marker::RST0, "RST0"},   {Marker::RST1, "RST1"},
        {Marker::RST2, "RST2"},   {Marker::RST3, "RST3"},   {Marker::RST4, "RST4"},   {Marker::RST5, "RST5"},
        {Marker::RST6, "RST6"},   {Marker::RST7, "RST7"},   {Marker::SOS, "SOS"},     {Marker::APP0, "APP0"},
        {Marker::APP1, "APP1"},   {Marker::APP2, "APP2"},   {Marker::APP3, "APP3"},   {Marker::APP4, "APP4"},
        {Marker::APP5, "APP5"},   {Marker::APP6, "APP6"},   {Marker::APP7, "APP7"},   {Marker::APP8, "APP8"},
        {Marker::APP9, "APP9"},   {Marker::APP10, "APP10"}, {Marker::APP11, "APP11"}, {Marker::APP12, "APP12"},
        {Marker::COM, "COM"}};

    if (markerNames.count(marker))
    {
        logInfo() << "JpegParser: Read marker '" << markerNames.at(marker) << "'" << std::endl;
    }
    else
    {
        logInfo() << "JpegParser: Read an unrecognized marker." << std::endl;
    }
}
