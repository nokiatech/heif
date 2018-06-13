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

    native private AlternativeTrackGroup getAlternativeTrackGroupNative();

    native private int getGroupCountNative();
    native private EntityGroup getGroupNative(int index);

    native private boolean hasInfiniteLoopPlaybackNative();
    native private double getDurationNative();
}
