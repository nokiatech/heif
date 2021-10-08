/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 */

package com.nokia.heif;

import java.util.ArrayList;
import java.util.List;

/**
 * Track, contains data as Samples
 */
public abstract class Track extends Base
{

    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws Exception
     */
    protected Track(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected Track(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the samples in the track
     *
     * @return A list of all the samples
     * @throws Exception
     */
    public List<Sample> getSamples()
            throws Exception
    {
        checkState();
        int count = getSampleCountNative();
        List<Sample> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getSampleNative(index));
        }
        return result;
    }

    /**
     * Removes a sample from the track
     *
     * @param sample The sample to be removed
     * @throws Exception
     */
    public void removeSample(Sample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        removeSampleNative(sample);
    }

    /**
     * Returns the timescale of the track.
     * Timescale defines how many timescale units are in a second.
     *
     * @return The timescale of the track
     * @throws Exception
     */
    public int getTimescale()
            throws Exception
    {
        checkState();
        return getTimescaleNative();
    }

    /**
     * Sets the timescale for the track.
     * Timescale defines how many timescale units are in a second.
     *
     * @param timescale The timescale, must be larger than 0
     * @throws Exception
     */
    public void setTimescale(int timescale)
            throws Exception
    {
        checkState();
        if (timescale <= 0)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Timescale must be > 0");
        }
        setTimescaleNative(timescale);
    }

    /**
     * Returns the auxiliary tracks of this track
     *
     * @return A list of the auxiliary tracks
     * @throws Exception
     */
    public List<Track> getAuxiliaries()
            throws Exception
    {
        checkState();
        int count =  getAuxiliaryCountNative();
        List<Track> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getAuxiliaryNative(index));
        }
        return result;
    }

    /**
     * Adds an auxiliary track
     *
     * @param auxiliary The auxiliary track to be added
     * @throws Exception
     */
    public void addAuxiliary(Track auxiliary)
            throws Exception
    {
        checkState();
        checkParameter(auxiliary);
        addAuxiliaryNative(auxiliary);
    }

    /**
     * Removes an auxiliary track
     *
     * @param auxiliary The auxiliary track to be removed
     * @throws Exception
     */
    public void removeAuxiliary(Track auxiliary)
            throws Exception
    {
        checkState();
        checkParameter(auxiliary);
        removeAuxiliaryNative(auxiliary);
    }

    /**
     * Returns the thumbnails tracks of this track
     *
     * @return A list of the thumbnail tracks
     * @throws Exception
     */
    public List<Track> getThumbnails()
            throws Exception
    {
        checkState();
        int count = getThumbnailCountNative();
        List<Track> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getThumbnailNative(index));
        }
        return result;
    }

    /**
     * Adds a thumbnail track
     *
     * @param thumbnail The thumbnail track to be added
     * @throws Exception
     */
    public void addThumbnail(Track thumbnail)
            throws Exception
    {
        checkState();
        checkParameter(thumbnail);
        addThumbnailNative(thumbnail);
    }

    /**
     * Removes a thumbnail track
     *
     * @param thumbnail The thumbnail track to be removed
     * @throws Exception
     */
    public void removeThumbnail(Track thumbnail)
            throws Exception
    {
        checkState();
        checkParameter(thumbnail);
        removeThumbnailNative(thumbnail);
    }

    /**
     * Returns the alternative track group of this track
     *
     * @return The altenative track group, null if this track isn't part of any alt track group
     * @throws Exception
     */
    public AlternativeTrackGroup getAlternativeTrackGroup()
            throws Exception
    {
        checkState();
        return getAlternativeTrackGroupNative();
    }

    /**
     * Returns a list of all the EntityGroups this track belongs to
     * @return A list of the EntityGroups
     * @throws Exception
     */
    public List<EntityGroup> getEntityGroups()
            throws Exception
    {
        checkState();
        int count = getGroupCountNative();
        List<EntityGroup> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            EntityGroup group = getGroupNative(index);
            if (!(group instanceof EquivalenceGroup))
            {
                result.add(group);
            }
        }
        return result;
    }

    /**
     * Returns if the track is set to loop infinitely
     * @return If the track loops infinitely
     * @throws Exception
     */
    public boolean hasInfiniteLoopPlayback()
            throws Exception
    {
        checkState();
        return hasInfiniteLoopPlaybackNative();
    }

    /**
     * The duration of the track
     * @return The duration in seconds
     */
    public double getDuration()
            throws Exception
    {
        checkState();
        return getDurationNative();
    }

    public class TimeStamp
    {
        public int sampleIndex;
        public long timeStamp; // Timestamp in milliseconds
    }

    /**
     * Returns the timestamps in the track
     *
     * @return A list of all the timestamps
     * @throws Exception
     */
    public List<TimeStamp> getTimestamps()
            throws Exception
    {
        checkState();
        int count = getTimestampCountNative();
        List<TimeStamp> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getTimestampNative(index));
        }
        return result;
    }

    // order here should be same as in heifcommondatatypes.h
    public enum EditType {
        EMPTY,
        DWELL,
        SHIFT,
        RAW
    }

    public class EditUnit
    {
        public long mediaTimeInTrackTS;
        public long durationInMovieTS;
        public int mediaRateInteger;
        public int mediaRateFraction;
        public EditType editType;
    }

    public void setEditListLooping(boolean isLooping)
            throws Exception
    {
        checkState();
        setEditListLoopingNative(isLooping);
    }

    public void setEditListRepetitions(double repetitions)
            throws Exception
    {
        checkState();
        setEditListRepetitionsNative(repetitions);
    }

    public void addEditListUnit(EditUnit editUnit)
            throws Exception
    {
        checkState();
        addEditListUnitNative(editUnit);
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    native private void destroyContextNative();

    native private int getSampleCountNative();
    native private Sample getSampleNative(int index);
    native private void removeSampleNative(Sample sample);

    native private int getTimescaleNative();
    native private void setTimescaleNative(int timescale);

    native private int getThumbnailCountNative();
    native private Track getThumbnailNative(int index);
    native private void addThumbnailNative(Track thumbnail);
    native private void removeThumbnailNative(Track thumbnail);

    native private int getAuxiliaryCountNative();
    native private Track getAuxiliaryNative(int index);
    native private void addAuxiliaryNative(Track auxiliary);
    native private void removeAuxiliaryNative(Track auxiliary);

    native private AlternativeTrackGroup getAlternativeTrackGroupNative();

    native private int getGroupCountNative();
    native private EntityGroup getGroupNative(int index);

    native private boolean hasInfiniteLoopPlaybackNative();
    native private double getDurationNative();

    native private int getTimestampCountNative();
    native private TimeStamp getTimestampNative(int index);

    native private void setEditListLoopingNative(boolean isLooping);
    native private void setEditListRepetitionsNative(double repetitions);
    native private void addEditListUnitNative(EditUnit editUnit);
}
