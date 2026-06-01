#include "vvcopiparser.hpp"
#include "heiferrorinfo.h"
#include "../vvcparser.hpp"
#include "heifcommondatatypes.h"
#include "vvcrbspreader.hpp"
#include <exception>

VvcParser::OpiInfo VvcOpiParser::parse(const Vector<uint8_t>& opiNalu) {
    VvcParser::OpiInfo info = {};
    
    VvcRbspReader reader;
    if (!reader.prepare(opiNalu)) {
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcOpiParser::parse: VvcRbspReader::prepare rejected OPI NAL (too short or NAL header invalid)");
        return info;  // Error case
    }

    try
    {

    // OPI RBSP syntax (after NAL header):
    info.mOlsinfopresentflag = reader.readFlag();   // opi_ols_info_present_flag
    info.mHtidinfopresentflag = reader.readFlag();  // opi_htid_info_present_flag
    
    if (info.mOlsinfopresentflag) {
        info.mOpiolsidx = reader.readUE();          // opi_ols_idx ue(v)
    }
    
    if (info.mHtidinfopresentflag) {
        info.mOpihtidplus1 = reader.readBits(3);    // opi_htid_plus1 u(3)
    }
    
    // opi_extension_flag, opi_extension_data_flag, rbsp_trailing_bits
    // (not needed for OpiInfo)

    return info;

    }  // try
    catch (const std::exception& e)
    {
        // Truncated or malformed OPI NALU -- return zeroed info rather than
        // propagating an exception through the Reader API.
        HEIF_RECORD_FROM(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ::HEIF::ErrorOrigin::Parser_VvcOpi,
                         "VvcOpiParser::parse: exception while reading OPI RBSP (truncated or malformed)",
                         e);
        return {};
    }
}
