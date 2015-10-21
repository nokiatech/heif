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

#ifndef TRACKALTREPWRITER_HPP
#define TRACKALTREPWRITER_HPP

#include "trackwriter.hpp"
#include <memory>
#include <string>

/**
 * @brief Track Alternative Representation writer
 * @details The class is used to write a TrackBox with 'vide' handler type, to accompany a TrackBox with 'pict' handler.
 * @todo Here is a lot similarity to TracKMasterWriter. Possibly these could be refactored to one class in some point.
 */
class TrackAltrepWriter : public TrackWriter
{
public:
    /** TrackAltrepWriter input configuration structure */
    struct Altrep
    {
        std::string file_path;   ///< Input bitstream file name
        std::string hdlr_type;   ///< Handler type ('vide', 'pict')
        std::string code_type;   ///< Coding type, should be here 'hvc1'
        std::string encp_type;   ///< Encapsulation type, should be he here 'trak'
        std::uint32_t disp_xdim; ///< Display width
        std::uint32_t disp_ydim; ///< Display height
        std::uint32_t disp_rate; ///< Frames per second
        std::uint32_t tick_rate; ///< Tick rate
        IsoMediaFile::EditList edit_list; ///< EditList configuration
    };

    /**
     * Constructor of TrackAltrepWriter
     * @param trackId Track/context ID of this TrackBox
     * @param config Input configuration */
    TrackAltrepWriter(const std::uint32_t trackId, const Altrep& config);
    virtual ~TrackAltrepWriter() = default;

    /** @brief Create a TrackBox based on configuration data given during writer construction.
     *  @see TrackWriter::writeTrack() */
    virtual std::unique_ptr<TrackBox> writeTrack();

private:
    Altrep mConfig;   ///< Configuration of this Altrep writer track

    void initWrite(); ///< Initialize the base class for writing operation
};

#endif /* end of include guard: TRACKALTREPWRITER_HPP */

