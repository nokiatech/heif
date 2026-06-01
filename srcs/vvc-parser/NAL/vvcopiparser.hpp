#ifndef VVCOPIPARSER_HPP
#define VVCOPIPARSER_HPP

#include "../vvcparser.hpp"
#include "../../common/customallocator.hpp"
#include <stdint.h>
#include "vvcrbspreader.hpp"

class VvcOpiParser {
public:
    static VvcParser::OpiInfo parse(const Vector<uint8_t>& opiNalu);
};


#endif /*VVCOPIPARSER_HPP */
