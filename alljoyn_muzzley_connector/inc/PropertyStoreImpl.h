/******************************************************************************
 * Copyright (c) 2013-2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#ifndef PROPERTYSTOREIMPL_H_
#define PROPERTYSTOREIMPL_H_

#include <stdio.h>
#include <iostream>

#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AboutPropertyStoreImpl.h>

/**
 * class PropertyStoreImpl
 * Property store implementation
 */
class PropertyStoreImpl : public ajn::services::AboutPropertyStoreImpl {

  public:
    /**
     * PropertyStoreImpl - constructor
     * @param factoryConfigFile
     * @param configFile
     */
    PropertyStoreImpl(const char* factoryConfigFile, const char* configFile);

    /**
     * FactoryReset
     */
    void FactoryReset();

    /**
     * GetConfigFileName
     * @return qcc::String&
     */
    const qcc::String& GetConfigFileName();

    /**
     * virtual Destructor
     */
    virtual ~PropertyStoreImpl();

    /**
     * virtual method ReadAll
     * @param languageTag
     * @param filter
     * @param all
     * @return QStatus
     */
    virtual QStatus ReadAll(const char* languageTag, Filter filter, ajn::MsgArg& all);

    /**
     * virtual method Update
     * @param name
     * @param languageTag
     * @param value
     * @return QStatus
     */
    virtual QStatus Update(const char* name, const char* languageTag, const ajn::MsgArg* value);

    /**
     * virtual method Delete
     * @param name
     * @param languageTag
     * @return QStatus
     */
    virtual QStatus Delete(const char* name, const char* languageTag);

    /**
     * method Initialize
     */
    void Initialize();

  private:

    ajn::services::PropertyMap m_factoryProperties;

    bool m_IsInitialized;

    qcc::String m_configFileName;

    qcc::String m_factoryConfigFileName;

    bool persistUpdate(const char* key, const char* value, const char* languageTag = NULL);

    ajn::services::PropertyStoreKey getPropertyStoreKeyFromName(qcc::String const& propertyStoreName);


    bool UpdateFactorySettings();

    bool FillDeviceNames();

    void UpdateFactoryProperty(ajn::services::PropertyStoreKey propertyKey, const char* languageTag, const ajn::MsgArg& value);
};

#endif /* PROPERTYSTOREIMPL_H_ */
