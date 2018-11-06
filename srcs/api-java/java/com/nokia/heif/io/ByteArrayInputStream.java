package com.nokia.heif.io;

import java.nio.ByteBuffer;

/**
 * Stream for loading HEIF files from a byte array
 */
public class ByteArrayInputStream
        implements InputStream
{
    private byte[] mData;

    private int mPosition = 0;

    public ByteArrayInputStream(byte[] data)
    {
        mData = data;
    }

    @Override
    public long read(ByteBuffer buffer, long size)
    {
        int readCount = (int) size;
        if (mPosition + size > mData.length) {
            readCount = mData.length - mPosition;
        }
        if (readCount > 0)
        {
            buffer.put(mData, mPosition, readCount);
        }
        mPosition += readCount;
        return readCount;
    }

    @Override
    public boolean seek(long offset)
    {
        mPosition = (int) offset;
        return true;
    }

    @Override
    public long position()
    {
        return mPosition;
    }

    @Override
    public long size()
    {
        return mData.length;
    }
}
