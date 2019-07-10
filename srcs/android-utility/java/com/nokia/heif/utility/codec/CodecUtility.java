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

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.Exception;
import com.nokia.heif.HEIF;
import com.nokia.heif.VideoSample;
import com.nokia.heif.VideoTrack;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class CodecUtility
{
    private List<Decoder> mStillDecoders = new ArrayList<>();
    private List<Decoder> mVideoDecoders = new ArrayList<>();

    public Decoder getDecoder(CodedImageItem imageItem)
            throws Exception
    {
        Decoder decoder = null;

        for (Decoder dec : mStillDecoders)
        {
            if (dec.compatible(imageItem.getDecoderConfig().getConfig()))
            {
                decoder = dec;
                break;
            }
        }
        if (decoder == null)
        {
            try
            {
                if (imageItem.getDecoderCodeType().equals(HEIF.FOURCC_HEVC) ||
                        imageItem.getDecoderCodeType().equals(HEIF.FOURCC_AVC))
                {
                    decoder = new HwDecoder(imageItem);
                } else if (imageItem.getDecoderCodeType().equals(HEIF.FOURCC_JPEG))
                {
                    decoder = new JpegDecoder(imageItem);
                }

                mStillDecoders.add(decoder);
            }
            catch (IOException e)
            {
                e.printStackTrace();
                return null;
            }
        }
        return decoder;
    }

    public Decoder getDecoder(VideoTrack track)
            throws Exception
    {
        List<VideoSample> samples = track.getVideoSamples();
        VideoSample sample = samples.get(0);

        try
        {
            Decoder decoder = new HwDecoder(sample);
            mVideoDecoders.add(decoder);
            return decoder;
        }
        catch (IOException e)
        {
            e.printStackTrace();
            return null;
        }
    }

    public void releaseResources()
    {
        for (Decoder dec : mStillDecoders)
        {
            dec.release();
        }
        for (Decoder dec : mVideoDecoders)
        {
            dec.release();
        }

        mStillDecoders.clear();
        mVideoDecoders.clear();
    }
}
