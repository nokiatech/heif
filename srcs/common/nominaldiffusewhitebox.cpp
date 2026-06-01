#include "nominaldiffusewhitebox.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

NominalDiffuseWhiteBox::NominalDiffuseWhiteBox()
    :Box("ndwt")
{
}

void NominalDiffuseWhiteBox::setDiffuseWhiteLuminance(uint32_t value){
    mDiffuseWhiteLuminance = value;
}

uint32_t NominalDiffuseWhiteBox::getDiffuseWhiteLuminance() const{
    return mDiffuseWhiteLuminance;
}

void NominalDiffuseWhiteBox::writeBox(ISOBMFF::BitStream &bitstr) const
{
    writeBoxHeader(bitstr);
    bitstr.write32Bits(mDiffuseWhiteLuminance);
    updateSize(bitstr);
}

void NominalDiffuseWhiteBox::parseBox(ISOBMFF::BitStream &bitstr)
{
    parseBoxHeader(bitstr);
    mDiffuseWhiteLuminance = bitstr.read32Bits();
}
