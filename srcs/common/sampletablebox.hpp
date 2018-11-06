/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef SAMPLETABLEBOX_HPP
#define SAMPLETABLEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "chunkoffsetbox.hpp"
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "customallocator.hpp"
#include "sampledescriptionbox.hpp"
#include "samplegroupdescriptionbox.hpp"
#include "samplesizebox.hpp"
#include "sampletochunkbox.hpp"
#include "sampletogroupbox.hpp"
#include "syncsamplebox.hpp"
#include "timetosamplebox.hpp"


/**
 * \class SampleTableBox
 * @brief Implements the SampleTableBox which is inherited from Box.
 * @details The SampleTableBox (4CC: "stbl") contains all data that pertains to
 * describing, locating, and timing of the media samples in the track. */
class SampleTableBox : public Box
{
public:
    /** @brief The constructor for SampleTableBox.
     *  @details - Handles initialization of member variables.
     */
    SampleTableBox();

    /** @brief The destructor for SampleTableBox.
     *  @details - Currently does nothing but is defined due to polymorphism.
     */
    virtual ~SampleTableBox() = default;

    SampleTableBox(const SampleTableBox& obj) = delete;

    /** @brief Provides access to a SampleDescriptionBox.
     *  @details - An instance of the SampleDescriptionBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleDescriptionBox.
     */
    SampleDescriptionBox& getSampleDescriptionBox();

    /** @brief Provides access to a SampleDescriptionBox.
     *  @details - An instance of the SampleDescriptionBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleDescriptionBox.
     */
    const SampleDescriptionBox& getSampleDescriptionBox() const;

    /** @brief Provides access to a TimeToSampleBox.
     *  @details - An instance of the TimeToSampleBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated TimeToSampleBox.
     */
    TimeToSampleBox& getTimeToSampleBox();

    /** @brief Provides access to a TimeToSampleBox.
     *  @details - An instance of the TimeToSampleBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated TimeToSampleBox.
     */
    const TimeToSampleBox& getTimeToSampleBox() const;

    /** @brief Set CompositionOffsetBox to SampleTableBox
     */
    void setCompositionOffsetBox(const CompositionOffsetBox& compositionOffsetBox);

    /** @return shared_ptr to CompositionOffsetBox if set, or nullptr */
    std::shared_ptr<const CompositionOffsetBox> getCompositionOffsetBox() const;

    /** @brief Set CompositionToDecodeBox to SampleTableBox
     */
    void setCompositionToDecodeBox(const CompositionToDecodeBox& compositionToDecodeBox);

    /** @return shared_ptr to CompositionToDecodeBox if set, or nullptr */
    std::shared_ptr<const CompositionToDecodeBox> getCompositionToDecodeBox() const;

    /** @brief Set SyncSampleBox to SampleTableBox
     */
    void setSyncSampleBox(const SyncSampleBox& syncSampleBox);

    /** @brief Whether SampleTableBox has SyncSampleBox sub-box.
         */
    bool hasSyncSampleBox() const;

    /** @brief Provides access to a SyncSampleBox.
     *  @details - An instance of the SyncSampleBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SyncSampleBox.
     */
    std::shared_ptr<const SyncSampleBox> getSyncSampleBox() const;

    /** @brief Provides access to a SampleToChunkBox.
     *  @details - An instance of the SampleToChunkBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleToChunkBox.
     */
    SampleToChunkBox& getSampleToChunkBox();

    /** @brief Provides access to a SampleToChunkBox.
     *  @details - An instance of the SampleToChunkBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleToChunkBox.
     */
    const SampleToChunkBox& getSampleToChunkBox() const;

    /** @brief Provides access to a ChunkOffsetBox.
     *  @details - An instance of the ChunkOffsetBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated ChunkOffsetBox.
     */
    ChunkOffsetBox& getChunkOffsetBox();

    /** @brief Provides access to a ChunkOffsetBox.
     *  @details - An instance of the ChunkOffsetBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated ChunkOffsetBox.
     */
    const ChunkOffsetBox& getChunkOffsetBox() const;

    /** @brief Provides access to a SampleSizeBox.
     *  @details - An instance of the SampleSizeBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleSizeBox.
     */
    SampleSizeBox& getSampleSizeBox();

    /** @brief Provides access to a SampleSizeBox.
     *  @details - An instance of the SampleSizeBox is created upon
     *  instantiating this class. This method provides access to the locally
     *  instantiated SampleSizeBox.
     */
    const SampleSizeBox& getSampleSizeBox() const;

    /** Set SampleGroupDescriptionBox.
     */
    void addSampleGroupDescriptionBox(UniquePtr<SampleGroupDescriptionBox> sgpd);

    /** @brief Adds a new SampleToGroup into a vector.
     *  @details - This method adds a filled SampleToGroupBox into a vector
     *  held locally.
     */
    SampleToGroupBox& getSampleToGroupBox();

    /** @brief Get SampleToGroup boxes.
     *  @returns Vector of sample group boxes. */
    const Vector<SampleToGroupBox>& getSampleToGroupBoxes() const;

    /** @brief Get Sample Group Description Box of a particular grouping type.
     *  @param [in] groupingType Sample grouping type.
     *  @returns Pointer to the SampleGroupDescriptionBox with the desired grouping type. Nullptr if
     *           no Sample Group Description Box of this type was found. */
    const SampleGroupDescriptionBox* getSampleGroupDescriptionBox(FourCCInt groupingType) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SampleTableBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Purge all information related to individual samples, still keep sample descriptions */
    void resetSamples();

private:
    // Mandatory Boxes
    SampleDescriptionBox mSampleDescriptionBox;  ///< Sample description box (mandatory)
    TimeToSampleBox mTimeToSampleBox;            ///< Time-to-sample box (mandatory)
    SampleToChunkBox mSampleToChunkBox;          ///< Sample-to-chunk box (mandatory)
    ChunkOffsetBox mChunkOffsetBox;              ///< Chunk offset box (mandatory)
    SampleSizeBox mSampleSizeBox;                ///< Sample size box (mandatory)

    // Non-Mandatory Boxes
    std::shared_ptr<SyncSampleBox> mSyncSampleBox;                    ///< Sync sample box (optional)
    std::shared_ptr<CompositionOffsetBox> mCompositionOffsetBox;      ///< Composition offset box (optional)
    std::shared_ptr<CompositionToDecodeBox> mCompositionToDecodeBox;  ///< Composition to decode box (optional)

    Vector<UniquePtr<SampleGroupDescriptionBox>> mSampleGroupDescriptionBoxes;  ///< Vectory of sample group description boxes
    Vector<SampleToGroupBox> mSampleToGroupBoxes;                               ///< Vectory of sample-to-group boxes

    bool mHasSyncSampleBox;
};

#endif /* end of include guard: SAMPLETABLEBOX_HPP */
