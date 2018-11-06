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

#include <DecoderConfiguration.h>
#include <Heif.h>
namespace HEIFPP
{
    class Track;

    class Sample
    {
        friend class EntityGroup;
        friend class Heif;
        friend class Track;
        friend class VideoTrack;
        friend class ImageSequence;
        friend class AudioTrack;

    public:
        virtual ~Sample();
        bool isVideo() const;
        bool isAudio() const;
        void setType(const HEIF::FourCC&);
        const HEIF::FourCC& getType() const;
        const HEIF::FourCC& getDecoderCodeType() const;  // exactly the same thing as getType.

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns a pointer to the custom user data. */
        const void* getContext() const;


        /** Returns the DecoderConfiguration of the image */
        DecoderConfig* getDecoderConfiguration();
        const DecoderConfig* getDecoderConfiguration() const;
        HEIF::ErrorCode setDecoderConfiguration(DecoderConfig* aConfig);

        void setSampleType(const HEIF::SampleType&);
        const HEIF::SampleType& getSampleType() const;
        const HEIF::SequenceImageId& getId() const;
        void setDuration(std::uint64_t);
        std::uint64_t getDuration() const;

        void setCompositionOffset(std::int64_t);
        std::int64_t getCompositionOffset() const;

        // baked timestampes in MS
        std::uint32_t getTimeStampCount() const;
        std::uint64_t getTimeStamp(std::uint32_t aId) const;

        /** Sets the item data for the image
         * @param [in] aData: A pointer to the data.
         * @param [in] aLength: The amount of data. */
        void setItemData(const std::uint8_t* aData, std::uint64_t aLength);

        /** Returns the size of the sample data */
        std::uint64_t getSampleDataSize() const;

        /** Returns a pointer to the sample data */
        const std::uint8_t* getSampleData();

        /** Returns the amount of metadata items */
        std::uint32_t getMetadataCount() const;
        /** Gets a Meta item with the given index
         * @param [in] aIndex: The index of the meta item. */
        MetaItem* getMetadata(std::uint32_t aIndex);
        const MetaItem* getMetadata(std::uint32_t aIndex) const;

        void addMetadata(MetaItem* aMeta);
        void removeMetadata(MetaItem* aMeta);


        std::uint32_t getDecodeDependencyCount() const;
        void setDecodeDependency(std::uint32_t aId, Sample* aSample);
        Sample* getDecodeDependency(std::uint32_t aId);
        const Sample* getDecodeDependency(std::uint32_t aId) const;
        void addDecodeDependency(Sample* aSample);
        void removeDecodeDependency(std::uint32_t aId);
        void removeDecodeDependency(Sample* aSample);


        Heif* getHeif();
        Track* getTrack();
        const Heif* getHeif() const;
        const Track* getTrack() const;

        // groups
        std::uint32_t getGroupCount() const;
        EntityGroup* getGroup(std::uint32_t aId);

        std::uint32_t getGroupByTypeCount(const HEIF::FourCC& aType);
        EntityGroup* getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId);
        EntityGroup* getGroupById(const HEIF::GroupId& aId);

    protected:
        void addToGroup(EntityGroup* aGroup);
        void removeFromGroup(EntityGroup* aGroup);
        // serialization methods.
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader,
                                     const HEIF::SequenceId& aTrack,
                                     const HEIF::SampleInformation& aInfo);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void setId(const HEIF::SequenceImageId&);

        Sample(Heif* aHeif);
        Heif* mHeif;

        HEIF::FourCC mType;
        HEIF::SequenceImageId mId;
        HEIF::SampleType mSampleType;
        std::uint64_t mDuration;
        std::int64_t mCompositionOffset;
        std::vector<std::uint64_t> mTimeStamps;
        DecoderConfig* mConfig;
        // HEIF::MediaFormat mFormat;
        bool mIsAudio;
        bool mIsVideo;
        void link(Track* aTrack);
        void unlink(Track* aTrack);
        void link(Sample* aSample);
        void unlink(Sample* aSample);

        std::vector<MetaItem*> mMetaItems;
        Track* mTrack;

        std::vector<Sample*> mDecodeDependency;
        LinkArray<Sample*> mDecodeDependencyLinks;
        std::vector<EntityGroup*> mGroups;
        std::uint64_t mBufferSize;
        std::uint8_t* mBuffer;

        const void* mContext;

    private:
        HEIF::ErrorCode loadSampleData(const HEIF::SequenceId& aTrackId);

    private:
        Sample& operator=(const Sample&) = delete;
        Sample& operator=(Sample&&) = delete;
        Sample(const Sample&)       = delete;
        Sample(Sample&&)            = delete;
        Sample()                    = delete;
    };

}  // namespace HEIFPP
