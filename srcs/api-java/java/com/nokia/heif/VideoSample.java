package com.nokia.heif;

/**
 * A sample containing video data
 */
public abstract class VideoSample extends Sample
{
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    protected VideoSample(HEIF heif)
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
    protected VideoSample(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

}
