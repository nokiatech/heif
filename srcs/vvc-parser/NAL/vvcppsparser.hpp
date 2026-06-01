#ifndef VVCPPSPARSER_HPP
#define VVCPPSPARSER_HPP

#include "../vvcparser.hpp"
#include "../../common/customallocator.hpp"
#include <stdint.h>
#include "vvcrbspreader.hpp"

class VvcPpsParser {
public:
    static VvcParser::PpsInfo parse(const Vector<uint8_t>& ppsNalu);
};


#endif /*VVCPPSPARSER_HPP */
