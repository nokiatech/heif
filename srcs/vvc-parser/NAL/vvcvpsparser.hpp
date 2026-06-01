#ifndef VVCVPSPARSER_HPP
#define VVCVPSPARSER_HPP

#include "../vvcparser.hpp"
#include "../../common/customallocator.hpp"
#include <stdint.h>
#include "vvcrbspreader.hpp"

class VvcVpsParser {
public:
    static VvcParser::VpsInfo parse(const Vector<uint8_t>& vpsNalu);
};


#endif /*VVCVPSPARSER_HPP */
