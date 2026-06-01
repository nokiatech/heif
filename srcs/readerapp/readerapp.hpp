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

#ifndef READERAPP_HPP
#define READERAPP_HPP

#include <set>

#include "customallocator.hpp"
#include "heifcommondatatypes.h"
#include "heifreader.h"
#include "heifreaderdatatypes.h"
#include <string>
#include <vector>

class ReaderApp
{
public:
    ReaderApp(int argc, char* argv[]);
    void processFile();
    void printHelp();

private:
    bool handleArguments();
    std::string findFilename() const;
    void handleRootMeta(HEIF::Reader* fileReader);
    void handleTracks(HEIF::Reader* fileReader);

    void writeCollectionBitstreams(HEIF::Reader* fileReader,
                                   const std::string& fileName,
                                   const Vector<HEIF::ImageId>& imageIds,
                                   bool createMultipleStreams);

    /// Detect groups of VVC items that belong to the same multi-layer VVC
    /// stream (i.e. share a vvcC with multi_layer_flag=1) and write each
    /// such group out as a single combined Annex-B '.h266' file containing:
    ///   1) the full all-layers parameter set bundle (VPS + per-layer
    ///      SPS/PPS + APS), followed by
    ///   2) the IDR access units of every layer item, ordered by ascending
    ///      nuh_layer_id, so that the resulting bitstream is decodable as
    ///      one multi-layer access unit (the consuming decoder is then told
    ///      which layer to actually output via its own command-line flag).
    /// @param fileReader   Initialized reader.
    /// @param baseFileName Output file base (e.g. "multilayer"); the suffix
    ///                     "_multilayer.h266" is appended per group.
    /// @param imageIds     All root-meta item ids to consider.
    /// @return Set of item ids that were emitted as part of a combined
    ///         multilayer stream; the caller should exclude these from any
    ///         per-feature single-item bitstream pass to avoid duplicate
    ///         emission of master.h266 / auxs.h266 alongside multilayer.h266.
    std::set<HEIF::ImageId> writeMultiLayerVvcStreams(HEIF::Reader* fileReader,
                                                      const std::string& baseFileName,
                                                      const Vector<HEIF::ImageId>& imageIds);


    void writeBitStreamFile(HEIF::Reader* fileReader,
                            const std::string& fileName,
                            const HEIF::FourCC& codeType,
                            const Vector<HEIF::ImageId>& imageIds,
                            bool createMultipleStreams);

    void writeBitStreamFile(HEIF::Reader* fileReader,
                            const std::string& fileName,
                            const HEIF::FourCC& codeType,
                            const HEIF::SequenceId contextId,
                            const Vector<HEIF::SequenceImageId>& imageIds,
                            bool createMultipleStreams);
    void printSampleInformation(const HEIF::TrackInformation& trackInformation, HEIF::Reader* fileReader) const;
    void dumpFileInfo(HEIF::Reader* reader);

    bool mMultipleOutputFiles;
    bool mWriteMasterOnly;
    bool mDumpMode;
    bool mDumpOnlyMode;
    std::string mExecutablePath;
    std::vector<std::string> mParameters;
};

bool compareSets(const HEIF::Array<HEIF::DecoderSpecificInfo>& a, const HEIF::Array<HEIF::DecoderSpecificInfo>& b);

#endif
