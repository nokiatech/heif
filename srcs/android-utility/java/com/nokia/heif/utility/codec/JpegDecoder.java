/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.heif.utility.codec;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.media.MediaFormat;
import android.util.Log;
import android.view.Surface;

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.Exception;
import com.nokia.heif.HEIF;
import com.nokia.heif.Sample;

import java.nio.ByteBuffer;
import java.util.Arrays;


public class JpegDecoder implements Decoder
{
    private final static String TAG = "JpegDecoder";

    private byte[] mDecoderConfig;

    private static MediaFormat createInputFormat(CodedImageItem imageItem)
            throws Exception
    {
        Log.d(TAG, "createInputFormat()");
        MediaFormat format = new MediaFormat();
        String codecName = "";
        if (imageItem.getDecoderCodeType().equals(HEIF.FOURCC_HEVC))
        {
            codecName = "video/hevc";
        }
        format.setInteger(MediaFormat.KEY_WIDTH, imageItem.getSize().width);
        format.setInteger(MediaFormat.KEY_HEIGHT, imageItem.getSize().height);
        format.setString(MediaFormat.KEY_MIME, codecName);
        format.setByteBuffer("csd-0", ByteBuffer.wrap(imageItem.getDecoderConfig().getConfig()));
        return format;
    }

    public JpegDecoder(CodedImageItem imageItem)
            throws Exception
    {
        Log.d(TAG, "JpegDecoder()");
        mDecoderConfig = imageItem.getDecoderConfig().getConfig();
    }

    public void release()
    {
    }

    public void queueDecode(DecodeClient client)
    {
        try
        {
            ByteBuffer bb = client.getItemData();
            byte[] data = new byte[bb.remaining()];
            bb.get(data);

            /* ImageDecoder wasn't introduced until Android 9, so use BitmapFactory here */
            final BitmapFactory.Options options = new BitmapFactory.Options();
            Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);

            Surface surface = client.getOutputSurface();
            client.setDefaultSize(bitmap.getWidth(), bitmap.getHeight());

            Canvas canvas = surface.lockCanvas(null);
            canvas.drawBitmap(bitmap, 0, 0, null);
            surface.unlockCanvasAndPost(canvas);
        }
        catch (Exception e) {
            Log.d(TAG, "Error when decoding JPEG: " + e.getMessage());
        }
    }

    public void queueDecode(DecodeClient client, Sample sample, boolean doRender)
    {
        Log.e(TAG, "This queueDecode() should not be called.");
    }

    public boolean compatible(byte[] decoderConfig)
    {
        return Arrays.equals(mDecoderConfig, decoderConfig);
    }

    public Result decodeImageSynchronous(ByteBuffer inputData, MediaFormat inputFormat)
    {
        return new Result();
    }
}
