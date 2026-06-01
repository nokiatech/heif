#ifndef REFERENCEVIEWINGENVIRONMENTBOX_HPP
#define REFERENCEVIEWINGENVIRONMENTBOX_HPP

#include "bbox.hpp"

#include <stdint.h>

namespace ISOBMFF
{
    class BitStream;
}

class ReferenceViewingEnvironmentBox : public Box
{
public:
    ReferenceViewingEnvironmentBox();
    ~ReferenceViewingEnvironmentBox() override = default;

    void setSurroundLuminance(uint32_t value);
    uint32_t getSurroundLuminance() const;

    void setSurroundLightX(uint16_t value);
    uint16_t getSurroundLightX() const;

    void setSurroundLightY(uint16_t value);
    uint16_t getSurroundLightY() const;


    void setPeripheryLuminance(uint32_t value);
    uint32_t getPeripheryLuminance() const;

    void setPeripheryLightX(uint16_t value);
    uint16_t getPeripheryLightX() const;

    void setPeripheryLightY(uint16_t value);
    uint16_t getPeripheryLightY() const;

    void writeBox(ISOBMFF::BitStream &bitstr) const override;
    void parseBox(ISOBMFF::BitStream &bitstr) override;

private:
    uint32_t mSurroundLuminance;
    uint16_t mSurroundLightX;
    uint16_t mSurroundLightY;
    uint32_t mPeripheryLuminance;
    uint16_t mPeripheryLightX;
    uint16_t mPeripheryLightY;

};

#endif // REFERENCEVIEWINGENVIRONMENTBOX_HPP
