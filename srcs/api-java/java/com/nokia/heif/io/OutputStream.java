/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 *
 */

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
