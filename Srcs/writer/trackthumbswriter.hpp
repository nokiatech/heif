/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef TRACKTHUMBSWRITER_HPP
#define TRACKTHUMBSWRITER_HPP

#include "datastore.hpp"
#include "isomediafile.hpp"
#include "trackwriter.hpp"

#include <memory>
#include <string>

/**
 * @brief Implements TrackThumbsWriter class.
 */
class TrackThumbsWriter : public TrackWriter
{
public:
    /** @brief Constructor for TrackThumbsWriter
     *  @param trackId  ID of the track to be written
     *  @param config   Writer input configuration for this track
     *  @param masterId ID of the master track of this thumbnail track */
    TrackThumbsWriter(const std::uint32_t trackId, const IsoMediaFile::Thumbs& config, std::uint32_t masterId);
    virtual ~TrackThumbsWriter() = default;

    /** @brief Fills all relevant boxes contained within the track.
     *  @see TrackWriter::writeTrack */
    virtual std::unique_ptr<TrackBox> writeTrack();

private:
    IsoMediaFile::Thumbs mConfig;             ///< Configuration relevant to this thumb writer track
    std::shared_ptr<DataStore>  mMasterStore; ///< Thumbs track will have a link to masters store
    std::string mHandlerType;                 ///< Handler type

    /// Initialize writer before writing the track
    void initWrite();

    /// Register DataStore of the this TrackWriter
    void initStore();

    /// Write the 'thmb' track reference
    void trefWrite();
};

#endif /* end of include guard: TRACKTHUMBSWRITER_HPP */

