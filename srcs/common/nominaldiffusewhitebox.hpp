#ifndef NOMINALDIFFUSEWHITEBOX_HPP
#define NOMINALDIFFUSEWHITEBOX_HPP

#include "bbox.hpp"

#include <stdint.h>

namespace ISOBMFF
{
    class BitStream;
}

class NominalDiffuseWhiteBox : public Box
{
public:
    NominalDiffuseWhiteBox();
    ~NominalDiffuseWhiteBox() override = default;

    void setDiffuseWhiteLuminance(uint32_t value);
    uint32_t getDiffuseWhiteLuminance() const;

    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;

private:
    uint32_t mDiffuseWhiteLuminance;
};

#endif // NOMINALDIFFUSEWHITEBOX_HPP
