/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2018-2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.heif.utility.codec;

import android.media.MediaFormat;
import android.view.Surface;

import com.nokia.heif.Exception;
import com.nokia.heif.Sample;

import java.nio.ByteBuffer;


public interface Decoder
{
    interface DecodeClient
    {
        ByteBuffer getItemData() throws Exception;

        Surface getOutputSurface();
        void setDefaultSize(int width, int height);

        void notifyDecodeReady();

        int getTextureId();

        void setAvailable(boolean available);
    }

    class Result
    {
        public byte[] imageData;
        public MediaFormat outputFormat;
    }

    void release();

    void queueDecode(DecodeClient client);
    void queueDecode(DecodeClient client, Sample sample, boolean doRender);
    boolean compatible(byte[] decoderConfig);
}
