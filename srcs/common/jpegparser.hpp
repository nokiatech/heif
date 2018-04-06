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

#ifndef JPEGPARSER_H
#define JPEGPARSER_H

#include <cstdint>

/**
 * @brief The JpegParser class
 * Parse a JPEG file to search contained image width and height.
 */
class JpegParser
{
public:
    JpegParser();
    ~JpegParser() = default;

    struct JpegInfo
    {
        bool parsingOk            = false;
        std::uint16_t imageWidth  = 0;
        std::uint16_t imageHeight = 0;
    };

    /**
     * @brief parse Parse a JPEG file data to find dimensions of the contained image.
     * @param data  JPEG data. Ownership of the data is not transferred. The caller must free the memory when it is no
     * more required.
     * @param size  Size of the JPEG data in bytes.
     * @return JpegInfo struct containing parsing results. parsingoK is set to true in case parsing was successfull.
     */
    JpegInfo parse(uint8_t* data, unsigned int size);

private:
    uint8_t* mData;       ///< JPEG file data.
    unsigned int mSize;   ///< JPEG file data size.
    unsigned int mIndex;  ///< Parsing index in the data.

    /// JPEG segment marker types.
    enum Marker : uint8_t
    {
        SOF0  = 0xC0,
        SOF1  = 0xC1,
        SOF2  = 0xC2,
        SOF3  = 0xC3,
        DHT   = 0xC4,
        SOF5  = 0xC5,
        SOF6  = 0xC6,
        SOF7  = 0xC7,
        SOF9  = 0xC9,
        SOF10 = 0xCA,
        SOF11 = 0xCB,
        SOF13 = 0xCD,
        SOF14 = 0xCE,
        SOF15 = 0xCF,
        RST0  = 0xD0,
        RST1  = 0xD1,
        RST2  = 0xD2,
        RST3  = 0xD3,
        RST4  = 0xD4,
        RST5  = 0xD5,
        RST6  = 0xD6,
        RST7  = 0xD7,
        SOI   = 0xD8,
        EOI   = 0xD9,
        SOS   = 0xDA,
        DQT   = 0xDB,
        DRI   = 0xDD,
        APP0  = 0xE0,
        APP1  = 0xE1,
        APP2  = 0xE2,
        APP3  = 0xE3,
        APP4  = 0xE4,
        APP5  = 0xE5,
        APP6  = 0xE6,
        APP7  = 0xE7,
        APP8  = 0xE8,
        APP9  = 0xE9,
        APP10 = 0xEA,
        APP11 = 0xEB,
        APP12 = 0xEC,
        COM   = 0xFE
    };

    /**
     * @brief readNextMarker Read marker of the next segment. Possible padding 0xff bytes are read before the marker.
     *                       Parsing index is updated to point to segment beginning (the byte after the marker).
     * @param marker         Marker if one was found.
     * @return               True if a marker was read successfully, false otherwise.
     */
    bool readNextMarker(Marker& marker);

    /**
     * @brief readNextSegment Read the type/marker and size of next JPEG segment.
     *                        Parsing index is updated to point to segment beginning (the byte after the marker).
     * @param marker          Marker which was read.
     * @param size            Size of the segment, not including marker.
     * @return                True if a segment was read successfully, false otherwise.
     */
    bool readNextSegment(Marker& marker, unsigned int& size);

    /**
     * @brief getSosSegmentSize Determine SOS segment size by scanning for the EOI marker.
     * @param size              Size of the segment in bytes.
     * @return                  True if an EOI marker was found and segment size was calculated successfully, false
     * otherwise.
     */
    bool getSosSegmentSize(unsigned int& size) const;

    /**
     * @brief readUint16 Read an 16-bit uint value from index.
     * @param index      Index where to read the value.
     * @return The read value.
     * @throws runtime_error in case index is invalid.
     */
    std::uint16_t readUint16(unsigned int index) const;

    /**
     * @brief printName Print the human-readable name of a marker enumeration.
     * @param marker    A marker enumeration.
     */
    void printName(Marker marker) const;
};

#endif  // JPEGPARSER_H
