package com.nokia.heif;

/**
 * DecoderConfig for AAC data
 */
public final class AACDecoderConfig extends DecoderConfig
{
    /**
     * Creates a new AACDecoderConfig to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param config Byte array containing the config data
     * @throws Exception
     */
    public AACDecoderConfig(HEIF heif, byte[] config)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setConfig(config);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected AACDecoderConfig(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    private native long createContextNative(HEIF heif);
}
