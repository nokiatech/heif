package com.nokia.heif;

public final class HEVCSample extends VideoSample
{
    /**
     * Creates a new HEVCSample and an HEVCDecoderConfig for the sample
     *
     * @param heif The parent HEIF instance for the new object.
     * @param decoderConfig Byte array containing the decoder config.
     * @param data Byte array containing the sample data as an encoded HEVC frame
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public HEVCSample(HEIF heif, byte[] decoderConfig, byte[] data, long duration)
            throws Exception
    {
        this(heif);
        try
        {
            DecoderConfig decConfig = new HEVCDecoderConfig(heif, decoderConfig);

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
     * Creates a new HEVCSample
     *
     * @param heif The parent HEIF instance for the new object.
     * @param config The decoder config for the new object.
     * @param data The sample data
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public HEVCSample(HEIF heif, HEVCDecoderConfig config, byte[] data, long duration)
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
     * @param heif The parent HEIF instance for the new object.
     * @throws Exception
     */
    private HEVCSample(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected HEVCSample(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder config of the sample
     *
     * @return HEVCDecoderConfig
     * @throws Exception
     */
    public HEVCDecoderConfig getDecoderConfig()
            throws Exception
    {
        return (HEVCDecoderConfig) super.getDecoderConfig();
    }

    @Override
    protected boolean checkDecoderConfigType(DecoderConfig config)
    {
        return config instanceof HEVCDecoderConfig;
    }

    private native long createContextNative(HEIF heif);
}
