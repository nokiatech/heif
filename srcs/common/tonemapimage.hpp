/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef TONEMAPIMAGE_HPP
#define TONEMAPIMAGE_HPP

#include "customallocator.hpp"
#include "bitstream.hpp"

#include <cstdint>
#include <vector>

/// @brief Version information for gain map metadata
struct GainMapVersion
{
    uint16_t minimumVersion;  ///< Minimum version required to parse this metadata
    uint16_t writerVersion;   ///< Version of the writer that created this metadata
};

/// @brief Per-channel gain map parameters
struct GainMapChannel
{
    int32_t gainMapMinNumerator{0};
    uint32_t gainMapMinDenominator{1};
    int32_t gainMapMaxNumerator{0};
    uint32_t gainMapMaxDenominator{1};
    uint32_t gammaNumerator{1};
    uint32_t gammaDenominator{1};
    int32_t baseOffsetNumerator{0};
    uint32_t baseOffsetDenominator{1};
    int32_t alternateOffsetNumerator{0};
    uint32_t alternateOffsetDenominator{1};

    GainMapChannel();
    void read(BitStream& bitstr);
    void write(BitStream& bitstr) const;
};

/// @brief Gain map metadata as defined in ISO/IEC 23008-12 AMD 1
class GainMapMetadata
{
private:
    GainMapVersion mVersion;

    bool mIsMultichannel{false};
    bool mUseBaseColourSpace{false};

    int mChannelCount;

    uint32_t mBaseHdrHeadroomNumerator{0};
    uint32_t mBaseHdrHeadroomDenominator{1};
    uint32_t mAlternateHdrHeadroomNumerator{0};
    uint32_t mAlternateHdrHeadroomDenominator{1};

    Vector<GainMapChannel> mChannels;

public:
    GainMapMetadata();
    ~GainMapMetadata() = default;

    /// @brief Parse gain map metadata from bitstream
    /// @param bitstr Input bitstream
    void read(BitStream& bitstr);

    /// @brief Write gain map metadata to bitstream
    /// @param bitstr Output bitstream
    void write(BitStream& bitstr) const;

    // Getters
    const GainMapVersion getVersion() const;
    bool isMultichannel() const;
    bool useBaseColourSpace() const;
    uint32_t getBaseHdrHeadroomNumerator() const;
    uint32_t getBaseHdrHeadroomDenominator() const;
    uint32_t getAlternateHdrHeadroomNumerator() const;
    uint32_t getAlternateHdrHeadroomDenominator() const;
    const Vector<GainMapChannel>& getChannels() const;

    // Setters
    void setVersion(const GainMapVersion& version);
    void setIsMultichannel(bool isMultichannel);
    void setUseBaseColourSpace(bool useBaseColourSpace);
    void setBaseHdrHeadroom(uint32_t numerator, uint32_t denominator);
    void setAlternateHdrHeadroom(uint32_t numerator, uint32_t denominator);
    void addChannel(const GainMapChannel& channel);
    void clearChannels();
};




/// @brief Class for holding tonemap item data. This is an item, and not a box.
/// @details As this is metadata and necessary for HDR applications, parsing here is needed.
/// @note Current existing implementations might not understand gainmap metadata, so this can expose the data for usage.
class ToneMapImage
{
private:
    uint8_t mVersion{0};  ///< shall be equal to 0 as of ISO/IEC 23008-12:2025/AMD 1:2025
    GainMapMetadata mGainMapMetadata;  ///< contains the gain map metadata

public:
    ToneMapImage();
    ~ToneMapImage() = default;

    /// @brief Parse tone map image data from bitstream
    /// @param bitstr Input bitstream containing the tmap item data
    void read(BitStream& bitstr);

    /// @brief Write tone map image data to bitstream
    /// @param bitstr Output bitstream
    void write(BitStream& bitstr) const;

    // Getters
    uint8_t getVersion() const;
    const GainMapMetadata& getGainMapMetadata() const;
    GainMapMetadata& getGainMapMetadata();

    // Setters
    void setVersion(uint8_t version);
    void setGainMapMetadata(const GainMapMetadata& metadata);
};

#endif
