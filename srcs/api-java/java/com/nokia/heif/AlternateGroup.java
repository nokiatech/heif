package com.nokia.heif;

public final class AlternateGroup extends EntityGroup
{
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    public AlternateGroup(HEIF heif)
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
    protected AlternateGroup(HEIF heif, long nativeHandle)
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
     * Adds a track to the group
     *
     * @param track The track to be added
     * @throws Exception
     */
    @Override
    public void addTrack(Track track)
            throws Exception
    {
        super.addTrack(track);
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
     * Removes a track from the group
     *
     * @param track The track to be removed
     * @throws Exception
     */
    @Override
    public void removeTrack(Track track)
            throws Exception
    {
        super.removeTrack(track);
    }

    private native long createContextNative(HEIF heif);
}
