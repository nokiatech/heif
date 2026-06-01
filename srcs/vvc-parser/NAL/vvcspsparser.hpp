#ifndef VVCSPSPARSER_HPP
#define VVCSPSPARSER_HPP

#include "../vvcparser.hpp"
#include "../../common/customallocator.hpp"
#include <stdint.h>
#include "vvcrbspreader.hpp"

class VvcSpsParser {
public:
    static VvcParser::SpsInfo parse(const Vector<uint8_t>& spsNalu);
private:
    static void parseSubpicInfo(VvcRbspReader& reader, VvcParser::SpsInfo& info,
                                uint32_t maxWidth, uint32_t maxHeight, uint32_t ctuSize);
};


#endif /*VVCSPSPARSER_HPP */
