/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef BITSTREAM_HPP
#define BITSTREAM_HPP

#include <cstdint>
#include "customallocator.hpp"

#include "fourccint.hpp"

namespace ISOBMFF
{
    /** @brief ISOBMFF compliant stream manipulation class.
     *  @details This class provides the necessary functionality to generate, modify and read an ISOBMFF compliant byte stream.
     */
    class BitStream
    {
    public:
        BitStream();
        BitStream(const Vector<std::uint8_t>& strData);
        BitStream(const BitStream&) = default;
        BitStream& operator=(const BitStream&) = default;
        BitStream(BitStream&&);
        BitStream& operator=(BitStream&&);
        ~BitStream();

        union FloatToUint
        {
            float asFloat;
            uint32_t asUint;
        };

        /// @return Current access position as a byte offset value
        std::uint64_t getPos() const;

        /** @brief Set access position of the  bitstream
         *  @param position The new position in bytes, counted from beginning of the bitstream. */
        void setPosition(std::uint64_t position);

        /// @return Current storage size of the bitsteam in bytes
        std::uint64_t getSize() const;

        /** @brief Sets or updates the storage size of the bitstream
         *  @param newSize Byte size of the bitstream */
        void setSize(std::uint64_t newSize);

        /// @return Reference to the stored data inside the bitstream
        const Vector<std::uint8_t>& getStorage() const;

        /// @return Reference to the stored data inside the bitstream
        Vector<std::uint8_t>& getStorage();

        /// @brief Reset any bit and byte offsets used in the bitstream access
        void reset();

        /// @brief Clear the stored data in the bitstream
        void clear();

        /** @brief Increments the byte offset pointer
         *  @param count Increment value in bytes */
        void skipBytes(std::uint64_t count);

        /** @brief Sets the value of a byte in the bitstream data storage.
         *  @param offset Byte offset location in the bitstream data storage.
         *  @param byte   Value to be set. */
        void setByte(std::uint64_t offset, std::uint8_t byte);

        /** @brief Get a byte value from the bitstream data storage.
         *  @param offset Byte offset location in the bitstream data storage.
         *  @return Byte value as an unsigned integer. */
        std::uint8_t getByte(std::uint64_t offset) const;

        /** @brief Writes bits to the bitstream data storage.
         *  @param bits Bits to be written to the bitstream data storage.
         *  @param len  Number of bits to be written to the bitstream data storage. */
        void writeBits(std::uint64_t bits, std::uint32_t len);

        /** @brief Writes 8 bits to the bitstream data storage.
         *  @param bits Bits to be written to the bitstream data storage. */
        void write8Bits(std::uint8_t bits);

        /** @brief Writes the contents of another bitstream.
         *  @param bitStr Bitstream to write. */
        void writeBitStream(const BitStream& bitStr);

        /** @brief Writes 16 bits to the bitstream data storage
         *  @param [in] bits bits to be written to the bitstream data storage */
        void write16Bits(std::uint16_t bits);

        /** @brief Writes 24 bits to the bitstream data storage
         *  @param [in] bits bits to be written to the bitstream data storage */
        void write24Bits(std::uint32_t bits);

        /** @brief Writes 32 bits to the bitstream data storage
         *  @param [in] bits bits to be written to the bitstream data storage */
        void write32Bits(std::uint32_t bits);

        /** @brief Writes 64 bits to the bitstream data storage
         *  @param [in] bits bits to be written to the bitstream data storage */
        void write64Bits(std::uint64_t bits);

        /** @brief Writes an array of 8 bit values to the bitstream data storage
         *  @param [in] bits vector of bits to be written to the bitstream data storage
         *  @param [in] len number of 8 bit elements to be written to the bitstream data storage
         *  @param [in] srcOffset offset location to start reading 8 bit elements in the bits vector */
        void write8BitsArray(const Vector<std::uint8_t>& bits, std::uint64_t len, std::uint64_t srcOffset = 0);

        /// @brief Writes a non-zero-terminated string to the bitstream data storage
        void writeString(const String& srcString);

        /// @brief Writes a zero-terminated string to the bitstream data storage
        void writeZeroTerminatedString(const String& srcString);

        /// @brief Writes a float as 32bit value to the bitstream data storage
        void write32BitFloat(float value);

        /** @brief Read a specific amount of bits from the bitstream data storage
         *  @param [in] len number of bits to read
         *  @return value of bits read as an unsigned integer */
        std::uint32_t readBits(const std::uint32_t len);

        /// @return read 8 bits as unigned uint8_t
        std::uint8_t read8Bits();

        /// @return read 16 bits as unigned uint16_t
        std::uint16_t read16Bits();

        /// @return read 24 bits as unigned integer
        std::uint32_t read24Bits();

        /// @return read 32 bits as unigned integer
        std::uint32_t read32Bits();

        /// @return read 64 bits as unigned long long
        std::uint64_t read64Bits();

        /** @brief Reads an array of 8 bit values from the bitstream data storage
         *  @param [in] len number of 8 bit elements to be read from the bitstream data storage
         *  @param [out] bits vector of bits read */
        void read8BitsArray(Vector<std::uint8_t>& bits, std::uint64_t len);

        /** @brief Reads an array of 8 bit values from the bitstream data storage
         *  @param [in] len number of 8 bit elements to be read from the bitstream data storage
         *  @param [out] buffer data buffer pointer where data is copied. */
        void readByteArrayToBuffer(char* buffer, std::uint64_t len);

        /** @brief Reads a string of defined length from the bitstream data storage
         *  @param [in] len number of byte characters to be read from the bitstream data storage
         *  @param [out] dstString the string that contains the read characters */
        void readStringWithLen(String& dstString, std::uint32_t len);

        /** @brief Reads a string of defined length from a defined byte offset from the bitstream data storage
         *  @param [in] len number of byte characters to be read from the bitstream data storage
         *  @param [in] pos byte offset location to start reading the characters
         *  @param [out] dstString the string that contains the read characters */
        void readStringWithPosAndLen(String& dstString, std::uint64_t pos, std::uint32_t len);

        /** @brief Reads a zero-terminated-string from the bitstream data storage
         *  @param [out] dstString zero-terminated-string that contains the read characters */
        void readZeroTerminatedString(String& dstString);

        /// @return read 32 bits as float
        float read32BitFloat();

        /** @brief Reads an Exponential-Golomb code from the bitstream data storage
         *  @return Exponential-Golomb code as unsigned integer*/
        uint32_t readExpGolombCode();

        /** @brief Reads a signed Exponential-Golomb code from the bitstream data storage
         *  @return Exponential-Golomb code as signed integer*/
        int32_t readSignedExpGolombCode();

        /** Get BitStream of a sub Box. First 32 bits read defines size, next 32 bits boxType.
         * Read pointer of BitStream is incremented by size.
         * @param boxType [out] Type of the read sub-box
         * @return Sub-box BitStream */
        BitStream readSubBoxBitStream(FourCCInt& boxType);

        /// @return Number of bytes left to process in the current bitstream data storage
        std::uint64_t numBytesLeft() const;

        /**
         * Clear destination BitStream and copy part of BitStream to it.
         *
         * @param [in] begin Start offset from the bitstream begin
         * @param [in] end   End offset from the bitstream begin
         * @param [out] dest Destination BitStream. */
        void extract(std::uint64_t begin, std::uint64_t end, BitStream& dest) const;

        /// @return True if current bit offset location inside a byte is zero, false otherwise.
        bool isByteAligned() const;

    private:
        /// @brief Bitstream data storage as a vector of unsigned integers
        Vector<std::uint8_t> mStorage;

        /// @brief The value of the current processed byte
        unsigned int mCurrByte;

        /// @brief Cursor that points to the current byte location of the bitstream data storage
        std::uint64_t mByteOffset;

        /// @brief Cursor that points to the current bit location of the current processed byte.
        unsigned int mBitOffset;

        /// @brief Indicates whether data storage byte vector is provided during construction (false) or not (true).
        bool mStorageAllocated;
    };
}  // namespace ISOBMFF

using BitStream = ISOBMFF::BitStream;

#endif /* end of include guard: BITSTREAM_HPP */
