
#include "vvcrbspreader.hpp"
#include "../../common/bitstream.hpp"
#include "../../common/customallocator.hpp"
#include <vector>

namespace {
    // Helper to find start code length (0x000001 = 3 bytes, 0x00000001 = 4 bytes)
    uint32_t findStartCodeLen(const Vector<uint8_t>& nalUnit) {
        if (nalUnit.size() >= 4 && nalUnit[0] == 0 && nalUnit[1] == 0 && nalUnit[2] == 0 && nalUnit[3] == 1) {
            return 4;
        }
        if (nalUnit.size() >= 3 && nalUnit[0] == 0 && nalUnit[1] == 0 && nalUnit[2] == 1) {
            return 3;
        }
        return 0;  // No start code
    }
}

VvcRbspReader::VvcRbspReader() 
    : mBitstream(),
      mHeader(),
      mEmulationByteLocations()
      
{
}


Vector<uint8_t> VvcRbspReader::convertToRbsp(const uint8_t* data, size_t size, Vector<uint32_t>& emulationLocations){
    Vector<uint8_t> rbsp;
    rbsp.reserve(size);
    emulationLocations.clear();
    
    uint32_t zeroCount = 0;
    for(size_t pos = 0; pos < size; ++pos){

        uint8_t byte = data[pos];
        
        // Detect emulation prevention: 00 00 03 XX
        if(zeroCount == 2 && byte == 0x03){
            emulationLocations.push_back(static_cast<uint32_t>(pos));
            zeroCount = 0;
            
            // Next byte after 0x03 (if exists)
            if(++pos < size){
                byte = data[pos];
                rbsp.push_back(byte);
                zeroCount = (byte == 0x00) ? 1 : 0;
            }
        }else{
            rbsp.push_back(byte);
            zeroCount = byte == 0x00 ? zeroCount + 1 : 0;
        }
    }
    return rbsp;
}


bool VvcRbspReader::prepare(const Vector<uint8_t>& nalUnit) {
    // Skip start code
    uint32_t startCodeLen = findStartCodeLen(nalUnit);  // Reuse existing
    const uint8_t* payload = nalUnit.data() + startCodeLen;
    size_t payloadSize = nalUnit.size() - startCodeLen;
    
    if (payloadSize < 2) return false;  // Need at least NAL header
    
    // Convert to RBSP
    Vector<uint8_t> rbsp = convertToRbsp(payload, payloadSize, mEmulationByteLocations);
    
    // Parse NAL header from first 2 bytes
    mHeader.forbiddenZeroBit = (rbsp[0] >> 7) & 0x01;
    mHeader.nuhReservedZeroBit = (rbsp[0] >> 6) & 0x01;
    mHeader.nuhLayerId = rbsp[0] & 0x3F;
    mHeader.nalUnitType = (rbsp[1] >> 3) & 0x1F;
    mHeader.temporalIdPlus1 = rbsp[1] & 0x07;
    
    // Setup bitstream for reading (skip 2-byte NAL header)
    Vector<uint8_t> rbspPayload(rbsp.begin() + 2, rbsp.end());
    mBitstream = ISOBMFF::BitStream(rbspPayload);
    mBitsRead = 0;
    
    return true;
}

uint32_t VvcRbspReader::readBits(uint32_t n)
{
    mBitsRead += n;
    return mBitstream.readBits(n);
}

bool VvcRbspReader::readFlag()
{
    mBitsRead += 1;
    return mBitstream.readBits(1) != 0;
}

uint32_t VvcRbspReader::readUE()
{
    uint64_t posBefore = mBitstream.getPos();
    uint32_t val = mBitstream.readExpGolombCode();
    mBitsRead += (mBitstream.getPos() - posBefore);
    return val;
}

int32_t VvcRbspReader::readSE()
{
    uint64_t posBefore = mBitstream.getPos();
    int32_t val = mBitstream.readSignedExpGolombCode();
    mBitsRead += (mBitstream.getPos() - posBefore);
    return val;
}

uint64_t VvcRbspReader::getBitsRead() const
{
    return mBitsRead;
}

void VvcRbspReader::byteAlign()
{
    while (!mBitstream.isByteAligned())
    {
        mBitstream.readBits(1);
        mBitsRead += 1;
    }
}

bool VvcRbspReader::hasEmulationBytesBefore(uint64_t bitPos) const
{
    // Convert bit position to byte position in the original (pre-RBSP) stream
    uint32_t bytePos = static_cast<uint32_t>(bitPos / 8);
    for (const auto& loc : mEmulationByteLocations)
    {
        if (loc <= bytePos)
        {
            return true;
        }
    }
    return false;
}
