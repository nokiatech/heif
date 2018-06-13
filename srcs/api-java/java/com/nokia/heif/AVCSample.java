package com.nokia.heif;

/**
 * A sample which contains AVC encoded data
 */
public class AVCSample extends VideoSample
{
    /**
     * Creates a new AVCSample to the given HEIF instance
     * Also creates a corresponding AVCDecoderConfig object with the given decoder config
     *
     * @param heif The parent HEIF instance for the new object
     * @param decoderConfig Byte array containing the decoder config
     * @param data Byte array containing the sample data as an encoded AVC frame
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public AVCSample(HEIF heif, byte[] decoderConfig, byte[] data, long duration)
            throws Exception
    {
        this(heif);
        try
        {
            DecoderConfig decConfig = new AVCDecoderConfig(heif, decoderConfig);

            setDecoderConfig(decConfig);
            setSampleData(data);
            setDuration(duration);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Creates a new AVCSample to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param config Decoder config for the sample
     * @param data Byte array containing the sample data as an encoded AAC frame
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public AVCSample(HEIF heif, AVCDecoderConfig config, byte[] data, long duration)
            throws Exception
    {
        this(heif);
        try
        {
            setDecoderConfig(config);
            setSampleData(data);
            setDuration(duration);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Private constructor
     *
     * @param heif The parent HEIF instance for the new object
     * @throws Exception
     */
    private AVCSample(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected AVCSample(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder config of the sample
     *
     * @return AVCDecoderConfig
     * @throws Exception
     */
    public AVCDecoderConfig getDecoderConfig()
            throws Exception
    {
        return (AVCDecoderConfig) super.getDecoderConfig();
    }

    @Override
    protected boolean checkDecoderConfigType(DecoderConfig config)
    {
        return config instanceof AVCDecoderConfig;
    }

    private native long createContextNative(HEIF heif);
}
