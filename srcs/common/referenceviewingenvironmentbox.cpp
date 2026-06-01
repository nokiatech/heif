#include "referenceviewingenvironmentbox.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

ReferenceViewingEnvironmentBox::ReferenceViewingEnvironmentBox()
    : Box("reve")
{
}

void ReferenceViewingEnvironmentBox::setSurroundLuminance(uint32_t value){
    mSurroundLuminance = value;
}

uint32_t ReferenceViewingEnvironmentBox::getSurroundLuminance() const{
    return mSurroundLuminance;
}

void ReferenceViewingEnvironmentBox::setSurroundLightX(uint16_t value){
    mSurroundLightX = value;
}

uint16_t ReferenceViewingEnvironmentBox::getSurroundLightX() const{
    return mSurroundLightX;
}

void ReferenceViewingEnvironmentBox::setSurroundLightY(uint16_t value){
    mSurroundLightY = value;
}

uint16_t ReferenceViewingEnvironmentBox::getSurroundLightY() const{
    return mSurroundLightY;
}


void ReferenceViewingEnvironmentBox::setPeripheryLuminance(uint32_t value){
    mPeripheryLuminance = value;
}

uint32_t ReferenceViewingEnvironmentBox::getPeripheryLuminance() const{
    return mPeripheryLuminance;
}

void ReferenceViewingEnvironmentBox::setPeripheryLightX(uint16_t value){
    mPeripheryLightX = value;
}

uint16_t ReferenceViewingEnvironmentBox::getPeripheryLightX() const{
    return mPeripheryLightX;
}

void ReferenceViewingEnvironmentBox::setPeripheryLightY(uint16_t value){
    mPeripheryLightY = value;
}

uint16_t ReferenceViewingEnvironmentBox::getPeripheryLightY() const{
    return mPeripheryLightY;
}

void ReferenceViewingEnvironmentBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    bitstr.write32Bits(mSurroundLuminance);
    bitstr.write16Bits(mSurroundLightX);
    bitstr.write16Bits(mSurroundLightY);
    bitstr.write32Bits(mPeripheryLuminance);
    bitstr.write16Bits(mPeripheryLightX);
    bitstr.write16Bits(mPeripheryLightY);
    updateSize(bitstr);
}

void ReferenceViewingEnvironmentBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    mSurroundLuminance = bitstr.read32Bits();
    mSurroundLightX = bitstr.read16Bits();
    mSurroundLightY = bitstr.read16Bits();
    mPeripheryLuminance = bitstr.read32Bits();
    mPeripheryLightX = bitstr.read16Bits();
    mPeripheryLightY = bitstr.read16Bits();
}
