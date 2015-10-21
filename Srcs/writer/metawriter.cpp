/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "metawriter.hpp"
#include "datastore.hpp"
#include "metabox.hpp"
#include "services.hpp"

MetaWriter::MetaWriter() :
    mMasterStore(nullptr),
    mInternStore(nullptr),
    mHandlerType()
{
}

MetaWriter::MetaWriter(const std::string& handlerType, const std::uint32_t contextId) :
    mMasterStore(nullptr),
    mInternStore(new DataStore),
    mHandlerType(handlerType)
{
    DataServe::regStore(contextId, mInternStore);
}

uint32_t MetaWriter::getBaseOffset() const
{
    return mBaseOffset;
}

void MetaWriter::setBaseOffset(const uint32_t offset)
{
    mBaseOffset = offset;
}


void MetaWriter::initWrite()
{
    if (mInternStore)
    {
        mInternStore->clear();
    }
}

void MetaWriter::storeValue(const std::string& key, const std::string& value)
{
    mInternStore->setValue(key, value);
}

void MetaWriter::linkMasterDataStore(const ContextId masterContextId)
{
    mMasterStore = DataServe::getStore(masterContextId);
}

/**
 * @todo Check if this should refactored. At least there should be a check that different writers would not silently
 * overwrite a different handler type possibly already in the HandlerBox.
 */
void MetaWriter::hdlrWrite(MetaBox* metaBox)
{
    metaBox->setHandlerType(mHandlerType);
}

std::vector<std::string> MetaWriter::getMasterStoreValue(const std::string& key) const
{
    return mMasterStore->getValue(key);
}
