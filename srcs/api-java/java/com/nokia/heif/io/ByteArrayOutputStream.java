package com.nokia.heif.io;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class ByteArrayOutputStream implements OutputStream
{

    private int INITIAL_ALLOCATION_SIZE = 2 * 1024 * 1024;

    byte[] mData = new byte[INITIAL_ALLOCATION_SIZE];
    private int mPosition = 0;
    private int mTotalBytes = 0;

    public void clear()
    {
        mData = new byte[INITIAL_ALLOCATION_SIZE];
        mPosition = 0;
        mTotalBytes = 0;
    }

    public byte[] getData()
    {
        byte[] cleanedArray = new byte[mTotalBytes];
        System.arraycopy(mData, 0, cleanedArray, 0, mTotalBytes);
        return cleanedArray;
    }

    /**
     * Seeks the stream to the given position
     *
     * @param position The location in the stream to seek to
     */
    @Override
    public void seek(long position)
    {
        mPosition = (int) position;
    }

    /**
     * Returns the current position of the stream
     *
     * @return
     */
    @Override
    public long position()
    {
        return mPosition;
    }

    @Override
    public void write(ByteBuffer buffer, long size)
    {
        if (mPosition + size > mData.length)
        {
            reAllocateData((int) (mPosition + size));
        }
        buffer.get(mData, mPosition, (int) size);
        mPosition += size;
        if (mPosition > mTotalBytes)
        {
            mTotalBytes = mPosition;
        }
    }

    private void reAllocateData(int required)
    {
        if (required > mData.length)
        {
            int newLength = 0;
            // Grow the data 2x
            if (required < mData.length * 2)
            {
                newLength = mData.length * 2;
            }
            // Grow to what is needed and some buffer
            else
            {
                newLength = required + INITIAL_ALLOCATION_SIZE;
            }
            byte[] temp = mData;
            mData = new byte[newLength];
            System.arraycopy(temp, 0, mData, 0, temp.length);
        }
    }

}
