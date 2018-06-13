package com.nokia.heif;

import java.util.ArrayList;
import java.util.List;

/**
 * A track containing audio samples
 */
public final class AudioTrack extends Track
{
    /**
     * Creates a new AudioTrack to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param timescale The timescale of the track. How many timescale units are per second
     * @throws Exception
     */
    public AudioTrack(HEIF heif, int timescale)
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
     * Protected constructor, used to create an object from the native side
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected AudioTrack(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns all the samples that belong to this track
     *
     * @return A list of the samples
     * @throws Exception
     */
    public List<AudioSample> getAudioSamples()
            throws Exception
    {
        checkState();
        List<Sample> samples = super.getSamples();
        List<AudioSample> result = new ArrayList<>(samples.size());
        for(Sample sample : samples)
        {
            result.add((AudioSample) sample);
        }
        return result;
    }

    /**
     * Adds a sample to the track
     *
     * @param sample The sample to be added
     * @throws Exception
     */
    public void addSample(AudioSample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        addSampleNative(sample);
    }

    private native long createContextNative(HEIF heif);

    private native void addSampleNative(AudioSample sample);

}
