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
#include "PropertyStoreImpl.h"
#include "IniParser.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#define CHECK(x) if ((status = x) != ER_OK) { break; }
#define CHECK_RETURN(x) if ((status = x) != ER_OK) { return status; }

using namespace ajn;
using namespace services;

PropertyStoreImpl::PropertyStoreImpl(const char* factoryConfigFile, const char* configFile) : m_IsInitialized(false)
{
    m_configFileName.assign(configFile);
    m_factoryConfigFileName.assign(factoryConfigFile);
}

void PropertyStoreImpl::Initialize()
{
    m_IsInitialized = true;
    m_factoryProperties.clear();
    m_factoryProperties.insert(m_Properties.begin(), m_Properties.end());

    //m_factoryProperties - overwrite the values that are found in FactoryConfigService file
    UpdateFactorySettings();
}

void PropertyStoreImpl::FactoryReset()
{
    std::ifstream factoryConfigFile(m_factoryConfigFileName.c_str(), std::ios::binary);
    std::ofstream configFile(m_configFileName.c_str(), std::ios::binary);

    if (factoryConfigFile && configFile) {
        configFile << factoryConfigFile.rdbuf();

        configFile.close();
        factoryConfigFile.close();
    } else {
        std::cout << "Factory reset failed" << std::endl;
    }

    m_Properties.clear();
    m_Properties.insert(m_factoryProperties.begin(), m_factoryProperties.end());
}

const qcc::String& PropertyStoreImpl::GetConfigFileName()
{
    return m_configFileName;
}

PropertyStoreImpl::~PropertyStoreImpl()
{
}

QStatus PropertyStoreImpl::ReadAll(const char* languageTag, Filter filter, ajn::MsgArg& all)
{
    if (!m_IsInitialized) {
        return ER_FAIL;
    }

    if (filter == ANNOUNCE || filter == READ) {
        return AboutPropertyStoreImpl::ReadAll(languageTag, filter, all);
    }

    if (filter != WRITE) {
        return ER_FAIL;
    }

    QStatus status = ER_OK;
    if (languageTag != NULL && languageTag[0] != 0) { // check that the language is in the supported languages;
        CHECK_RETURN(isLanguageSupported(languageTag))
    } else {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }
        CHECK_RETURN(it->second.getPropertyValue().Get("s", &languageTag))
    }

    MsgArg* argsWriteData = new MsgArg[m_Properties.size()];
    uint32_t writeArgCount = 0;
    do {
        for (PropertyMap::const_iterator it = m_Properties.begin(); it != m_Properties.end(); ++it) {
            const PropertyStoreProperty& property = it->second;

            if (!property.getIsWritable()) {
                continue;
            }

            // check that it is from the defaultLanguage or empty.
            if (!(property.getLanguage().empty() || property.getLanguage().compare(languageTag) == 0)) {
                continue;
            }

            CHECK(argsWriteData[writeArgCount].Set("{sv}", property.getPropertyName().c_str(),
                                                   new MsgArg(property.getPropertyValue())))
            argsWriteData[writeArgCount].SetOwnershipFlags(MsgArg::OwnsArgs, true);
            writeArgCount++;
        }
        CHECK(all.Set("a{sv}", writeArgCount, argsWriteData))
        all.SetOwnershipFlags(MsgArg::OwnsArgs, true);
    } while (0);

    if (status != ER_OK) {
        delete[] argsWriteData;
    }

    return status;
}

QStatus PropertyStoreImpl::Update(const char* name, const char* languageTag, const ajn::MsgArg* value)
{
    if (!m_IsInitialized) {
        return ER_FAIL;
    }

    PropertyStoreKey propertyKey = getPropertyStoreKeyFromName(name);
    if (propertyKey >= NUMBER_OF_KEYS) {
        return ER_FEATURE_NOT_AVAILABLE;
    }

    // check the languageTag
    // case languageTag == NULL: is not a valid value for the languageTag
    // case languageTag == "": use the default language
    // case languageTag == string: check value, must be one of the supported languages
    QStatus status = ER_OK;
    if (languageTag == NULL) {
        return ER_INVALID_VALUE;
    } else if (languageTag[0] == 0) {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }
        status = it->second.getPropertyValue().Get("s", &languageTag);
    } else {
        status = isLanguageSupported(languageTag);
        if  (status != ER_OK) {
            return status;
        }
    }

    // Special case DEFAULT_LANG is not associated with a language in the PropertyMap and
    // its only valid languageTag = NULL
    // By setting it here, we to let the user follow the same language rules as any other property
    if (propertyKey == DEFAULT_LANG) {
        languageTag = NULL;
    }

    //validate that the value is acceptable
    qcc::String languageString = languageTag ? languageTag : "";
    status = validateValue(propertyKey, *value, languageString);
    if (status != ER_OK) {
        std::cout << "New Value failed validation. Will not update" << std::endl;
        return status;
    }

    PropertyStoreProperty* temp = NULL;
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = m_Properties.equal_range(propertyKey);

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; it++) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) ||
                (languageTag != NULL && property.getLanguage().compare(languageTag) == 0)) {
                temp = new PropertyStoreProperty(property.getPropertyName(), *value, property.getIsPublic(),
                                                 property.getIsWritable(), property.getIsAnnouncable());
                if (languageTag) {
                    temp->setLanguage(languageTag);
                }
                m_Properties.erase(it);
                break;
            }
        }
    }

    if (temp == NULL) {
        return ER_INVALID_VALUE;
    }

    m_Properties.insert(PropertyPair(propertyKey, *temp));

    if (persistUpdate(temp->getPropertyName().c_str(), value->v_string.str, languageTag)) {
        AboutService* aboutService = AboutServiceApi::getInstance();
        if (aboutService) {
            aboutService->Announce();
            std::cout << "Calling Announce after UpdateConfiguration" << std::endl;
        }
        delete temp;
        return ER_OK;
    } else {
        delete temp;
        return ER_INVALID_VALUE;
    }
}

QStatus PropertyStoreImpl::Delete(const char* name, const char* languageTag)
{
    if (!m_IsInitialized) {
        return ER_FAIL;
    }

    PropertyStoreKey propertyKey = getPropertyStoreKeyFromName(name);
    if (propertyKey >= NUMBER_OF_KEYS) {
        return ER_FEATURE_NOT_AVAILABLE;
    }

    QStatus status = ER_OK;
    if (languageTag == NULL) {
        return ER_INVALID_VALUE;
    } else if (languageTag[0] == 0) {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }
        status = it->second.getPropertyValue().Get("s", &languageTag);
    } else {
        status = isLanguageSupported(languageTag);
        if  (status != ER_OK) {
            return status;
        }
    }

    if (propertyKey == DEFAULT_LANG) {
        languageTag = NULL;
    }

    bool deleted = false;
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = m_Properties.equal_range(propertyKey);

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; it++) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) ||
                (languageTag != NULL && property.getLanguage().compare(languageTag) == 0)) {
                m_Properties.erase(it);
                // insert from backup.
                deleted = true;
                break;
            }
        }
    }

    if (!deleted) {
        if (languageTag != NULL) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        } else {
            return ER_INVALID_VALUE;
        }
    }

    propertiesIter = m_factoryProperties.equal_range(propertyKey);

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; it++) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) ||
                (languageTag != NULL && property.getLanguage().compare(languageTag) == 0)) {

                m_Properties.insert(PropertyPair(it->first, it->second));
                char* value;
                it->second.getPropertyValue().Get("s", &value);
                if (persistUpdate(it->second.getPropertyName().c_str(), value, languageTag)) {
                    AboutService* aboutService = AboutServiceApi::getInstance();
                    if (aboutService) {
                        aboutService->Announce();
                        std::cout << "Calling Announce after ResetConfiguration" << std::endl;
                    }
                    return ER_OK;
                }
            }
        }
    }
    return ER_INVALID_VALUE;
}

bool PropertyStoreImpl::persistUpdate(const char* key, const char* value, const char* languageTag)
{
    std::map<std::string, std::string> data;
    std::string skey(key);
    if (languageTag && languageTag[0]) {
        skey.append(".");
        skey.append(languageTag);
    }

    data[skey] = value;
    return IniParser::UpdateFile(m_configFileName.c_str(), data);
}

PropertyStoreKey PropertyStoreImpl::getPropertyStoreKeyFromName(qcc::String const& propertyStoreName)
{
    for (int indx = 0; indx < NUMBER_OF_KEYS; indx++) {
        if (PropertyStoreName[indx].compare(propertyStoreName) == 0) {
            return (PropertyStoreKey)indx;
        }
    }
    return NUMBER_OF_KEYS;
}

bool PropertyStoreImpl::FillDeviceNames()
{
    std::map<std::string, std::string> data;

    if (!IniParser::ParseFile(m_factoryConfigFileName.c_str(), data)) {
        std::cerr << "Could not parse configFile" << std::endl;
        return false;
    }

    typedef std::map<std::string, std::string>::iterator it_data;
    for (it_data iterator = data.begin(); iterator != data.end(); iterator++) {

        if (iterator->first.find(AboutPropertyStoreImpl::getPropertyStoreName(DEVICE_NAME).c_str()) == 0) {
            size_t lastDotLocation = iterator->first.find(".");
            if ((lastDotLocation ==  std::string::npos) || (lastDotLocation + 1 >= iterator->first.length())) {
                continue;
            }
            std::string language = iterator->first.substr(lastDotLocation + 1);
            std::string value = iterator->second;

            UpdateFactoryProperty(DEVICE_NAME, language.c_str(), MsgArg("s", value.c_str()));
        }
    }

    return true;
}

bool PropertyStoreImpl::UpdateFactorySettings()
{
    std::map<std::string, std::string> data;
    if (!IniParser::ParseFile(m_factoryConfigFileName.c_str(), data)) {
        std::cerr << "Could not parse configFile" << std::endl;
        return false;
    }

    std::map<std::string, std::string>::iterator iter;

    iter = data.find(AboutPropertyStoreImpl::getPropertyStoreName(DEVICE_ID).c_str());
    if (iter != data.end()) {
        qcc::String deviceId = iter->second.c_str();
        UpdateFactoryProperty(DEVICE_ID, NULL, MsgArg("s", deviceId.c_str()));
    }

    if (!FillDeviceNames()) {
        return false;
    }

    iter = data.find(AboutPropertyStoreImpl::getPropertyStoreName(APP_ID).c_str());
    if (iter != data.end()) {
        qcc::String appGUID = iter->second.c_str();
        UpdateFactoryProperty(APP_ID, NULL, MsgArg("s", appGUID.c_str()));
    }

    iter = data.find(AboutPropertyStoreImpl::getPropertyStoreName(APP_NAME).c_str());
    if (iter != data.end()) {
        qcc::String appName = iter->second.c_str();
        UpdateFactoryProperty(APP_NAME, NULL, MsgArg("s", appName.c_str()));
    }

    iter = data.find(AboutPropertyStoreImpl::getPropertyStoreName(DEFAULT_LANG).c_str());
    if (iter != data.end()) {
        qcc::String defaultLanguage = iter->second.c_str();
        UpdateFactoryProperty(DEFAULT_LANG, NULL, MsgArg("s", defaultLanguage.c_str()));
    }

    return true;
}


void PropertyStoreImpl::UpdateFactoryProperty(PropertyStoreKey propertyKey, const char* languageTag, const ajn::MsgArg& value)
{
    PropertyStoreProperty* temp = NULL;
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = m_factoryProperties.equal_range(propertyKey);

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; it++) {
        const PropertyStoreProperty& property = it->second;

        if ((languageTag == NULL && property.getLanguage().empty()) ||
            (languageTag != NULL && property.getLanguage().compare(languageTag) == 0)) {
            temp = new PropertyStoreProperty(property.getPropertyName(), value, property.getIsPublic(),
                                             property.getIsWritable(), property.getIsAnnouncable());
            if (languageTag) {
                temp->setLanguage(languageTag);
            }
            m_factoryProperties.erase(it);
            break;
        }
    }



    if (temp == NULL) {
        return;
    }

    m_factoryProperties.insert(PropertyPair(propertyKey, *temp));
    delete temp;
}
