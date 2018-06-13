package com.nokia.heif;

import java.util.ArrayList;
import java.util.List;

/**
 * A track containing video samples.
 */
public class VideoTrack extends Track
{

    /**
     * Creates a new AudioTrack to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param timescale The timescale of the track. How many timescale units are per second
     * @throws Exception
     */
    public VideoTrack(HEIF heif, int timescale)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setTimescale(timescale);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, called by child class
     * @param heif
     * @throws Exception
     */
    protected VideoTrack(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected VideoTrack(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns all the samples that belong to this track
     *
     * @return A list of the samples
     * @throws Exception
     */
    public List<VideoSample> getVideoSamples()
            throws Exception
    {
        checkState();
        List<Sample> samples = super.getSamples();
        List<VideoSample> result = new ArrayList<>(samples.size());
        for(Sample sample : samples)
        {
            result.add((VideoSample) sample);
        }
        return result;
    }

    /**
     * Adds a sample to the track
     *
     * @param sample The sample to be added
     * @throws Exception
     */
    public void addSample(VideoSample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        addSampleNative(sample);
    }

    private native long createContextNative(HEIF heif);

    private native void addSampleNative(VideoSample sample);
}
