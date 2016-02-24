/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#include <cstdint>
#include <string>
#include <vector>

/** @brief ISOBMFF compliant stream manipulation class
 *  @details This class provides the necessary functionality to generate, modify and read an ISOBMFF compliant byte stream.
 */
class BitStream
{
public:
    BitStream();
    BitStream(const std::vector<std::uint8_t>& strData);
    ~BitStream();

    /// @return Current access position as a byte offset value
    std::uint32_t getPos() const;

    /** @brief Set access position of the  bitstream
     *  @param [in] position The new position in bytes, counted from beginning of the bitstream */
    void setPosition(unsigned int position);

    /// @return Current storage size of the bitsteam in bytes
    std::uint32_t getSize() const;

    /** @brief Sets or updates the storage size of the bitstream
     *  @param [in] newSize Byte size of the bitstream */
    void setSize(unsigned int newSize);

    /// @return Reference to the stored data inside the bitstream
    const std::vector<std::uint8_t>& getStorage() const;

    ///@brief Reset any bit and byte offsets used in the bitstream access
    void reset();

    ///@brief Clear the stored data in the bitstream
    void clear();

    /** @brief Increments the byte offset pointer
     *  @param [in] x increment value in bytes */
    void skipBytes(unsigned int x);

    /** @brief Sets the value of a byte in the bitstream data storage
     *  @param [in] offset byte offset location in the bitstream data storage
     *  @param [in] byte value to be set */
    void setByte(unsigned int offset, std::uint8_t byte);

    /** @brief Writes bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage
     *  @param [in] len  number of bits to be written to the bitstream data storage */
    void writeBits(unsigned int bits, unsigned int len);

    /** @brief Writes 8 bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage */
    void write8Bits(unsigned int bits);

    /** @brief Writes 16 bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage */
    void write16Bits(unsigned int bits);

    /** @brief Writes 24 bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage */
    void write24Bits(unsigned int bits);

    /** @brief Writes 32 bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage */
    void write32Bits(unsigned int bits);

    /** @brief Writes 64 bits to the bitstream data storage
     *  @param [in] bits bits to be written to the bitstream data storage */
    void write64Bits(unsigned long long int bits);

    /** @brief Writes an array of 8 bit values to the bitstream data storage
     *  @param [in] bits vector of bits to be written to the bitstream data storage
     *  @param [in] len number of 8 bit elements to be written to the bitstream data storage
     *  @param [in] srcOffset offset location to start reading 8 bit elements in the bits vector */
    void write8BitsArray(const std::vector<std::uint8_t>& bits, unsigned int len, unsigned int srcOffset = 0);

    ///@brief Writes a non-zero-terminated string to the bitstream data storage
    void writeString(const std::string& srcString);

    ///@brief Writes a zero-terminated string to the bitstream data storage
    void writeZeroTerminatedString(const std::string& srcString);

    /** @brief Get a byte value from the bitstream data storage
     *  @param [in] offset byte offset location in the bitstream data storage
     *  @return byte value as an unsigned integer*/
    std::uint8_t getByte(unsigned int offset) const;

    /** @brief Read a specific amount of bits from the bitstream data storage
     *  @param [in] len number of bits to read
     *  @return value of bits read as an unsigned integer */
    unsigned int readBits(int len);

    /// @return read 8 bits as unigned integer
    std::uint8_t read8Bits();

    /// @return read 16 bits as unigned integer
    std::uint16_t read16Bits();

    /// @return read 24 bits as unigned integer
    unsigned int read24Bits();

    /// @return read 32 bits as unigned integer
    unsigned int read32Bits();

    /// @return read 64 bits as unigned long long
    unsigned long long read64Bits();

    /** @brief Reads an array of 8 bit values from the bitstream data storage
     *  @param [in] len number of 8 bit elements to be read from the bitstream data storage
     *  @param [out] bits vector of bits read */
    void read8BitsArray(std::vector<std::uint8_t>& bits, unsigned int len);

    /** @brief Reads a string of defined length from the bitstream data storage
     *  @param [in] len number of byte characters to be read from the bitstream data storage
     *  @param [out] dstString the string that contains the read characters */
    void readStringWithLen(std::string& dstString, unsigned int len);

    /** @brief Reads a string of defined length from a defined byte offset from the bitstream data storage
     *  @param [in] len number of byte characters to be read from the bitstream data storage
     *  @param [in] pos byte offset location to start reading the characters
     *  @param [out] dstString the string that contains the read characters */
    void readStringWithPosAndLen(std::string& dstString, unsigned int pos, unsigned int len);

    /** @brief Reads a zero-terminated-string from the bitstream data storage
     *  @param [out] dstString zero-terminated-string that contains the read characters */
    void readZeroTerminatedString(std::string& dstString);

    /** @brief Reads an Exponential-Golomb code from the bitstream data storage
     *  @return Exponential-Golomb code as unsigned integer*/
    unsigned int readExpGolombCode();

    /** @brief Reads a signed Exponential-Golomb code from the bitstream data storage
     *  @return Exponential-Golomb code as signed integer*/
    int readSignedExpGolombCode();

    /** Get BitStream of a sub Box. First 32 bits read defines size, next 32 bits boxType.
     * Read pointer of BitStream is incremented by size.
     * @param boxType [out] Type of the read sub-box
     * @return Sub-box BitStream */
    BitStream readSubBoxBitStream(std::string& boxType);

    ///@return Number of bytes left to process in the current bitstream data storage
    unsigned int numBytesLeft() const;

    /**
     * Clear destination BitStream and copy part of BitStream to it.
     *
     * @param [in] begin Start offset from the bitstream begin
     * @param [in] end   End offset from the bitstream begin
     * @param [out] dest Destination BitStream. */
    void extract(int begin, int end, BitStream& dest) const;

private:
    ///@brief Bitstream data storage as a vector of unsigned integers
    std::vector<std::uint8_t> mStorage;

    ///@brief the value of the current processed byte
    unsigned int mCurrByte;

    ///@brief cursor that points to the current byte location of the bitstream data storage
    unsigned int mByteOffset;

    ///@brief cursor that points to the current bit location of the current processed byte
    unsigned int mBitOffset;

    ///@brief Indicates whether data storage byte vector is provided during construction (FALSE) or not (TRUE)
    bool mStorageAllocated;
};

#endif /* end of include guard: BITSTREAM_HPP */

