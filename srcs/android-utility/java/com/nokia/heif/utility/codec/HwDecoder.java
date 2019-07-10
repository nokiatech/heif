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

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Surface;

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.Exception;
import com.nokia.heif.Sample;
import com.nokia.heif.VideoSample;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.atomic.AtomicInteger;


public class HwDecoder implements Decoder
{
    private final static String TAG = "HwDecoder";

    private MediaFormat mInputFormat;
    private MediaCodec mCodec;
    private byte[] mDecoderConfig;
    boolean mCodecStarted;

    /**
     * Set true for still image decoders, false for video tracks and image sequences.
     * When true, decoding is done sequentially, as output surface of images might change.
     */
    boolean mStillImageDecoder;
    AtomicInteger mInputBuffersQueued = new AtomicInteger(0);

    private long mPTS = 10; // start from 10 as pts=0 is used for marking non-rendered decoding dependencies

    private class DecodeJob
    {
        DecodeClient client;
        boolean doRender;
        Sample sample;
    }

    private Surface mCurrentTargetSurface;

    private ConcurrentLinkedQueue<DecodeJob> mJobQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<Integer> mInputBufferQueue = new ConcurrentLinkedQueue<>();
    private Object mCodecStartLock = new Object();
    private Object mQueueLock = new Object();

    public HwDecoder(CodedImageItem imageItem)
            throws Exception, IOException
    {
        Log.d(TAG, "HwDecoder()");
        mDecoderConfig = imageItem.getDecoderConfig().getConfig();
        mInputFormat = DecoderUtil.createInputFormat(imageItem);
        mCodec = MediaCodec.createDecoderByType(mInputFormat.getString(MediaFormat.KEY_MIME));
        mStillImageDecoder = true;
    }

    public HwDecoder(VideoSample videoSample)
            throws Exception, IOException
    {
        Log.d(TAG, "HwDecoder()");
        mDecoderConfig = videoSample.getDecoderConfig().getConfig();
        mInputFormat = DecoderUtil.createInputFormat(videoSample);
        mCodec = MediaCodec.createDecoderByType(mInputFormat.getString(MediaFormat.KEY_MIME));
        mStillImageDecoder = false;
    }

    public void release()
    {
        try
        {
            mJobQueue.clear();
            mInputBufferQueue.clear();
            mCodec.stop();
            mCodec.release();
            mCodec = null;
            mInputFormat = null;
            mCurrentTargetSurface = null;
        }
        catch (java.lang.Exception ex)
        {
            Log.d(TAG, ex.getMessage());
            ex.printStackTrace();
        }
    }

    private void feedInputToDecoder()
    {
        //Log.d(TAG, "feedInputToDecoder()");
        if (mStillImageDecoder && mInputBuffersQueued.get() > 0)
        {
            // When coding still images, do not feed new data to the decoder until input buffer
            // queue is empty.
            return;
        }

        DecodeJob currentDecodeJob;
        int inputBufferIndex;

        synchronized (mQueueLock)
        {
            if (mJobQueue.isEmpty() || mInputBufferQueue.isEmpty())
            {
                return;
            }
            currentDecodeJob = mJobQueue.remove();
            inputBufferIndex = mInputBufferQueue.remove();
        }

        ByteBuffer inputBuffer = mCodec.getInputBuffer(inputBufferIndex);

        Surface outputSurface = currentDecodeJob.client.getOutputSurface();
        boolean failed = false;
        if (outputSurface == null)
        {
            Log.e(TAG, "null surface in feedInputToDecoder");
            failed = true;
        }
        else if (mCurrentTargetSurface != outputSurface)
        {
            Log.d(TAG, "Output surface changed");
            try
            {
                mCodec.setOutputSurface(outputSurface);
                mCurrentTargetSurface = outputSurface;
            }
            catch (IllegalArgumentException ie)
            {
                // TODO: Handle the error in some elegant way (it is possible that the surface is lost before we get here)
                Log.e(TAG, "setOutputSurface failed: " + ie.getMessage());
                ie.printStackTrace();
                failed = true;
            }
        }

        if (!failed)
        {
            ByteBuffer inputData = null;
            try
            {
                if (currentDecodeJob.sample != null)
                {
                    inputData = currentDecodeJob.sample.getSampleData();
                }
                else
                {
                    inputData = currentDecodeJob.client.getItemData();
                }
                final int size = inputData.remaining();
                inputBuffer.put(inputData);

                // If we were decoding only IDR frames, flags here could be (MediaCodec.BUFFER_FLAG_KEY_FRAME  | MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                // That is not wanted when decoding video, although without it is possible that
                // some decoders (e.g. Huawei) tend to queue a lot frames before outputting any.
                if (currentDecodeJob.doRender)
                {
                    mCodec.queueInputBuffer(inputBufferIndex, 0, size, mPTS, 0);
                }
                else
                {
                    mCodec.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
                }

                mPTS += 10;
                mInputBuffersQueued.incrementAndGet();
            }
            catch (Exception e)
            {
                e.printStackTrace();
                failed = true;
                Log.d(TAG, "Failed to decode frame during feeding data");
            }
        }
        if (failed)
        {
            Log.e(TAG, "feedInputToDecoder() failed -- output surface problem?");

            //Complete the failed job.
            currentDecodeJob.client.notifyDecodeReady();
            currentDecodeJob.client.setAvailable(true);
            //And continue to next.
            feedInputToDecoder();
        }
    }

    private void stopCodec()
    {
        Log.d(TAG, "stopDecoder()");
        if (!mCodecStarted)
        {
            return;
        }
        mCodec.stop();
        mInputBufferQueue.clear();
        mCodecStarted = false;
    }

    private void startCodec()
    {
        Log.d(TAG, "startCodec()");
        if (mCodecStarted)
        {
            //Log.d(TAG, "Codec already started");
            return;
        }
        assert (mCurrentTargetSurface == null);
        mCurrentTargetSurface = mJobQueue.peek().client.getOutputSurface();
        mCodec.setCallback(new MediaCodec.Callback()
        {
            @Override
            public void onInputBufferAvailable(@NonNull MediaCodec codec, int index)
            {
                mInputBufferQueue.add(index);
                feedInputToDecoder();
            }

            @Override
            public void onOutputBufferAvailable(@NonNull MediaCodec codec, int index, @NonNull MediaCodec.BufferInfo info)
            {
                codec.releaseOutputBuffer(index, info.presentationTimeUs != 0);
                mInputBuffersQueued.getAndDecrement();
                feedInputToDecoder();
            }

            @Override
            public void onError(@NonNull MediaCodec codec, @NonNull MediaCodec.CodecException e)
            {
                Log.e(TAG, e.getMessage());
                // TODO handle error
            }

            @Override
            public void onOutputFormatChanged(@NonNull MediaCodec codec, @NonNull MediaFormat format)
            {
                Log.e(TAG, "OutputFormatChanged: " + format.toString());
            }
        });
        mCodec.configure(mInputFormat, mCurrentTargetSurface, null, 0);
        mCodec.start();
        mCodecStarted = true;
        feedInputToDecoder();
    }

    public void queueDecode(DecodeClient client)
    {
        queueDecode(client, null, true);
    }

    public void queueDecode(DecodeClient client, Sample sample, boolean doRender)
    {
        if (mCodec == null)
        {
            Log.e(TAG, "queueDecode called before creating codec.");
            return;
        }

        DecodeJob decodeJob = new DecodeJob();
        decodeJob.doRender = doRender;
        decodeJob.sample = sample;
        decodeJob.client = client;

        mJobQueue.add(decodeJob);
        if (mCodecStarted)
        {
            feedInputToDecoder();
        }
        else
        {
            synchronized (mCodecStartLock)
            {
                startCodec();
            }
        }
    }

    public boolean compatible(byte[] decoderConfig)
    {
        return Arrays.equals(mDecoderConfig, decoderConfig);
    }

    public Result decodeImageSynchronous(ByteBuffer inputData, MediaFormat inputFormat)
            throws IOException
    {
        Result result = new Result();
        MediaCodec codec = MediaCodec.createDecoderByType(inputFormat.getString(MediaFormat.KEY_MIME));
        codec.configure(inputFormat, null, null, 0);
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
                ByteBuffer outputBuffer = codec.getOutputBuffer(outputBufferId);
                MediaFormat outputFormat = codec.getOutputFormat(outputBufferId);

                final int inputWidth = inputFormat.getInteger(MediaFormat.KEY_WIDTH);
                final int inputHeight = inputFormat.getInteger(MediaFormat.KEY_HEIGHT);

                final int outputWidth = outputFormat.getInteger(MediaFormat.KEY_WIDTH);
                final int outputHeight = outputFormat.getInteger(MediaFormat.KEY_HEIGHT);

                result.imageData = new byte[inputWidth * inputHeight * 3 / 2];
                if (inputWidth == outputWidth
                        && inputHeight == outputHeight)
                {
                    outputBuffer.get(result.imageData);
                }
                else
                {
                    final int stride = outputFormat.getInteger(MediaFormat.KEY_STRIDE);
                    final int bytesToSkip = stride - inputWidth;

                    for (int rowIndex = 0; rowIndex < inputHeight - 1; rowIndex++)
                    {
                        outputBuffer.get(result.imageData, rowIndex * inputWidth, inputWidth);
                        outputBuffer.position(outputBuffer.position() + bytesToSkip);
                    }

                    outputBuffer.get(result.imageData, inputWidth * (inputHeight - 1), inputWidth);

                    outputBuffer.position(outputWidth * outputHeight);

                    final int rowsToRead = inputHeight / 2;

                    final int offset = inputWidth * inputHeight;
                    for (int rowIndex = 0; rowIndex < rowsToRead - 1; rowIndex++)
                    {
                        outputBuffer.get(result.imageData, offset + rowIndex * inputWidth, inputWidth);
                        outputBuffer.position(outputBuffer.position() + bytesToSkip);
                    }

                    outputBuffer.get(result.imageData, offset + inputWidth * (rowsToRead - 1), inputWidth);
                }

                result.outputFormat = outputFormat;
                break;
            }
        }
        codec.stop();
        codec.release();
        return result;
    }
}
