/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 *
 */

package com.nokia.heif;

import com.nokia.heif.io.InputStream;
import com.nokia.heif.io.OutputStream;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class HEIF
{

    private static final String TAG = "HEIF";

    static
    {
        System.loadLibrary("heifjni");
    }

    public static final FourCC FOURCC_AVC = new FourCC("avc1", true);
    public static final FourCC FOURCC_HEVC = new FourCC("hvc1", true);
    public static final FourCC FOURCC_JPEG = new FourCC("jpeg", true);
    public static final FourCC FOURCC_IDENTITY = new FourCC("iden", true);
    public static final FourCC FOURCC_OVERLAY = new FourCC("iovl", true);
    public static final FourCC FOURCC_GRID = new FourCC("grid", true);
    public static final FourCC FOURCC_EXIF = new FourCC("Exif", true);
    public static final FourCC FOURCC_AAC = new FourCC("mp4a", true);


    public static final FourCC FOURCC_AUDIO_TRACK = new FourCC("soun", true);
    public static final FourCC FOURCC_VIDEO_TRACK = new FourCC("vide", true);
    public static final FourCC FOURCC_IMAGE_SEQUENCE = new FourCC("pict", true);
    public static final FourCC FOURCC_EQUIVALENCE = new FourCC("eqiv", true);
    public static final FourCC FOURCC_ALTERNATE = new FourCC("altr", true);
    public static final FourCC FOURCC_STEREO_PAIR = new FourCC("ster", true);
    public static final FourCC FOURCC_AUXILIARY_PROPERTY = new FourCC("auxc", true);
    public static final FourCC FOURCC_CLEAN_APERTURE_PROPERTY = new FourCC("clap", true);
    public static final FourCC FOURCC_NCLX_COLOUR_PROPERTY = new FourCC("nclx", true);
    public static final FourCC FOURCC_MIRROR_PROPERTY = new FourCC("imir", true);
    public static final FourCC FOURCC_ROTATE_PROPERTY = new FourCC("irot", true);
    public static final FourCC FOURCC_PIXEL_ASPECT_RATIO_PROPERTY = new FourCC("pasp", true);
    public static final FourCC FOURCC_PIXEL_INFORMATION_RATIO_PROPERTY = new FourCC("pixi", true);
    public static final FourCC FOURCC_RELATIVE_LOCATION_PROPERTY = new FourCC("rloc", true);

    public static final FourCC BRAND_MIF1 = new FourCC("mif1", true);
    public static final FourCC BRAND_HEIC = new FourCC("heic", true);
    public static final FourCC BRAND_HEIX = new FourCC("heix", true);
    public static final FourCC BRAND_HEVC = new FourCC("hevc", true);
    public static final FourCC BRAND_AVCS = new FourCC("avcs", true);
    public static final FourCC BRAND_AVCI = new FourCC("avci", true);
    public static final FourCC BRAND_JPEG = new FourCC("jpeg", true);
    public static final FourCC BRAND_ISO8 = new FourCC("iso8", true);
    public static final FourCC BRAND_MSF1 = new FourCC("msf1", true);
    public static final FourCC BRAND_MP41 = new FourCC("mp41", true);


    /**
     * Creates a HEIF instance which can be used to read and write HEIF files
     */
    public HEIF()
    {
        createInstanceNative();
    }

    /**
     * Creates a HEIF instance from the given file
     * @param filename Path to the file to be opened
     * @throws Exception Thrown if the loading fails
     */
    public HEIF(String filename)
            throws Exception
    {
        this();
        try
        {
            load(filename);
        }
        catch (Exception ex)
        {
            release();
            throw ex;
        }
    }

    /**
     * Creates a HEIF instance from the given input stream
     * @param inputStream Input stream
     * @throws Exception Thrown if the loading fails
     */
    public HEIF(InputStream inputStream)
            throws Exception
    {
        this();
        try
        {
            load(inputStream);
        }
        catch (Exception ex)
        {
            release();
            throw ex;
        }
    }

    /**
     * Registers a custom entity group type.
     *
     * @param groupClass The class of the custom group
     * @param groupType The FourCC type of the custom group
     * @throws Exception
     */
    public static void registerEntityGroup(Class groupClass, FourCC groupType)
            throws Exception
    {
        if (groupClass.equals(ENTITY_GROUP_CLASSES.get(groupType.toString())))
        {
            // No need to add the class again
            return;
        }
        else if (!EntityGroup.class.isAssignableFrom(groupClass))
        {
            throw new Exception(ErrorHandler.INCORRECT_SUPERCLASS, "Given class does not inherit EntityGroup");
        }
        else if (ENTITY_GROUP_CLASSES.containsKey(groupType.toString()))
        {
            throw new Exception(ErrorHandler.TYPE_ALREADY_EXISTS, "The type is already registered");
        }
        ENTITY_GROUP_CLASSES.put(groupType.toString(), groupClass);
    }

    /**
     * Registers a custom item property type
     *
     * @param groupClass The class of the custom property
     * @param groupType The FourCC type of the custom property
     * @throws Exception
     */
    public static void registerItemProperty(Class groupClass, FourCC groupType)
            throws Exception
    {
        if (!ItemProperty.class.isAssignableFrom(groupClass))
        {
            throw new Exception(ErrorHandler.INCORRECT_SUPERCLASS, "Given class does not inherit EntityGroup");
        }
        else if (ITEM_PROPERTY_CLASSES.containsKey(groupType.toString()))
        {
            throw new Exception(ErrorHandler.TYPE_ALREADY_EXISTS, "The type is already registered");
        }
        ITEM_PROPERTY_CLASSES.put(groupType.toString(), groupClass);
    }

    /**
     * Releases the resources held by the HEIF instance. This will invalidate every child item of the instance
     */
    public void release()
    {
        if (mNativeHandle != 0)
        {
            destroyInstanceNative();
        }
    }

    @Override
    protected void finalize()
            throws Throwable
    {
        if (mNativeHandle != 0)
        {
            destroyInstanceNative();
        }
        super.finalize();
    }

    protected long mNativeHandle;

    /**
     * Loads a HEIF file. Can be called only once per instance.
     * @param filename Filename including the path
     * @throws Exception
     */
    public void load(String filename)
            throws Exception
    {
        checkState();
        checkParameter(filename);
        loadNative(filename);
    }

    /**
     * Loads a HEIF file from a stream
     * @param inputStream The input stream for the file
     * @throws Exception
     */
    public void load(InputStream inputStream)
            throws Exception
    {
        checkState();
        checkParameter(inputStream);
        loadStreamNative(inputStream);
    }

    /**
     * Saves the HEIF instance to a file. Can be called multiple times with the same instance.
     * Checks are performed to ensure that the file will be a valid HEIF file so the instance should have at least
     * a displayable primary image and a major and compatible brands set.
     * @param filename Filename including the path
     * @throws Exception
     */
    public void save(String filename)
            throws Exception
    {
        checkState();
        checkParameter(filename);
        saveNative(filename);
    }

    public void save(OutputStream outputStream)
            throws Exception
    {
        checkState();
        checkParameter(outputStream);
        saveStreamNative(outputStream);
    }

    /**
     * Returns all objects of Item type in this HEIF instance.
     * @return List of all items
     * @throws Exception
     */
    public List<Item> getItems()
            throws Exception
    {
        checkState();
        int itemCount = getItemCountNative();
        List<Item> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getItemNative(index));
        }
        return items;
    }

    /**
     * Returns all objects of ImageItem type in this HEIF instance
     * @return List of all ImageItems
     * @throws Exception
     */
    public List<ImageItem> getImages()
            throws Exception
    {
        checkState();
        int itemCount = getImageCountNative();
        List<ImageItem> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getImageNative(index));
        }
        return items;
    }

    /**
     * Returns all master images of the HEIF instance.
     * Master images are all ImageItems except those that are either thumbnails or auxiliary images.
     * @return List of all master images
     * @throws Exception
     */
    public List<ImageItem> getMasterImages()
            throws Exception
    {
        checkState();
        int itemCount = getMasterImageCountNative();
        List<ImageItem> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getMasterImageNative(index));
        }
        return items;
    }

    /**
     * Returns all Items of the given type
     * @param typeFourCC FourCC code of the wanted type
     * @return List of all Items with the given type
     * @throws Exception
     */
    public List<Item> getItemsOfType(FourCC typeFourCC)
            throws Exception
    {
        checkState();
        checkParameter(typeFourCC);
        int itemCount = getItemsOfTypeCountNative(typeFourCC.toString());
        List<Item> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getItemOfTypeNative(typeFourCC.toString(), index));
        }
        return items;
    }

    /**
     * Returns the primary item of the HEIF instance. Every valid HEIF file should have a primary image.
     * The Primary image can be either a coded image or a derived image.
     * @return The Primary image for this HEIF file
     * @throws Exception
     */
    public ImageItem getPrimaryImage()
            throws Exception
    {
        checkState();
        return getPrimaryItemNative();
    }

    /**
     * Sets the primary image for the HEIF instance.
     * Will replace any existing primary image. Old primary image will still remain in the instance as a master image
     * @param primaryImage The ImageItem to be set as primary image
     * @throws Exception
     */
    public void setPrimaryImage(ImageItem primaryImage)
            throws Exception
    {
        checkState();
        checkParameterAllowNull(primaryImage);
        setPrimaryItemNative(primaryImage);
    }

    /**
     * Returns all compatible brands of the HEIF instance.
     * @return List containing the compatible brands as Strings
     * @throws Exception
     */
    public List<FourCC> getCompatibleBrands()
            throws Exception
    {
        checkState();
        int brandCount = getCompatibleBrandsCountNative();
        List<FourCC> brands = new ArrayList<>(brandCount);
        for (int index = 0; index < brandCount; index++)
        {
            brands.add(new FourCC(getCompatibleBrandNative(index), true));
        }
        return brands;
    }

    /**
     * Adds a compatible brand for the HEIF instance
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void addCompatibleBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        addCompatibleBrandNative(brand.toString());
    }

    /**
     * Removes a compatible brand from the HEIF instance
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void removeCompatibleBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        removeCompatibleBrandNative(brand.toString());
    }

    /**
     * Returns the major brand of the HEIF instance
     * Every valid HEIF file must have the major brand set
     * @return The brand FourCC as a String
     * @throws Exception
     */
    public FourCC getMajorBrand()
            throws Exception
    {
        checkState();
        return new FourCC(getMajorBrandNative(), true);
    }

    /**
     * Sets the major brand for the HEIF instance.
     * Every valid HEIF file must have a major brand so one must be set before saving a HEIF instance to file
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void setMajorBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        setMajorBrandNative(brand.toString());
    }

    /**
     * Returns all the ItemProperties in the HEIF instance
     * @return List of all the ItemProperties
     * @throws Exception
     */
    public List<ItemProperty> getItemProperties()
            throws Exception
    {
        checkState();
        int itemPropertyCount = getPropertyCountNative();
        List<ItemProperty> properties = new ArrayList<>(itemPropertyCount);
        for (int index = 0; index < itemPropertyCount; index++)
        {
            properties.add(getPropertyNative(index));
        }
        return properties;
    }

    /**
     * Returns all the EntityGroups in the HEIF instance
     * @return List of all the EntityGroups
     * @throws Exception
     */
    public List<EntityGroup> getEntityGroups()
            throws Exception
    {
        return getEntityGroupsPrivate(null);
    }


    public List<EntityGroup> getEntityGroupsByType(FourCC type)
            throws Exception
    {
        return getEntityGroupsPrivate(type);
    }

    /**
     * Returns all the tracks in the HEIF file
     * @return List of the tracks
     * @throws Exception
     */
    public List<Track> getTracks()
            throws Exception
    {
        checkState();
        int trackCount = getTrackCountNative();
        List<Track> tracks = new ArrayList<>(trackCount);
        for (int index = 0; index < trackCount; index++)
        {
            tracks.add(getTrackNative(index));
        }
        return tracks;
    }

    /**
     * Returns a list of all the alternative track groups in the HEIF file
     * @return List of all the alternative track groups
     * @throws Exception
     */
    public List<AlternativeTrackGroup> getAlternativeTrackGroups()
            throws Exception
    {
        checkState();
        int count = getAlternativeTrackGroupCountNative();
        List<AlternativeTrackGroup> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getAlternativeTrackGroupNative(index));
        }
        return result;
    }

    /**
     * Resets the entity group class registrations
     */
    static public void resetEntityGroupClasses()
    {
        ENTITY_GROUP_CLASSES = new HashMap<>();
        ENTITY_GROUP_CLASSES.put(FOURCC_EQUIVALENCE.toString(), EquivalenceGroup.class);
        ENTITY_GROUP_CLASSES.put(FOURCC_STEREO_PAIR.toString(), StereoPairGroup.class);
        ENTITY_GROUP_CLASSES.put(FOURCC_ALTERNATE.toString(), AlternateGroup.class);
    }

    /**
     * Resets the item property class registrations
     */
    static public void resetItemPropertyClasses()
    {
        ITEM_PROPERTY_CLASSES = new HashMap<>();
        ITEM_PROPERTY_CLASSES.put(FOURCC_AUXILIARY_PROPERTY.toString(), AuxiliaryProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_CLEAN_APERTURE_PROPERTY.toString(), CleanApertureProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_NCLX_COLOUR_PROPERTY.toString(), NCLXColourProperty.class);
        ITEM_PROPERTY_CLASSES.put("rICC", ICCColourProperty.class);
        ITEM_PROPERTY_CLASSES.put("prof", ICCColourProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_MIRROR_PROPERTY.toString(), MirrorProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_ROTATE_PROPERTY.toString(), RotateProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_PIXEL_ASPECT_RATIO_PROPERTY.toString(), PixelAspectRatioProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_PIXEL_INFORMATION_RATIO_PROPERTY.toString(), PixelInformationProperty.class);
        ITEM_PROPERTY_CLASSES.put(FOURCC_RELATIVE_LOCATION_PROPERTY.toString(), RelativeLocationProperty.class);
    }


    static private void resetItemClasses()
    {
        ITEM_CLASSES = new HashMap<>();
        ITEM_CLASSES.put(FOURCC_HEVC.toString(), HEVCImageItem.class);
        ITEM_CLASSES.put(FOURCC_AVC.toString(), AVCImageItem.class);
        ITEM_CLASSES.put(FOURCC_JPEG.toString(), CodedImageItem.class);
        ITEM_CLASSES.put(FOURCC_IDENTITY.toString(), IdentityImageItem.class);
        ITEM_CLASSES.put(FOURCC_OVERLAY.toString(), OverlayImageItem.class);
        ITEM_CLASSES.put(FOURCC_GRID.toString(), GridImageItem.class);
        ITEM_CLASSES.put(FOURCC_EXIF.toString(), ExifItem.class);
        ITEM_CLASSES.put("xmp1", XMPItem.class);
        ITEM_CLASSES.put("mpg7", MPEG7Item.class);
    }

    static private void resetDecoderConfigClasses()
    {
        DECODER_CONFIG_CLASSES = new HashMap<>();
        DECODER_CONFIG_CLASSES.put(FOURCC_HEVC.toString(), HEVCDecoderConfig.class);
        DECODER_CONFIG_CLASSES.put(FOURCC_AVC.toString(), AVCDecoderConfig.class);
        DECODER_CONFIG_CLASSES.put(FOURCC_AAC.toString(), AACDecoderConfig.class);
    }

    static private void resetSampleClasses()
    {
        SAMPLE_CLASSES = new HashMap<>();
        SAMPLE_CLASSES.put(FOURCC_HEVC.toString(), HEVCSample.class);
        SAMPLE_CLASSES.put(FOURCC_AVC.toString(), AVCSample.class);
        SAMPLE_CLASSES.put(FOURCC_AAC.toString(), AACSample.class);

    }

    static private void resetTrackClasses()
    {
        TRACK_CLASSES = new HashMap<>();
        TRACK_CLASSES.put(FOURCC_AUDIO_TRACK.toString(), AudioTrack.class);
        TRACK_CLASSES.put(FOURCC_IMAGE_SEQUENCE.toString(), ImageSequence.class);
        TRACK_CLASSES.put(FOURCC_VIDEO_TRACK.toString(), VideoTrack.class);
    }

    private List<EntityGroup> getEntityGroupsPrivate(FourCC type)
            throws Exception
    {
        checkState();
        int entityGroupCount = getEntityGroupCountNative();
        List<EntityGroup> entityGroups = new ArrayList<>(entityGroupCount);
        for (int index = 0; index < entityGroupCount; index++)
        {
            EntityGroup group = getEntityGroupNative(index);
            if (type == null || group.getType().equals(type))
            {
                entityGroups.add(group);
            }
        }
        return entityGroups;
    }

    /**
     * Verifies that the HEIF instance hasn't already been released and throws an exception if it has been.
     * @throws Exception
     */
    private void checkState()
            throws Exception
    {
        if (mNativeHandle == 0)
        {
            throw new Exception(ErrorHandler.OBJECT_ALREADY_DELETED, "Object already deleted");
        }
    }

    private void checkParameter(Object parameter)
            throws Exception
    {
        if (parameter == null)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Parameter is null");
        }
        else if (parameter instanceof Base)
        {
            Base parameterAsBase = (Base) parameter;
            if (parameterAsBase.mNativeHandle == 0)
            {
                throw new Exception(ErrorHandler.OBJECT_ALREADY_DELETED, "Object already deleted");
            }
            else if (parameterAsBase.getParentHEIF() != this)
            {
                throw new Exception(ErrorHandler.WRONG_HEIF_INSTANCE, "Incorrect HEIF instance");
            }
        }
    }

    private void checkParameterAllowNull(Object parameter)
            throws Exception
    {
        if (parameter != null)
        {
            checkParameter(parameter);
        }
    }

    static private Map<String, Class> ITEM_CLASSES = null;
    static private Map<String, Class> ITEM_PROPERTY_CLASSES = null;
    static private Map<String, Class> DECODER_CONFIG_CLASSES = null;
    static private Map<String, Class> SAMPLE_CLASSES = null;
    static private Map<String, Class> ENTITY_GROUP_CLASSES = null;
    static private Map<String, Class> TRACK_CLASSES = null;

    static
    {
        resetItemClasses();
        resetItemPropertyClasses();
        resetDecoderConfigClasses();
        resetEntityGroupClasses();
        resetSampleClasses();
        resetTrackClasses();
    }

    private Base createBase(Class clazz, long nativeHandle)
    {
        try
        {
            if (clazz != null)
            {
                Constructor constructor = clazz.getDeclaredConstructor(HEIF.class, Long.TYPE);
                return (Base) constructor.newInstance(this, nativeHandle);
            }
            else
            {
                return null;
            }
        }

        catch (java.lang.Exception ex)
        {
            ex.printStackTrace();
            return null;
        }
    }

    private Base createItem(String fourCC, long nativeHandle)
    {
        Class itemClass = ITEM_CLASSES.get(fourCC);
        if (itemClass != null)
        {
            return createBase(itemClass, nativeHandle);
        }
        else
        {
            return null;
        }
    }

    private Base createItemProperty(String fourCC, long nativeHandle)
    {
        Class itemClass = ITEM_PROPERTY_CLASSES.get(fourCC);
        if (itemClass == null)
        {
            itemClass = RawProperty.class;
        }
        return createBase(itemClass, nativeHandle);
    }

    private Base createDecoderConfig(String fourCC, long nativeHandle)
    {
        Class itemClass = DECODER_CONFIG_CLASSES.get(fourCC);
        if (itemClass == null)
        {
            itemClass = DecoderConfig.class;
        }
        return createBase(itemClass, nativeHandle);
    }

    private Base createSample(String fourCC, long nativeHandle)
    {
        Class itemClass = SAMPLE_CLASSES.get(fourCC);
        if (itemClass != null)
        {
            return createBase(itemClass, nativeHandle);
        }
        else
        {
            return null;
        }
    }

    private Base createEntityGroup(String fourCC, long nativeHandle)
    {
        Class itemClass = ENTITY_GROUP_CLASSES.get(fourCC);
        if (itemClass == null)
        {
            itemClass = EntityGroup.class;
        }
        return createBase(itemClass, nativeHandle);
    }

    private Base createTrack(String fourCC, long nativeHandle)
    {
        Class itemClass = TRACK_CLASSES.get(fourCC);
        if (itemClass != null)
        {
            return createBase(itemClass, nativeHandle);
        }
        else
        {
            return null;
        }
    }

    private native void createInstanceNative();

    private native void destroyInstanceNative();

    private native void loadNative(String filename);

    private native void loadStreamNative(InputStream stream);

    private native void saveNative(String filename);

    private native void saveStreamNative(OutputStream stream);

    private native int getItemCountNative();

    private native Item getItemNative(int itemIndex);

    private native int getImageCountNative();

    private native ImageItem getImageNative(int itemIndex);

    private native int getMasterImageCountNative();

    private native ImageItem getMasterImageNative(int itemIndex);

    private native int getItemsOfTypeCountNative(String type);

    private native Item getItemOfTypeNative(String type, int itemIndex);

    private native ImageItem getPrimaryItemNative();

    private native void setPrimaryItemNative(ImageItem item);

    private native String getMajorBrandNative();

    private native void setMajorBrandNative(String brand);

    private native int getCompatibleBrandsCountNative();

    private native String getCompatibleBrandNative(int index);

    private native void addCompatibleBrandNative(String brand);

    private native void removeCompatibleBrandNative(String brand);

    private native int getPropertyCountNative();

    private native ItemProperty getPropertyNative(int index);

    private native int getTrackCountNative();

    private native Track getTrackNative(int index);

    private native int getAlternativeTrackGroupCountNative();
    private native AlternativeTrackGroup getAlternativeTrackGroupNative(int index);

    private native int getEntityGroupCountNative();
    private native EntityGroup getEntityGroupNative(int index);
}
