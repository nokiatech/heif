#ifndef VVC_CODED_IMAGE_ITEM_H
#define VVC_CODED_IMAGE_ITEM_H

#pragma once

#include <CodedImageItem.h>
#include "heifcommondatatypes.h"
#include <cstdint>

namespace HEIFPP
{
    class VVCCodedImageItem : public HEIFPP::CodedImageItem
    {
        // Construction is private; use Heif::create<T>() factory.
        friend class HEIFPP::Heif;

    public:
        ~VVCCodedImageItem() override = default;
        // serialization
    protected:
        explicit VVCCodedImageItem(Heif* aHeif);
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) override;

    public:
        VVCCodedImageItem& operator=(const VVCCodedImageItem&) = delete;
        VVCCodedImageItem& operator=(VVCCodedImageItem&&) = delete;
        VVCCodedImageItem(const VVCCodedImageItem&)       = delete;
        VVCCodedImageItem(VVCCodedImageItem&&)            = delete;
        VVCCodedImageItem()                                = delete;
    };
}  // namespace HEIFPP


#endif /* VVC_CODED_IMAGE_ITEM_H */
