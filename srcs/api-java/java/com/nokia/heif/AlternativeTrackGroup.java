package com.nokia.heif;

import java.util.ArrayList;
import java.util.List;

/**
 * Group containing multiple tracks which are alternatives of eachother
 */
public final class AlternativeTrackGroup extends Base
{
    /**
     * Creates a new AlternativeTrackGroup to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    public AlternativeTrackGroup(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected AlternativeTrackGroup(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns a list of all the Tracks belonging to the alternative group
     * @return A list of the tracks
     * @throws Exception
     */
    public List<Track> getTracks()
            throws Exception
    {
        checkState();
        int count = getTrackCountNative();
        List<Track> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getTrackNative(index));
        }
        return result;
    }

    /**
     * Adds a track to the track group
     *
     * @param track The track to be added
     * @throws Exception
     */
    public void addTrack(Track track)
            throws Exception
    {
        checkState();
        checkParameter(track);
        addTrackNative(track);
    }

    /**
     * Removes a track to the track group
     *
     * @param track The track to be removed
     * @throws Exception
     */
    public void removeTrack(Track track)
            throws Exception
    {
        checkState();
        checkParameter(track);
        removeTrackNative(track);
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    private native long createContextNative(HEIF heif);
    native private void destroyContextNative();

    native private int getTrackCountNative();
    native private Track getTrackNative(int index);
    native private void addTrackNative(Track track);
    native private void removeTrackNative(Track track);

}
