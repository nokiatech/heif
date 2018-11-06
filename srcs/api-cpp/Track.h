/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <Heif.h>

namespace HEIFPP
{
    class Track;
    class VideoTrack;
    class Sample;
    class AudioSample;
    class VideoSample;
    class CleanApertureProperty;
    class AuxProperty;
    class AlternativeTrackGroup;

    /** @brief Track abstraction*/
    class Track
    {
        friend class VideoTrack;
        friend class ImageSequence;
        friend class EntityGroup;
        friend class Heif;
        friend class Sample;

    public:
        virtual ~Track();

        bool isMasterImageSequence()
            const;  ///< Track handler type is 'pict', and the track is not referencing any another track.
        bool isThumbnailImageSequence() const;  ///< Is referencing another track with a 'thmb' type track reference.
        bool isAuxiliaryImageSequence() const;  ///< Is referencing another track with an 'auxl' type track reference.
        bool isEnabled() const;                 ///< The track is enabled.
        bool isInMovie() const;                 ///< The track is used in the presentation.
        bool isInPreview() const;               ///< The track is used when previewing the presentation.
        bool hasAlternatives() const;           ///< The track has alternative track or tracks.
        bool hasCodingConstraints() const;      ///< From Coding Constraints Box in HevcSampleEntry
        bool hasSampleGroups() const;           ///< The track has SampleToGroupBoxes.
        bool hasLinkedAuxiliaryImageSequence() const;  ///< There is a 'auxl' track reference pointing to this track.
        bool hasLinkedThumbnailImageSequence() const;  ///< There is a 'thmb' track reference pointing to this track.
        bool hasSampleToItemGrouping() const;  ///< The track has one or more SampleToMetadataItemEntry ('stmi') sample
                                               ///< groups present.
        bool hasExifSampleEntry() const;            ///< From SampleEntryBox, not implemented yet
        bool hasXmlSampleEntry() const;             ///< From SampleEntryBox, not implemented yet
        bool hasEditList() const;                   ///< Track has an edit list
        bool hasInfiniteLoopPlayback() const;       ///< Infinite looping has been set on in the edit list
        bool hasSampleEquivalenceGrouping() const;  ///< The track has one or more VisualSampleGroupEntry ('eqiv')
                                                    ///< sample group entries present.
        bool isAudioTrack() const;       ///< Track is an audio track (handler type 'soun').
        bool isVideoTrack() const;       ///< Track is a video track (handler type 'vide').
        bool isImageSequence() const;    ///< Track is a video track (handler type 'pict').
        bool displayAllSamples() const;  ///< Edit List presentation indicates 0 or 1 samples. The player should ignore
                                         ///< timestamps and display all non-hidden samples.

        const HEIF::FourCC& getHandler() const;

        /** Return count of thumbnails associated with this track. */
        std::uint32_t getThumbnailCount() const;

        /** Gets the thumbnail with the given index.
         * @param [in] aIndex: Index of the thumbnail. */
        Track* getThumbnail(uint32_t aIndex);
        const Track* getThumbnail(uint32_t aIndex) const;
        void addThumbnail(Track* aThumb);
        void removeThumbnail(Track*);


        std::uint32_t getAuxCount() const;
        Track* getAux(uint32_t aIndex);
        const Track* getAux(uint32_t aIndex) const;
        void addAux(Track* aAux);
        void removeAux(Track*);

        std::uint32_t getReferenceCount() const;
        const std::pair<const HEIF::FourCC, const Track*> getReference(std::uint32_t) const;
        const std::pair<const HEIF::FourCC, Track*> getReference(std::uint32_t);

        std::uint32_t getReferenceTypeCount() const;
        const HEIF::FourCC getReferenceType(std::uint32_t) const;
        HEIF::FourCC getReferenceType(std::uint32_t);
        std::uint32_t getReferenceCount(const HEIF::FourCC& aType) const;
        const Track* getReference(const HEIF::FourCC& aType, std::uint32_t) const;
        Track* getReference(const HEIF::FourCC& aType, std::uint32_t);

        std::uint32_t getAlternativeTrackCount() const;
        Track* getAlternativeTrack(std::uint32_t aId);
        const Track* getAlternativeTrack(std::uint32_t aId) const;

        void setAlternativeTrackGroup(AlternativeTrackGroup*);
        AlternativeTrackGroup* getAlternativeTrackGroup();
        const AlternativeTrackGroup* getAlternativeTrackGroup() const;

        std::uint64_t getMaxSampleSize();


        std::uint32_t getSampleCount() const;
        Sample* getSample(std::uint32_t);
        Sample* getSample(std::uint32_t) const;
        Sample* getSampleByType(HEIF::TrackSampleType, std::uint32_t);
        Sample* getSampleByType(HEIF::TrackSampleType, std::uint32_t) const;
        void removeSample(Sample* aSample);


        double getDuration();
        std::uint32_t getTimescale();
        void setTimescale(std::uint32_t);

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns a pointer to the custom user data. */
        const void* getContext() const;

        const HEIF::SequenceId& getId() const;

        /** Returns the parent HEIF object of the item */
        Heif* getHeif();
        const Heif* getHeif() const;

        // groups
        std::uint32_t getGroupCount() const;
        EntityGroup* getGroup(uint32_t aId);

        std::uint32_t getGroupByTypeCount(const HEIF::FourCC& aType);
        EntityGroup* getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId);
        EntityGroup* getGroupById(const HEIF::GroupId& aId);

        // TODO: add methods to access/edit/set the editlist.

    protected:
        // NOTE: moved add/removeReference to protected. since custom references are not possible ATM.
        void addReference(const HEIF::FourCC& aType, Track* aTrack);
        void removeReference(const HEIF::FourCC& aType, Track* aTrack);

        void addToGroup(EntityGroup* aGroup);
        void removeFromGroup(EntityGroup* aGroup);
        void addSample(Sample* aSample);
        void setSample(std::uint32_t, Sample* aSample);
        void setSample(Sample* aOldSample, Sample* aNewSample);

        // serialization methods.
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::SequenceId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void setId(const HEIF::SequenceId&);

        Track(Heif* aHeif);
        Heif* mHeif;
        HEIF::SequenceId mId;
        const void* mContext;

        HEIF::FeatureBitMask mFeatures;
        double mDuration;
        std::uint32_t mTimeScale;
        std::uint64_t mMaxSampleSize;
        std::vector<Track*> mAux;
        std::vector<Track*> mThumbnail;
        std::map<HEIF::FourCC, std::vector<Track*>> mRefs;
        std::map<HEIF::FourCC, LinkArray<Track*>> mIsRefTo;

        AlternativeTrackGroup* mAltGroup;
        LinkArray<Track*> mIsThumbnailTo;
        LinkArray<Track*> mIsAuxiliaryTo;
        std::vector<Sample*> mSamples;
        std::vector<EntityGroup*> mGroups;
        class EditList
        {
        public:
            bool mLooping       = false;
            double mRepetitions = 0.0;
            std::vector<HEIF::EditUnit> mEditUnits;
        };
        EditList mEditList;

        HEIF::FourCC mHandler;  // soun,vide,pict
    private:
        Track& operator=(const Track&) = delete;
        Track& operator=(Track&&)      = delete;
        Track(const Track&)            = delete;
        Track(Track&&)                 = delete;
        Track()                        = delete;
    };

}  // namespace HEIFPP
