package com.nokia.heif;

/**
 * ImageSequence, a special case of a VideoTrack
 */
public final class ImageSequence extends VideoTrack
{
    /**
     * Creates a new ImageSequence to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param timescale The timescale of the track. How many timescale units are per second.
     * @throws Exception
     */
    public ImageSequence(HEIF heif, int timescale)
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
    protected ImageSequence(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    private native long createContextNative(HEIF heif);
}
