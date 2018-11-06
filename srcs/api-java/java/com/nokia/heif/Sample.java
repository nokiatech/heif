package com.nokia.heif;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * Sample, a storage unit for data which belongs to a Track
 */
public abstract class Sample extends Base
{

    public enum Type
    {
        NON_REFERENCE,
        REFERENCE,
        HIDDEN_REFERENCE
    }
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws Exception
     */
    protected Sample(HEIF heif)
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
    protected Sample(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder config of the sample
     *
     * @return DecoderConfig
     * @throws Exception
     */
    public DecoderConfig getDecoderConfig()
            throws Exception
    {
        checkState();
        return getDecoderConfigNative();
    }

    /**
     * Sets the decoder config for the sample
     *
     * @param config Decoder config, must match the type of the Sample
     * @throws Exception
     */
    public void setDecoderConfig(DecoderConfig config)
            throws Exception
    {
        checkState();
        checkParameter(config);

        if (!checkDecoderConfigType(config))
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Incorrect decoder config type");
        }

        setDecoderConfigNative(config);
    }

    /**
     * Returns the type of the sample
     *
     * @see Sample.Type
     * @return The Type of the sample
     * @throws Exception
     */
    public Type getType()
            throws Exception
    {
        checkState();
        int typeNative = getSampleTypeNative();
        if (typeNative == 0)
        {
            return Type.NON_REFERENCE;
        }
        else if (typeNative == 1)
        {
            return Type.REFERENCE;
        }
        else
        {
            return Type.HIDDEN_REFERENCE;
        }
    }

    /**
     * Returns the size of the sample data.
     *
     * @return The size in bytes
     * @throws Exception
     */
    public int getSampleDataSize()
            throws Exception
    {
        checkState();
        return getSampleDataSizeNative();
    }

    /**
     * Returns the sample data as a byte array.
     * Note that this does a memory copy from the ByteBuffer so using the ByteBuffer getter is more optimized
     *
     * @return A ByteArray containing the sample data
     * @throws Exception
     */
    public byte[] getSampleDataAsArray()
            throws Exception
    {
        checkState();
        ByteBuffer output = getSampleData();
        byte[] data = new byte[output.remaining()];
        output.get(data);
        return data;
    }

    /**
     * Returns the sample data as a ByteBuffer. The buffer is a DirectBuffer with the data being owned by the native
     * object so the CodedImageItem or its parent HEIF instance is destroyed, the ByteBuffer is invalidated
     *
     * @return The sample data in a ByteBuffer
     * @throws Exception
     */
    public ByteBuffer getSampleData()
            throws Exception
    {
        checkState();
        return getSampleDataNative();
    }

    /**
     * Sets the sample data for the sAmple. The byte arrays contents are copied in the native code so
     * the byte array can be released after calling the setter.
     *
     * @param sampleData Sample data as a byte array
     * @throws Exception
     */
    public void setSampleData(byte[] sampleData)
            throws Exception
    {
        checkState();
        checkParameter(sampleData);
        setSampleDataNative(sampleData);
    }

    /**
     * Returns the list of the timestamps of the sample. Not that these timestamps are prederived
     * from the sample durations of the other samples in the track.
     *
     * @return List of the time stamps in milliseconds
     * @throws Exception
     */
    public List<Long> getTimestamps()
            throws Exception
    {
        checkState();
        int count = getTimestampCountNative();
        List<Long> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getTimestampNative(index));
        }
        return result;
    }

    /**
     * Returns the duration of sample
     *
     * @return The duration in timescale units
     * @throws Exception
     */
    public long getDuration()
            throws Exception
    {
        checkState();
        return getDurationNative();
    }

    /**
     * Sets the duration of the sample
     *
     * @param duration The duration in timescale units
     * @throws Exception
     */
    public void setDuration(long duration)
            throws Exception
    {
        checkState();
        setDurationNative(duration);
    }

    /**
     * Returns the composition offset of the sample
     *
     * @return The composition offset in timescale units
     * @throws Exception
     */
    public long getCompositionOffset()
            throws Exception
    {
        checkState();
        return getCompositionOffsetNative();
    }

    /**
     * Sets the composition offset of the sample
     *
     * @param offset The composition offset in timescale units
     * @throws Exception
     */
    public void setCompositionOffset(long offset)
            throws Exception
    {
        checkState();
        setCompositionOffsetNative(offset);
    }

    /**
     * Returns the decode dependencies of the sample
     *
     * @return A list of the decode dependency samples
     * @throws Exception
     */
    public List<Sample> getDecodeDependencies()
            throws Exception
    {
        checkState();
        int count = getDecodeDependencyCountNative();
        List<Sample> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getDecodeDependencyNative(index));
        }
        return result;
    }

    /**
     * Adds a decode dependency
     *
     * @param dependency  The sample to be added as a dependency
     * @throws Exception
     */
    public void addDecodeDependency(Sample dependency)
            throws Exception
    {
        checkState();
        checkParameter(dependency);
        addDecodeDependencyNative(dependency);
    }

    /**
     * Removes a decode dependency
     *
     * @param dependency  The sample to be removed as a dependency
     * @throws Exception
     */
    public void removeDecodeDependency(Sample dependency)
            throws Exception
    {
        checkState();
        checkParameter(dependency);
        removeDecodeDependencyNative(dependency);
    }

    /**
     * Returns the MetaItems of the Sample
     *
     * @return List of the metadata items
     * @throws Exception
     */
    public List<MetaItem> getMetadatas()
            throws Exception
    {
        checkState();
        int count = getMetadataCountNative();
        List<MetaItem> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getMetadataNative(index));
        }
        return result;
    }

    /**
     * Adds a metadata item for the Sample
     *
     * @param item Item to be added
     * @throws Exception
     */
    public void addMetadata(MetaItem item)
            throws Exception
    {
        checkState();
        checkParameter(item);
        addMetadataNative(item);
    }

    /**
     * Removes a metadata item from the Sample
     *
     * @param item Item to be removed
     * @throws Exception
     */
    public void removeMetadata(MetaItem item)
            throws Exception
    {
        checkState();
        removeMetadataNative(item);
    }

    /**
     * Returns a list of all the EntityGroups this sample belongs to
     *
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
            result.add(getGroupNative(index));
        }
        return result;
    }

    /**
     * Returns the track that this sample belongs to
     *
     * @return The track
     * @throws Exception
     */
    public Track getTrack()
            throws Exception
    {
        checkState();
        return getTrackNative();
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    abstract protected boolean checkDecoderConfigType(DecoderConfig config);

    native private void destroyContextNative();

    native private DecoderConfig getDecoderConfigNative();
    native private void setDecoderConfigNative(DecoderConfig config);


    native private int getSampleDataSizeNative();
    native private ByteBuffer getSampleDataNative();
    native private void setSampleDataNative(byte[] data);

    native private long getDurationNative();
    native private void setDurationNative(long duration);

    native private int getTimestampCountNative();
    native private long getTimestampNative(int index);

    native private void setCompositionOffsetNative(long offset);
    native private long getCompositionOffsetNative();

    native private int getDecodeDependencyCountNative();
    native private Sample getDecodeDependencyNative(int index);
    native private void addDecodeDependencyNative(Sample dependency);
    native private void removeDecodeDependencyNative(Sample dependency);


    native private int getMetadataCountNative();
    native private MetaItem getMetadataNative(int index);
    native private void addMetadataNative(MetaItem item);
    native private void removeMetadataNative(MetaItem item);

    native private int getGroupCountNative();
    native private EntityGroup getGroupNative(int index);

    native private Track getTrackNative();

    native private int getSampleTypeNative();
}
