/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2019-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.heif.utility.codec;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.util.Log;

import com.nokia.heif.*;
import com.nokia.heif.Exception;

import java.nio.ByteBuffer;

public class DecoderUtil
{
    private final static String TAG = "DecoderUtil";

    public static MediaFormat createInputFormat(Base decodable)
            throws Exception
    {
        MediaFormat format = new MediaFormat();
        String codecName;
        Size size;
        FourCC type;
        DecoderConfig config;

        if (decodable instanceof VideoSample)
        {
            VideoSample sample = (VideoSample) decodable;
            type = sample.getType();
            size = sample.getSize();
            config = sample.getDecoderConfig();
        }
        else if (decodable instanceof CodedImageItem)
        {
            CodedImageItem image = (CodedImageItem) decodable;
            type = image.getDecoderCodeType();
            size = image.getSize();
            config = image.getDecoderConfig();
        }
        else
        {
            throw new Exception(ErrorHandler.UNDEFINED_ERROR, "Object must be a VideoSample or a CodedImageItem.");
        }

        if (type.equals(HEIF.FOURCC_HEVC))
        {
            codecName = "video/hevc";
        }
        else if (type.equals(HEIF.FOURCC_AVC))
        {
            codecName = "video/avc";
        }
        else
        {
            throw new Exception(ErrorHandler.UNDEFINED_ERROR, "Unknown encoding: " + type);
        }

        format.setInteger(MediaFormat.KEY_WIDTH, size.width);
        format.setInteger(MediaFormat.KEY_HEIGHT, size.height);
        format.setString(MediaFormat.KEY_MIME, codecName);
        format.setByteBuffer("csd-0", ByteBuffer.wrap(config.getConfig()));
        return format;
    }

    /**
     * Decide whether a CodedImageItem or a VideoSample is decodable, by configuring the decoder and
     * trying to decode it.
     *
     * @param decodable A CodedImageItem or a VideoSample
     * @return True if the phone is capable of decoding the input, false if not.
     */
    public static boolean isDecodable(Base decodable)
    {
        MediaCodec codec = null;
        try
        {
            ByteBuffer inputData = null;

            if (decodable instanceof CodedImageItem)
            {
                inputData = ((CodedImageItem) decodable).getItemData();
            }
            else if (decodable instanceof VideoSample)
            {
                inputData = ((VideoSample) decodable).getSampleData();
            }
            else
            {
                throw new java.lang.Exception("Invalid decodable object given.");
            }

            MediaFormat format = createInputFormat(decodable);
            codec = MediaCodec.createDecoderByType(format.getString(MediaFormat.KEY_MIME));
            codec.configure(format, null, null, 0);

            codec.start();

            for (; ; )
            {
                int inputBufferId = codec.dequeueInputBuffer(500);
                if (inputBufferId >= 0)
                {
                    ByteBuffer inputBuffer = codec.getInputBuffer(inputBufferId);
                    int size = inputData.remaining();
                    inputBuffer.put(inputData);
                    codec.queueInputBuffer(inputBufferId, 0, size, 0, 0);
                    break;
                }
            }

            for (; ; )
            {
                MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
                int outputBufferId = codec.dequeueOutputBuffer(bufferInfo, 500);
                if (outputBufferId >= 0)
                {
                    // decoding ok
                    break;
                }
            }
            codec.stop();
        }
        catch (java.lang.Exception e)
        {
            // decoding not possible
            Log.d(TAG, "isDecodable() failed: " + e.getMessage());
            if (codec != null) codec.release();
            return false;
        }

        codec.release();
        return true;
    }
}
