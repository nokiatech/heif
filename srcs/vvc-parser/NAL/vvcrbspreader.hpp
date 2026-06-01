#ifndef VVCRBSPREADER_HPP
#define VVCRBSPREADER_HPP

#include "../../common/bitstream.hpp"
#include "../../common/customallocator.hpp"
#include <stddef.h>
#include <stdint.h>


struct NalHeader {
    uint8_t forbiddenZeroBit;
    uint8_t nuhReservedZeroBit;
    uint8_t nuhLayerId;
    uint8_t nalUnitType;
    uint8_t temporalIdPlus1;
};

class VvcRbspReader {
public:
    VvcRbspReader();
    
    /// Prepare NALU for parsing: skip startcode, convert to RBSP, parse header
    bool prepare(const Vector<uint8_t>& nalUnit);
    
    /// Access NAL header info
    const NalHeader& getNalHeader() const { return mHeader; }
    
    // Reading methods (delegate to BitStream)
    uint32_t readBits(uint32_t n);
    bool readFlag();
    uint32_t readUE();  // ue(v) - uses readExpGolombCode()
    int32_t readSE();   // se(v) - uses readSignedExpGolombCode()
    
    // Position tracking
    uint64_t getBitsRead() const;
    void byteAlign();
    
    // Emulation prevention info (for subpicIdBitPos calculation)
    bool hasEmulationBytesBefore(uint64_t bitPos) const;
    
private:
    /// Remove emulation prevention bytes from payload
    static Vector<uint8_t> convertToRbsp(
        const uint8_t* data, size_t size,
        Vector<uint32_t>& emulationLocations);
    
    ISOBMFF::BitStream mBitstream;
    NalHeader mHeader;
    Vector<uint32_t> mEmulationByteLocations;
    uint64_t mBitsRead = 0;
};

#endif
