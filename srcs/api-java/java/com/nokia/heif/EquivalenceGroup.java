package com.nokia.heif;

public final class EquivalenceGroup extends EntityGroup
{
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    public EquivalenceGroup(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected EquivalenceGroup(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }


    /**
     * Adds an item to the group
     *
     * @param item The Item to be added
     * @throws Exception
     */
    @Override
    public void addItem(Item item)
            throws Exception
    {
        super.addItem(item);
    }

    /**
     * Removes an item from the group
     *
     * @param item The item to be removed
     * @throws Exception
     */
    @Override
    public void removeItem(Item item)
            throws Exception
    {
        super.removeItem(item);
    }

    /**
     * Adds a sample to the group
     *
     * @param sample The sample to be added
     * @throws Exception
     */
    @Override
    public void addSample(Sample sample)
            throws Exception
    {
        addSample(sample, 0, 1 << 8);
    }

    /**
     * Adds a sample to the group with an offset
     *
     * @param sample The sample to be added
     * @param offset The offset of the sample
     * @param multiplier The offset multiplier of the sample
     * @throws Exception
     */
    public void addSample(Sample sample, int offset, int multiplier)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        addSampleNative(sample, offset, multiplier);
    }

    /**
     * Removes a sample from the group
     *
     * @param sample The sample to be removed
     * @throws Exception
     */
    @Override
    public void removeSample(Sample sample)
            throws Exception
    {
        super.removeSample(sample);
    }

    /**
     * Returns the time offset of the sample
     *
     * @param sample The sample
     * @return The time offset of the sample
     * @throws Exception
     */
    public int getSampleTimeOffset(Sample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        return getOffsetNative(sample);
    }

    /**
     * Returns the time offset multiplier of the sample
     *
     * @param sample The sample
     * @return The time offset multiplier
     * @throws Exception
     */
    public int getSampleTimeOffsetMultiplier(Sample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        return getMultiplierNative(sample);
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    native private long createContextNative(HEIF heif);
    native private void destroyContextNative();

    native private void addSampleNative(Sample sample, int offset, int multiplier);
    native private int getOffsetNative(Sample sample);
    native private int getMultiplierNative(Sample sample);
}
