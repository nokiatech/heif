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

#ifndef TRACKWRITER_HPP
#define TRACKWRITER_HPP

#include "hevcsampleentry.hpp"
#include "isomediafile.hpp"
#include "trackbox.hpp"

#include <cstdint>
#include <memory>
#include <string>

class CodingConstraintsBox;
class DataStore;

/**
 * @brief Declares abstract base class for track writers.
 *
 * @details This class is the base class for derived classes such as
 * TrackMasterWriter, TrackThumbsWriter, and TrackAltrepWriter.
 */
class TrackWriter
{
public:
    /**
     * @brief Constructor for the TrackWriter class.
     * @param trackId  A positive 32 bit integer that is to be used as a
     *                 trackId for this track.
     * @param editList A structure that contains the edit lists for this
     *                 track. If a track has no edits, this structure contains and empty vector.
     * @param filename A string that provides the complete path to the
     *                 encoded bitstream that is to be encapsulated by this TrackWriter.
     * @param tickRate How many time units pass during one second (timescale)
     */
    TrackWriter(std::uint32_t trackId, const IsoMediaFile::EditList& editList, const std::string& filename,
        std::uint32_t tickRate);

    /**
     * @brief The virtual default destructor
     */
    virtual ~TrackWriter() = default;

    /**
     * @brief Fill TrackBox structures in a way specific to the derived TrackWriter class,
     *        and return the filled TrackBox.
     *
     * @return Filled TrackBox with unique ownership.
     */
    virtual std::unique_ptr<TrackBox> writeTrack() = 0;

    /**
     * @brief This method sets an internal variable that is used to set the
     *        track_enabled bit in the flags parameter of the TrackHeaderBox.
     *
     * @param value A boolean value; when value is true/1, this track is  set as enabled;
     *              when value is set to false/0, this track is set as not enabled.
     */
    void setTrackEnabled(bool value);

    /**
     * @brief This method sets an internal variable that is used to set the
     *        track_in_movie bit in the flags paramter of the TrackHeaderBox.
     *
     * @param value A boolean value, when value is true/1, this track is
     *              set to used in the movie presentation; when value is set to false/0, this
     *              track is set as not to be used in the movie presentation.
     */
    void setDisplayTrack(bool value);

    /**
     * @brief This method sets an internal variable that is used to set the
     *        track_in_preview bit in the flags parameter of the TrackHeaderBox.
     *
     * @param value A boolean value, when value is true/1, this track is
     *              is set as a preview track (e.g. thumbnail track); when value is set to
     *              false/0, this track is set as not used for preview.
     */
    void setPreviewTrack(bool value);

    /**
     * @brief This method sets an internal variable that is used to set the
     *        alternate_group field in the TrackHeaderBox.
     *
     * @param value A 32 bit unsigned integer value that is the alternate group identifier
     *              for all tracks that are marked as alternates of this track
     */
    void setTrackAsAlter(std::uint32_t value);

protected:
    /**
     * @brief Move a ready TrackBox out from the TrackWriter. TrackWriter will
     * internally create a new TrackBox for the next write.
     *
     * @return A filled TrackBox.
     */
    std::unique_ptr<TrackBox> finalizeWriting();

    /** @brief Fill TrackBox structures which are common for all TrackWriters. */
    void writeTrackCommon();

    /**
     * @brief This method is used to store a key-value pair in its internal store.
     * @param key   A string that is used as the key for identifying the value associated with it.
     * @param value A string that holds the value in the key-value pair.
     */
    void storeValue(const std::string& key, const std::string& value);

    /**
     * @brief Instantiate a DataStore for the track, registering it to its track ID, and register it to DataServe to
     *        make it available globally. */
    void registerDataStore();

    /**
     * @brief This method handles the clearing of all internal variables.
     */
    void clearVars();

    /**
     * @brief This method fills in the fields of the DataReferenceBox.
     */
    void drefWrite();

    /**
     * @brief This method fills in the fields of the HandlerBox.
     * @param handlerType A string that is a valid fourCC handler value.
     */
    void hdlrWrite(const std::string& handlerType);

    /**
     * @brief This method fills in the fields of the MediaHeaderBox.
     */
    void mdhdWrite();

    /**
     * @brief This method handles the filling up of the various sample groupings associated with this track.
     */
    void sgrpWrite();

    /**
     * @brief This method fills in the fields of the ChunkOffsetBox.
     */
    void stcoWrite();

    /**
     * @brief This method fills in the fields of the SampleToChunkBox.
     */
    void stscWrite();

    /**
     * @brief Fills in the fields of the SampleDescriptionBox.
     */
    void stsdWrite();

    /**
     * @brief Fills in the fields of the SampleDescriptionBox with a 'ccst' box.
     * @param ccst CodingConstraintsBox to be written into the SampleDescriptionBox.
     */
    void stsdWrite(const IsoMediaFile::CodingConstraints& ccst);

    /**
     * @brief This method fills in the fields of the SampleDescriptionBox. Coding Constraints Box
     *        will not be written.
     * @param codeType Configured code_type
     */
    void stsdWrite(const std::string& codeType);

    /**
     * @brief Get HEVC SampleEntry box
     */
    std::unique_ptr<HevcSampleEntry> getHevcSampleEntry();

    /**
     * @brief Get HEVC SampleEntry box with a filled Coding Constraints Box
     * @param ccst CodingConstraintsBox is to be written into the SampleDescriptionBox.
     */
    std::unique_ptr<SampleEntryBox> getHevcSampleEntry(const IsoMediaFile::CodingConstraints& ccst);

    /**
     * @brief This method fills in the fields of the SyncSampleTableBox.
     */
    void stssWrite();

    /**
     * @brief This method fills in the fields of the SampleSizesBox
     */
    void stszWrite();

    /**
     * @brief This method handles the filling up of fields of all time related
     *        boxes (stss, ctts and cslg boxes).
     */
    void timeWrite();

    /**
     * @brief This method handles the writing of the editlist box.
     */
    void editWrite();

    /**
     * @brief This method handles the flattening of the sample presentation
     *        timings considering all the edits in the editlist.
     */
    void decodePts();

    /**
     * @brief This method handles the writing of the TrackHeaderBox.
     */
    void tkhdWrite();

    /**
     * @brief This method computes the start code size of NAL units in a NAL based media track.
     * @param nalU A vector of bytes contained in one NAL unit.
     * @return The size in bytes of the given NAL unit header.
     */
    uint8_t getNalStartCodeSize(const std::vector<uint8_t>& nalU) const;

    /**
     * @brief This method handles the parsing of the input bitstream to analyse
     * the structure of access units.
     */
    void bstrParse();

    /**
     * @brief This method is used to set the display width of the visual samples described in this track.
     *
     * @param width A positive 32 bit integer that indicates the width in
     * pixels of the visual sample.
     */
    void setDisplayWidth(std::uint32_t width);

    /**
     * @brief This method is used to set the display height of the visual samples described in this track.
     *
     * @param height A positive 32 bit integer that indicates the height in
     *               pixels of the visual sample.
     */
    void setDisplayHeight(std::uint32_t height);

    /**
     * @brief This method is used to set the display rate of the visual samples described in this track.
     *
     * @param rate A positive 32 bit integer that indicates the rate at
     *             which the visual samples are displayed.
     */
    void setDisplayRate(std::uint32_t rate);

    /**
     * @brief This method is used to set the number of time units that pass in one second.
     *
     * @param clockticks A positive 32 bit integer that indicates the
     *                   number of timeunits counted in one second of displayed media.
     */
    void setClockTicks(std::uint32_t clockticks);

    /**
     * @brief This method returns the trackId used by this track
     * @return A positive 32 bit integer that is the trackId as written by this track in its TrackHeaderBox.
     */
    std::uint32_t getTrackId() const;

    /**
     * @brief This method returns a reference to the TrackHeaderBox that is process by this TrackWriter object.
     * @return An instance of the TrackBox object that is processed by this track writer.
     */
    TrackBox* getTrackBox();

private:
    std::unique_ptr<TrackBox> mTrackBox;                    /**< An internal object of type TrackBox */

    std::uint32_t mTrackId;                                 /**< Track Id */
    std::uint32_t mAlterId;                                 /**< Alternate Id */

    bool mDisplayTrack;                                     /**< Is track used in the movie presentation */
    bool mPreviewTrack;                                     /**< Is track used in preview mode */
    bool mTrackEnabled;                                     /**< Is track enabled for presentation */
    bool mTrackIsAlter;                                     /**< Is track an alternate of another track */

    std::vector<std::uint32_t> mDisplayTimes;               /**< A vector of display time for samples */

    std::vector<std::uint8_t> mVpsNals;                     /**< A vector of Video Parameter Set NAL units. */
    std::vector<std::uint8_t> mSpsNals;                     /**< A vector of Sequence Parameter Set NAL units. */
    std::vector<std::uint8_t> mPpsNals;                     /**< A vector of Picture Parameter Set NAL units. */

    std::vector<std::vector<std::uint32_t>> mRefsList;      /**< A vector that holds the direct reference list for each access unit */

    bool mHasPred;                                          /**< Flag set when predicted pictures are found */

    std::shared_ptr<DataStore> mInternalStore;              /**< An internal store registered by this track writer */
    std::uint64_t mDuration;                                /**< Presentation duration of the media in this track (considering edit lists) */
    std::uint32_t mDisplayWidth;                            /**< The display width */
    std::uint32_t mDisplayHeight;                           /**< The display height */
    std::uint32_t mDisplayRate;                             /**< The display frame rate */
    std::uint32_t mClockTicks;                              /**< Number of time units that passes in once second of presented media */

    /**
     * @brief Documents AccessUnit information as obtained from the parser.
     */
    struct AccessUnitInfo
    {
        std::uint32_t mOffset;                              /**< Offset of AU in the bitstream */
        std::uint32_t mLength;                              /**< AU length in bytes */
    };
    std::vector<AccessUnitInfo> mAccessUnitVector;          /**< A vector of AccessUnitInfo */

    std::vector<std::uint32_t> mDecodeOrder;                /**< A vector that hold the decoding order of access units in this track */
    std::vector<std::uint32_t> mDisplayOrder;               /**< A vector that hold the display order of access units in this track */
    std::vector<bool> mIsSync;                              /**< A boolean vector whose elements indicate if the access unit is a sync sample or not */

    std::string mFilename;                                  /** The filename of the bitstream that is encapsulated in this track */
    IsoMediaFile::EditList mEditlist;                       /** The edit list structure that is passed to the writer from the interface */

    void fillCcst(CodingConstraintsBox* ccst, const IsoMediaFile::CodingConstraints& pCcst);
};

#endif /* end of include guard: TRACKWRITER_HPP */

