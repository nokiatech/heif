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

// Worked examples for the HEIFPP (heif::heifpp) C++ wrapper API.
//
// Read demos take a HEIF file path on argv[1] (default: C003.heic).
// Write demos take an output path on argv[2] and use placeholder bytes
// -- the file structure is valid but the bitstream will not decode.
// Replace the placeholder buffers with output from a real HEVC encoder
// for production use.
//
// See also:
//   - srcs/examples/example.cpp                       -- raw HEIF::Reader / HEIF::Writer
//   - srcs/api-java/.../HEIFExample.java              -- same demos via JNI/HEIFPP

// Standard library
#include <cstdint>
#include <iostream>
#include <vector>

// HEIFPP root, image items, and grid derived item
#include "GridImageItem.h"
#include "HEVCCodedImageItem.h"
#include "Heif.h"

// Decoder configuration + descriptive properties (pixi/colr definitions)
#include "DescriptiveProperty.h"
#include "HEVCDecoderConfiguration.h"

// Error-model surfaces (sink + slot for HEIFPP, stderr sink for the C tier)
#include "ErrorContext.h"
#include "heiferrorinfo.h"

// Common HEIF value types (FourCC, ImageId, ...)
#include "heifcommondatatypes.h"
#include "CodedImageItem.h"
#include "ErrorCodes.h"
#include "ImageItem.h"
#include <stdio.h>
#include <string>


// Demo 1 -- open a file and look at every master image.
static void example1_loadAndIterate(const char* path)
{
    std::cout << "[demo 1] load + iterate master images\n";

    // Heif is the single owning root. Everything created or loaded under
    // it is destroyed when it goes out of scope -- no manual cleanup.
    HEIFPP::Heif heif;
    if (heif.load(path) != HEIFPP::Result::OK)
    {
        std::cout << "  skipped: could not open " << path << "\n";
        return;
    }

    // Master images are the top-level still images of the file.
    // Auxiliary, thumbnail and tile items are reachable from these
    // but are not enumerated here.
    for (std::uint32_t i = 0; i < heif.getMasterImageCount(); ++i)
    {
        HEIFPP::ImageItem* img = heif.getMasterImage(i);

        if (img->isCodedImage())
        {
            // Coded image = an actual encoded bitstream (HEVC/AVC/JPEG).
            // In your app you would hand the buffer below to your decoder.
            auto* coded = static_cast<HEIFPP::CodedImageItem*>(img);
            (void) coded->getItemData();             // forces lazy load
            // coded->getItemDataSize()              // payload size in bytes
            // coded->getDecoderConfiguration()      // SPS/PPS/VPS for the decoder
        }
        else if (img->isDerivedImage())
        {
            // Derived image = grid, overlay, identity, iden-transform.
            // dynamic_cast to the concrete subclass (see demo 3).
        }
    }
    // ~Heif() destroys every owned Item / Property / DecoderConfig here.
}


// Demo 2 -- inspect the primary image and its descriptive properties.
// Properties answer "how do I render this image?" (channel depths,
// colour space, rotation/mirroring, ...).
static void example2_primaryAndProperties(const char* path)
{
    std::cout << "[demo 2] primary image + properties\n";

    HEIFPP::Heif heif;
    if (heif.load(path) != HEIFPP::Result::OK)
    {
        std::cout << "  skipped: could not open " << path << "\n";
        return;
    }

    // The primary item is the image a viewer should show first.
    const HEIFPP::ImageItem* primary = heif.getPrimaryItem();
    if (primary == nullptr) return;

    // Typed accessors return nullptr if the property is not present
    // -- no need to enumerate properties and switch on FourCC.
    if (const auto* pixi = primary->pixelInformation(); pixi != nullptr)
    {
        // Bits-per-channel array; feed into your render pipeline.
        (void) pixi->mPixelInformation.bitsPerChannel;
    }
    if (const auto* colr = primary->colourInformation(); colr != nullptr)
    {
        // Colour space descriptor (nclx / rICC / prof). Hand to your
        // colour management layer.
        (void) colr->mColourInformation.colourType;
    }

    // Counts of property/related-item categories the integrator may
    // want to walk separately:
    //   transformativePropertyCount() -> IROT/IMIR/CLAP rotations etc.
    //   getThumbnailCount()           -> linked thumbnail items
    (void) primary->transformativePropertyCount();
    (void) primary->getThumbnailCount();
}


// Demo 3 -- if the primary image is a Grid (tiled), walk its tiles.
static void example3_loadGrid(const char* path)
{
    std::cout << "[demo 3] derived (Grid) image walk\n";

    HEIFPP::Heif heif;
    if (heif.load(path) != HEIFPP::Result::OK)
    {
        std::cout << "  skipped: could not open " << path << "\n";
        return;
    }

    // GridImageItem is one of several DerivedImageItem subclasses.
    // dynamic_cast returns nullptr if the primary is not a grid.
    auto* grid = dynamic_cast<HEIFPP::GridImageItem*>(heif.getPrimaryItem());
    if (grid == nullptr)
    {
        std::cout << "  skipped: primary is not a grid\n";
        return;
    }

    // Tiles are addressed by (column, row). Each tile is itself an
    // ImageItem -- typically a coded HEVC item -- ready to be decoded.
    for (std::uint32_t r = 0; r < grid->rows(); ++r)
    {
        for (std::uint32_t c = 0; c < grid->columns(); ++c)
        {
            HEIFPP::ImageItem* tile = nullptr;
            if (grid->getImage(c, r, tile) != HEIFPP::Result::OK) continue;
            // Hand `tile` (cast to CodedImageItem) to your decoder, then
            // composite the result at offset (c * tileW, r * tileH).
        }
    }
}


// Demo 4 -- write a single-image HEIC.
// Placeholder bytes are used; the produced file will not decode. In a
// real integration the two buffers below come from your HEVC encoder.
static void example4_writeSimple(const char* outFile)
{
    std::cout << "[demo 4] write a single-image HEIC (placeholder bytes)\n";

    constexpr std::uint32_t width = 640, height = 480;
    std::vector<std::uint8_t> decoderConfigData(1024, 0);  // SPS/PPS/VPS goes here
    std::vector<std::uint8_t> imageData(50000, 0);         // encoded HEVC NAL units

    HEIFPP::Heif heif;

    // All HEIFPP items are constructed via heif.create<T>(). Constructors
    // are protected so this is the only legal path; ownership stays
    // with `heif` (no manual delete).
    auto* config = heif.create<HEIFPP::HEVCDecoderConfiguration>();
    (void) config->setConfig(decoderConfigData.data(),
                             static_cast<std::uint32_t>(decoderConfigData.size()));

    // Create the image item, set its dimensions, attach the decoder
    // config, and feed it the encoded bitstream.
    auto* item = heif.create<HEIFPP::HEVCCodedImageItem>();
    item->setSize(width, height);
    (void) item->setDecoderConfiguration(config);
    item->setItemData(imageData.data(), imageData.size());

    // File-level metadata: which item viewers should show first, plus
    // the brand declarations a HEIF reader uses to recognise the file.
    heif.setPrimaryItem(item);
    heif.setMajorBrand(HEIF::FourCC("mif1"));
    heif.addCompatibleBrand(HEIF::FourCC("heic"));

    HEIFPP::Result rc = heif.save(outFile);
    std::cout << "  save -> code " << static_cast<int>(rc)
              << " (failure expected with placeholder bytes)\n";
}


// Demo 5 -- write a 3x2 grid HEIC. Same placeholder caveat as demo 4.
static void example5_writeGrid(const char* outFile)
{
    std::cout << "[demo 5] write a 3x2 Grid HEIC (placeholder bytes)\n";

    constexpr std::uint32_t cols = 3, rows = 2, tileW = 640, tileH = 480;
    std::vector<std::uint8_t> decoderConfigData(1024, 0);
    std::vector<std::uint8_t> imageData(50000, 0);

    HEIFPP::Heif heif;

    // Create the grid container and declare the assembled output size.
    auto* grid = heif.create<HEIFPP::GridImageItem>(cols, rows);
    grid->setSize(cols * tileW, rows * tileH);

    // One decoder config can be shared across all tiles when they were
    // encoded together (typical case for a tiled HEIC output).
    auto* config = heif.create<HEIFPP::HEVCDecoderConfiguration>();
    (void) config->setConfig(decoderConfigData.data(),
                             static_cast<std::uint32_t>(decoderConfigData.size()));

    // Each tile is its own coded item; the grid owns the (col, row) map.
    for (std::uint32_t r = 0; r < rows; ++r)
    {
        for (std::uint32_t c = 0; c < cols; ++c)
        {
            auto* tile = heif.create<HEIFPP::HEVCCodedImageItem>();
            tile->setSize(tileW, tileH);
            (void) tile->setDecoderConfiguration(config);
            tile->setItemData(imageData.data(), imageData.size());
            (void) grid->setImage(c, r, tile);
        }
    }

    heif.setPrimaryItem(grid);
    heif.setMajorBrand(HEIF::FourCC("mif1"));
    heif.addCompatibleBrand(HEIF::FourCC("heic"));

    HEIFPP::Result rc = heif.save(outFile);
    std::cout << "  save -> code " << static_cast<int>(rc)
              << " (failure expected with placeholder bytes)\n";
}


// Demo 6 -- HEIFPP error model.
// Two complementary surfaces:
//   * setErrorSink() : a process-wide callback fired on every failure
//                      (use it to forward into your logger).
//   * getLastError() : a thread-local slot you can poll right after
//                      a call that returned non-OK, with structured
//                      fields (code, source, file, line, cause).
static void example6_errorHandling()
{
    std::cout << "[demo 6] error sink + thread-local slot\n";

    // Install a sink that forwards into your logger. The lambda runs
    // on the thread that produced the error.
    HEIFPP::setErrorSink([](const HEIFPP::ErrorContext& ctx) {
        std::cout << "  sink: " << HEIFPP::formatErrorContext(ctx) << "\n";
    });

    // Provoke a failure: open a file that does not exist.
    HEIFPP::Heif heif;
    HEIFPP::clearLastError();
    HEIFPP::Result rc = heif.load("/tmp/__heifpp_example_missing__.heic");
    (void) rc;

    // After any non-OK return, the slot carries structured detail
    // about *what* failed and *where*. formatErrorContext() prints
    // code, source, file:line, message, and (when present) cause.
    if (HEIFPP::hasLastError())
    {
        std::cout << "  slot: " << HEIFPP::formatErrorContext(HEIFPP::getLastError()) << "\n";
    }

    // Tidy up the process-wide sink at shutdown.
    HEIFPP::setErrorSink(nullptr);
    HEIFPP::clearLastError();
}


int main(int argc, char** argv)
{
    // Route C-tier diagnostics to stderr. One-line install at startup;
    // every C-tier failure surfaces automatically (see wiki page 03).
    HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);

    const char* inFile  = (argc >= 2) ? argv[1] : "C003.heic";
    const char* outFile = (argc >= 3) ? argv[2] : "heifpp_example_out.heic";

    example1_loadAndIterate(inFile);
    example2_primaryAndProperties(inFile);
    example3_loadGrid(inFile);
    example4_writeSimple(outFile);
    example5_writeGrid(outFile);
    example6_errorHandling();

    return 0;
}
