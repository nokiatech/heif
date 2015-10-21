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

#ifndef TRACKMASTERWRITER_HPP
#define TRACKMASTERWRITER_HPP

#include "isomediafile.hpp"
#include "trackwriter.hpp"

#include <memory>

/**
 * @brief Implements TrackMasterWriter class.
 */
class TrackMasterWriter : public TrackWriter
{
public:
    /** @brief The constructor for TrackMasterWriter
     *  @param trackId ID of the track to be written
     *  @param config  Writer input configuration for this track */
    TrackMasterWriter(const std::uint32_t trackId, const IsoMediaFile::Master& config);
    virtual ~TrackMasterWriter() = default;

    /** @see TrackWriter::writeTrack */
    virtual std::unique_ptr<TrackBox> writeTrack();

private:
    IsoMediaFile::Master mConfig; ///< Configuration relevant to this thumb writer track

    /// Initialize base class for writing
    void initWrite();

    /// Register DataStore of the this TrackWriter
    void initStore();
};

#endif /* end of include guard: TRACKMASTERWRITER_HPP */

