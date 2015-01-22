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

#include <algorithm>
#include "CommonSampleUtil.h"
#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/AnnouncementRegistrar.h>

using namespace ajn;
using namespace services;

#define CHECK_RETURN(x) if ((status = x) != ER_OK) { return status; }

BusAttachment* CommonSampleUtil::prepareBusAttachment(ajn::AuthListener* authListener)
{
    BusAttachment* bus = new BusAttachment("CommonServiceApp", true);

    /* Start the BusAttachment */
    QStatus status = bus->Start();
    if (status != ER_OK) {
        delete bus;
        return NULL;
    }

    /* Connect to the daemon using address provided*/
    status = bus->Connect();
    if (status != ER_OK) {
        delete bus;
        return NULL;
    }

    if (authListener) {
        status = CommonSampleUtil::EnableSecurity(bus, authListener);
        if (status != ER_OK) {
            delete bus;
            return NULL;
        }
    }

    return bus;
}

QStatus CommonSampleUtil::fillPropertyStore(AboutPropertyStoreImpl* propertyStore, qcc::String const& appIdHex,
                                            qcc::String const& appName, qcc::String const& deviceId, DeviceNamesType const& deviceNames,
                                            qcc::String const& defaultLanguage)
{
    if (!propertyStore) {
        return ER_BAD_ARG_1;
    }

    QStatus status = ER_OK;

    CHECK_RETURN(propertyStore->setDeviceId(deviceId))
    CHECK_RETURN(propertyStore->setAppId(appIdHex))
    CHECK_RETURN(propertyStore->setAppName(appName))

    std::vector<qcc::String> languages(3);
    languages[0] = "en";
    languages[1] = "es";
    languages[2] = "fr";
    CHECK_RETURN(propertyStore->setSupportedLangs(languages))
    CHECK_RETURN(propertyStore->setDefaultLang(defaultLanguage))

    CHECK_RETURN(propertyStore->setModelNumber("Wxfy388i"))
    CHECK_RETURN(propertyStore->setDateOfManufacture("10/1/2199"))
    CHECK_RETURN(propertyStore->setSoftwareVersion("12.20.44 build 44454"))
    CHECK_RETURN(propertyStore->setAjSoftwareVersion(ajn::GetVersion()))
    CHECK_RETURN(propertyStore->setHardwareVersion("355.499. b"))

    DeviceNamesType::const_iterator iter = deviceNames.find(languages[0]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(propertyStore->setDeviceName(iter->second.c_str(), languages[0]));
    } else {
        CHECK_RETURN(propertyStore->setDeviceName("My device name", "en"));
    }

    iter = deviceNames.find(languages[1]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(propertyStore->setDeviceName(iter->second.c_str(), languages[1]));
    } else {
        CHECK_RETURN(propertyStore->setDeviceName("Mi nombre de dispositivo", "sp"));
    }

    iter = deviceNames.find(languages[2]);
    if (iter != deviceNames.end()) {
        CHECK_RETURN(propertyStore->setDeviceName(iter->second.c_str(), languages[2]));
    } else {
        CHECK_RETURN(propertyStore->setDeviceName("Mon nom de l'appareil", "fr"));
    }

    CHECK_RETURN(propertyStore->setDescription("This is an Alljoyn Application", "en"))
    CHECK_RETURN(propertyStore->setDescription("Esta es una Alljoyn aplicacion", "sp"))
    CHECK_RETURN(propertyStore->setDescription("C'est une Alljoyn application", "fr"))

    CHECK_RETURN(propertyStore->setManufacturer("Company", "en"))
    CHECK_RETURN(propertyStore->setManufacturer("Empresa", "sp"))
    CHECK_RETURN(propertyStore->setManufacturer("Entreprise", "fr"))

    CHECK_RETURN(propertyStore->setSupportUrl("http://www.alljoyn.org"))

    return status;
}

QStatus CommonSampleUtil::prepareAboutService(BusAttachment* bus, AboutPropertyStoreImpl* propertyStore,
                                              CommonBusListener* busListener, uint16_t port)
{
    if (!bus) {
        return ER_BAD_ARG_1;
    }

    if (!propertyStore) {
        return ER_BAD_ARG_2;
    }

    if (!busListener) {
        return ER_BAD_ARG_3;
    }

    AboutServiceApi::Init(*bus, *propertyStore);
    AboutServiceApi* aboutService = AboutServiceApi::getInstance();
    if (!aboutService) {
        return ER_BUS_NOT_ALLOWED;
    }

    busListener->setSessionPort(port);
    bus->RegisterBusListener(*busListener);

    TransportMask transportMask = TRANSPORT_ANY;
    SessionPort sp = port;
    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, transportMask);

    QStatus status = bus->BindSessionPort(sp, opts, *busListener);
    if (status != ER_OK) {
        return status;
    }

    status = aboutService->Register(port);
    if (status != ER_OK) {
        return status;
    }

    return (bus->RegisterBusObject(*aboutService));
}

QStatus CommonSampleUtil::aboutServiceAnnounce()
{
    AboutServiceApi* aboutService = AboutServiceApi::getInstance();
    if (!aboutService) {
        return ER_BUS_NOT_ALLOWED;
    }

    return (aboutService->Announce());
}

void CommonSampleUtil::aboutServiceDestroy(BusAttachment* bus,
                                           CommonBusListener* busListener)
{
    if (bus && busListener) {
        bus->UnregisterBusListener(*busListener);
        bus->UnbindSessionPort(busListener->getSessionPort());
    }

    AboutServiceApi::DestroyInstance();
    return;
}

QStatus CommonSampleUtil::EnableSecurity(BusAttachment* bus, AuthListener* authListener)
{
    QStatus status = bus->EnablePeerSecurity("ALLJOYN_PIN_KEYX ALLJOYN_SRP_KEYX ALLJOYN_ECDHE_PSK", authListener);
    return status;
}
