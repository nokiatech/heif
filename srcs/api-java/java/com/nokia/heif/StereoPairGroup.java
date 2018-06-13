package com.nokia.heif;

import java.util.List;

public final class StereoPairGroup extends EntityGroup
{
    /**
     * Constructor, abstract class so not called directly
     *
     * @param heif The parent HEIF instance for the new object
     * @throws java.lang.Exception Thrown if the parent HEIF instance is invalid
     */
    public StereoPairGroup(HEIF heif, ImageItem leftView, ImageItem rightView)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setStereoPair(leftView, rightView);
        }
        catch (Exception ex)
        {
            destroyNative();
            throw ex;
        }
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected StereoPairGroup(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Sets the image items for this stereo pair
     *
     * @param leftView The left view Image
     * @param rightView The right view Image
     * @throws Exception
     */
    public void setStereoPair(ImageItem leftView, ImageItem rightView)
            throws Exception
    {
        checkState();
        checkParameter(leftView);
        checkParameter(rightView);

        List<Base> existingItems = getMembers();
        for (int i = 0; i < existingItems.size() ; i++)
        {
            removeEntityFromGroup(existingItems.get(i));
        }

        addItem(leftView);
        addItem(rightView);
    }

    /**
     * Returns the left view image item of the pair
     *
     * @return The left image
     * @throws Exception
     */
    public ImageItem getLeftView()
            throws Exception
    {
        List<Base> members = getMembers();
        if (members.size() == 2) // must be 2 according to HEIF specification
        {
            if (super.isItem(0))  // 0 = left view according to HEIF specification
            {
                return (ImageItem)members.get(0); // 0 = left view according to HEIF specification
            }
            else
            {
                throw new Exception(ErrorHandler.INVALID_STRUCTURE, "Stereo pair contains something else than Items");
            }
        }
        else
        {
            throw new Exception(ErrorHandler.INVALID_STRUCTURE, "Stereo pair shall contain only 2 items");
        }
    }

    /**
     * Returns the right view image of the pair
     *
     * @return The right image
     * @throws Exception
     */
    public ImageItem getRightView()
            throws Exception
    {
        List<Base> members = getMembers();
        if (members.size() == 2) // must be 2 according to HEIF specification
        {
            if (super.isItem(1)) // 1 = right view according to HEIF specification
            {
                return (ImageItem)members.get(1); // 1 = right view according to HEIF specification
            }
            else
            {
                throw new Exception(ErrorHandler.INVALID_STRUCTURE, "Stereo pair contains something else than Items");
            }
        }
        else
        {
            throw new Exception(ErrorHandler.INVALID_STRUCTURE, "Stereo pair shall contain only 2 items");
        }
    }

    private native long createContextNative(HEIF heif);

}
