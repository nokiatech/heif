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
 * CleanApertureProperty
 */
public final class CleanApertureProperty extends TransformativeProperty
{

    /**
     * Fractional number class used by CleanApertureProperty
     */
    static public class Fraction
    {
        public Fraction(int numerator, int denominator)
        {
            this.numerator = numerator;
            this.denominator = denominator;
        }

        public final int numerator;
        public final int denominator;

        @Override
        public boolean equals(Object obj)
        {
            if (obj instanceof Fraction)
            {
                Fraction compareTo = (Fraction) obj;
                return compareTo.denominator == this.denominator
                        && compareTo.numerator == this.numerator;
            }
            else
            {
                return false;
            }
        }
    }

    /**
     * Creates a new CleanApertureProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public CleanApertureProperty(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected CleanApertureProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the width
     * @return Width as a fraction
     * @throws Exception
     */
    public Fraction getWidth()
            throws Exception
    {
        checkState();
        return getWidthNative();
    }

    /**
     * Sets the width
     * @param width Width as a fraction
     * @throws Exception
     */
    public void setWidth(Fraction width)
            throws Exception
    {
        checkState();
        setWidthDenNative(width.denominator);
        setWidthNumNative(width.numerator);
    }

    /**
     * Returns the height
     * @return The height as a fraction
     * @throws Exception
     */
    public Fraction getHeight()
            throws Exception
    {
        checkState();
        return getHeightNative();
    }

    /**
     * Sets the height
     * @param height Height as a fraction
     * @throws Exception
     */
    public void setHeight(Fraction height)
            throws Exception
    {
        checkState();
        setHeightDenNative(height.denominator);
        setHeightNumNative(height.numerator);
    }

    /**
     * Gets the horizontal offset
     * @return Horizontal offset as a fraction
     * @throws Exception
     */
    public Fraction getHorizontalOffset()
            throws Exception
    {
        checkState();
        return getHorizontalOffsetNative();
    }

    /**
     * Sets the horizontal offset
     * @param horizontalOffset Horizontal offset as a fraction
     * @throws Exception
     */
    public void setHorizontalOffset(Fraction horizontalOffset)
            throws Exception
    {
        checkState();
        setHorizontalOffsetDenNative(horizontalOffset.denominator);
        setHorizontalOffsetNumNative(horizontalOffset.numerator);
    }

    /**
     * Returns the vertical offset
     * @return Vertical offset as a fraction
     * @throws Exception
     */
    public Fraction getVerticalOffset()
            throws Exception
    {
        checkState();
        return getVerticalOffsetNative();
    }

    /**
     * Sets the vertical offset
     * @param verticalOffset Vertical offset as a fraction
     * @throws Exception
     */
    public void setVerticalOffset(Fraction verticalOffset)
            throws Exception
    {
        checkState();
        setVerticalOffsetNumNative(verticalOffset.numerator);
        setVerticalOffsetDenNative(verticalOffset.denominator);
    }

    private CleanApertureProperty.Fraction createFraction(int numerator, int denominator)
    {
        return new CleanApertureProperty.Fraction(numerator, denominator);
    }

    private native long createContextNative(HEIF heif);

    private native Fraction getWidthNative();

    private native void setWidthNumNative(int value);

    private native void setWidthDenNative(int value);

    private native Fraction getHeightNative();

    private native void setHeightNumNative(int value);

    private native void setHeightDenNative(int value);

    private native Fraction getHorizontalOffsetNative();

    private native void setHorizontalOffsetNumNative(int value);

    private native void setHorizontalOffsetDenNative(int value);

    private native Fraction getVerticalOffsetNative();

    private native void setVerticalOffsetNumNative(int value);

    private native void setVerticalOffsetDenNative(int value);

}
