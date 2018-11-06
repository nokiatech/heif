package com.nokia.heif.io;

import java.nio.ByteBuffer;

public interface OutputStream
{
    /**
     * Seeks the stream to the given position
     * @param position The location in the stream to seek to
     */
    void seek(long position);

    /**
     * Returns the current position of the stream
     * @return
     */
    long position();

    void write(ByteBuffer buffer, long size);

    void clear();


}
