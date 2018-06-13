package com.nokia.heif;

import java.util.ArrayList;
import java.util.List;

public class EntityGroup extends Base
{
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    protected EntityGroup(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor which can be called by a child class to create a custom entity group
     *
     * @param heif The parent HEIF instance for the new object
     * @param groupType The type FourCC of the custom group.
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    protected EntityGroup(HEIF heif, FourCC groupType)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif, groupType.toString());
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected EntityGroup(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the type of the entity group
     *
     * @return The type as a string
     * @throws Exception
     */
    public FourCC getType()
            throws Exception
    {
        checkState();
        return new FourCC(getTypeNative());
    }

    /**
     * Returns a list of all the members of the group
     *
     * @return List containing all the members
     * @throws Exception
     */
    public List<Base> getMembers()
            throws Exception
    {
        checkState();
        int count = getEntityCountNative();
        List<Base> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            if (isItemNative(index))
            {
                result.add(getItemNative(index));
            }
            else if (isSampleNative(index))
            {
                result.add(getSampleNative(index));
            }
            else if (isTrackNative(index) && !(this instanceof EquivalenceGroup))
            {
                result.add(getTrackNative(index));
            }
        }
        return result;
    }

    /**
     * Adds an item to the group. Protected, child classes will expose this when needed
     *
     * @param item The Item to be added
     * @throws Exception
     */
    protected void addItem(Item item)
            throws Exception
    {
        checkState();
        checkParameter(item);
        addEntityToGroup(item);
    }

    /**
     * Adds a track to the group. Protected, child classes will expose this when needed
     *
     * @param track The track to be added
     * @throws Exception
     */
    protected void addTrack(Track track)
            throws Exception
    {
        checkState();
        checkParameter(track);
        addEntityToGroup(track);
    }

    /**
     * Adds a sample to the group. Protected, child classes will expose this when needed
     *
     * @param sample The sample to be added
     * @throws Exception
     */
    protected void addSample(Sample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        addEntityToGroup(sample);
    }

    /**
     * Removes an item from the group. Protected, child classes will expose this when needed
     *
     * @param item The item to be removed
     * @throws Exception
     */
    protected void removeItem(Item item)
            throws Exception
    {
        checkState();
        checkParameter(item);
        removeEntityFromGroup(item);
    }

    /**
     * Removes a track from the group. Protected, child classes will expose this when needed
     *
     * @param track The track to be removed
     * @throws Exception
     */
    protected void removeTrack(Track track)
            throws Exception
    {
        checkState();
        checkParameter(track);
        removeEntityFromGroup(track);
    }

    /**
     * Removes a sample from the group. Protected, child classes will expose this when needed
     *
     * @param sample The sample to be removed
     * @throws Exception
     */
    protected void removeSample(Sample sample)
            throws Exception
    {
        checkState();
        checkParameter(sample);
        removeEntityFromGroup(sample);
    }

    /**
     * Returns if the member is an Item. Protected, used only from child classes
     *
     * @param index The index of the member to be checked
     * @return If the member is an Item
     * @throws Exception
     */
    protected boolean isItem(int index)
            throws Exception
    {
        checkState();
        return isItemNative(index);
    }


    /**
     * Adds an object to the group. Private method.
     *
     * @param entity The object to be added.
     * @throws Exception
     */
    private void addEntityToGroup(Base entity)
    {
        if (entity instanceof Item)
        {
            addItemNative((Item)entity);
        }
        else if (entity instanceof Track)
        {
            addTrackNative((Track)entity);
        }
        else if (entity instanceof Sample)
        {
            addSampleNative((Sample)entity);
        }
    }

    /**
     * Removes an object from the group. Called by child classes
     *
     * @param entity The object to be removed
     * @throws Exception
     */
    protected void removeEntityFromGroup(Base entity)
            throws Exception
    {
        checkState();
        checkParameter(entity);
        if (entity instanceof Item)
        {
            removeItemNative((Item)entity);
        }
        else if (entity instanceof Track)
        {
            removeTrackNative((Track)entity);
        }
        else if (entity instanceof Sample)
        {
            removeSampleNative((Sample)entity);
        }
    }

    private native long createContextNative(HEIF heif, String type);

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    native private void destroyContextNative();

    native private String getTypeNative();
    native private int getEntityCountNative();

    native private boolean isItemNative(int index);
    native private Item getItemNative(int index);
    native private void addItemNative(Item item);
    native private void removeItemNative(Item item);

    native private boolean isTrackNative(int index);
    native private Track getTrackNative(int index);
    native private void addTrackNative(Track track);
    native private void removeTrackNative(Track track);

    native private boolean isSampleNative(int index);
    native private Sample getSampleNative(int index);
    native private void addSampleNative(Sample sample);
    native private void removeSampleNative(Sample sample);
}
