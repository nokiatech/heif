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

/**
 * A GridImageItem contains several ImageItems arranged in a grid
 * For a HEIF file to be valid, every cell of the GridImageItem must have an image in it.
 */
public final class GridImageItem extends DerivedImageItem
{
    /**
     * Creates a new GridImageItem to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param columns The column count for the grid
     * @param rows The row count for the grid
     * @param size The display size of the grid image
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public GridImageItem(HEIF heif, int columns, int rows, Size size)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setDimensions(columns, rows);
            setSize(size);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected GridImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Sets the cell dimensions of the GridImageItem
     * @param columns Column count for the grid
     * @param rows Row count for the grid
     * @throws Exception
     */
    public void setDimensions(int columns, int rows)
            throws Exception
    {
        checkState();
        if (columns == 0 || rows == 0)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Column or row count cannot be 0");
        }
        resizeNative(columns, rows);
    }

    /**
     * Returns the column count of the grid
     * @return Column count
     * @throws Exception
     */
    public int getColumnCount()
            throws Exception
    {
        checkState();
        return getColumnCountNative();
    }

    /**
     * Returns the row count of the grid
     * @return Row count
     * @throws Exception
     */
    public int getRowCount()
            throws Exception
    {
        checkState();
        return getRowCountNative();
    }

    /**
     * Returns the image item at the given position in the grid
     * @param column Column index
     * @param row Row Index
     * @return The ImageItem from the given indexes
     * @throws Exception
     */
    public ImageItem getImage(int column, int row)
            throws Exception
    {
        checkState();
        return getImageNative(column, row);
    }

    /**
     * Sets the image to the given position in the grid.
     * If an image has been set to the position before, this will replace it from the grid.
     * The old image will remain in the HEIF instance as a master image
     * @param column Column index
     * @param row Row index
     * @param image Image at the given position
     * @throws Exception
     */
    public void setImage(int column, int row, ImageItem image)
            throws Exception
    {
        checkState();
        checkParameterAllowNull(image);
        setImageNative(column, row, image);
    }

    /**
     * Removes the given ImageItem from the grid from whatever positions it is in.
     * @param image The image to be removed
     * @throws Exception
     */
    public void removeImage(ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        removeImageNative(image);
    }

    private native long createContextNative(HEIF heif);

    private native void resizeNative(int width, int height);

    private native int getColumnCountNative();

    private native int getRowCountNative();

    private native ImageItem getImageNative(int column, int row);

    private native void setImageNative(int column, int row, ImageItem image);

    private native void removeImageNative(ImageItem image);
}
