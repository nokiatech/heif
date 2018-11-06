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

import java.util.ArrayList;
import java.util.List;

/**
 * Base class for several different HEIF items
 */
public abstract class Item extends Base
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected Item(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected Item(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the type of the item
     * @return Type as a String
     * @throws Exception
     */
    public FourCC getType()
            throws Exception
    {
        checkState();
        return new FourCC(getTypeNative(), true);
    }

    public boolean isEssentialProperty(ItemProperty property)
            throws Exception
    {
        checkState();
        checkParameter(property);
        return isEssentialPropertyNative(property);
    }

    public void setEssentialProperty(ItemProperty property, boolean essential)
            throws Exception
    {
        checkState();
        checkParameter(property);
        setEssentialPropertyNative(property, essential);
    }

    /**
     * Returns a list of all the EntityGroups this item belongs to
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
     * Returns a list of all the Properties this item has
     * @return A list of the Properties
     * @throws Exception
     */
    public List<ItemProperty> getProperties()
            throws Exception
    {
        checkState();
        int count = getPropertyCountNative();
        List<ItemProperty> result = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            result.add(getPropertyNative(index));
        }
        return result;
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }

    native private String getTypeNative();

    native private void destroyContextNative();

    native private boolean isEssentialPropertyNative(ItemProperty item);
    native private void setEssentialPropertyNative(ItemProperty item, boolean essential);

    native private int getGroupCountNative();
    native private EntityGroup getGroupNative(int index);

    native private int getPropertyCountNative();
    native private ItemProperty getPropertyNative(int index);
}
