/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
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
#include <signal.h>

#include <qcc/platform.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/BusObject.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/DBusStd.h>
#include <alljoyn/AllJoynStd.h>
#include <qcc/Util.h>
#include <qcc/Log.h>
#include <qcc/String.h>
#include <qcc/StringUtil.h>

#include <alljoyn/config/ConfigClient.h>
#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <alljoyn/notification/NotificationReceiver.h>
#include <LSFTypes.h>
#include <LSFKeyListener.h>

using namespace ajn;
using namespace std;
using namespace qcc;

#define QCC_MODULE "LAMP_SERVICE_TEST"

char* get_line(char* str, size_t num, FILE* fp)
{
    char* p = fgets(str, num, fp);

    // fgets will capture the '\n' character if the string entered is shorter than
    // num. Remove the '\n' from the end of the line and replace it with nul '\0'.
    if (p != NULL) {
        size_t last = strlen(str) - 1;
        if (str[last] == '\n') {
            str[last] = '\0';
        }
    }
    return p;
}

static qcc::String NextTok(qcc::String& inStr)
{
    qcc::String ret;
    size_t off = inStr.find_first_of(' ');
    if (off == qcc::String::npos) {
        ret = inStr;
        inStr.clear();
    } else {
        ret = inStr.substr(0, off);
        inStr = qcc::Trim(inStr.substr(off));
    }
    return qcc::Trim(ret);
}

static const char* interfaces[] =
{
    LampServiceInterfaceName,
    LampServiceStateInterfaceName,
    LampServiceParametersInterfaceName,
    LampServiceDetailsInterfaceName,
    ConfigServiceInterfaceName
};

class MyTestHandler : public services::AnnounceHandler, public SessionListener, services::NotificationReceiver {
  public:

    MyTestHandler(BusAttachment& bus) : services::AnnounceHandler(), bus(bus), _object(NULL)
    {
        QCC_DbgTrace(("%s", __func__));
        ajn::services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *this, sizeof(interfaces) / sizeof(interfaces[0]));

        // to receive notifications:
        services::NotificationService::getInstance()->initReceive(&bus, this);
    }

    virtual void SessionLost(SessionId sessionId, SessionLostReason reason)
    {
        QCC_DbgTrace(("%s", __func__));
        delete _object;
        _object = NULL;
        delete _config;
        _config = NULL;
    }

    virtual void Receive(const services::Notification& notification)
    {
        QCC_DbgPrintf(("New notification from %s\n", notification.getDeviceId()));
        QCC_DbgPrintf(("\tDeviceName: %s\n", notification.getDeviceName()));
        QCC_DbgPrintf(("\tAppId: %s\n", notification.getAppId()));
        QCC_DbgPrintf(("\tAppName: %s\n", notification.getAppName()));

        const std::vector<services::NotificationText>& texts = notification.getText();
        for (uint32_t i = 0; i < texts.size(); ++i) {
            QCC_DbgPrintf(("NOTIFICATION: [%s]\n", texts[i].getText().c_str()));
        }
    }

    virtual void Dismiss(const int32_t msgId, const qcc::String appId)
    {
        QCC_DbgPrintf(("Notification %d dismissed by %s\n", msgId, appId.c_str()));
    }

    void LampsStateChangedSignalHandler(const InterfaceDescription::Member* member, const char* sourcePath, Message& msg)
    {
        QCC_DbgPrintf(("SIGNAL: org.allseen.LSF.LampService.LampsStateChanged from /org/allseen/LSF/Lamp\n"));
    }

    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);
    QCC_DbgTrace(("%s", __func__));
    BusAttachment& bus;
    qcc::String _lampID;
    ProxyBusObject* _object;
    ProxyBusObject* _config;
};

void MyTestHandler::Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();
    bool is_lamp = false;
    QCC_DbgPrintf(("Got announcement for %s on port %u\n", busName, port));

    QCC_DbgPrintf(("ObjectDescriptions:\n"));
    for (ObjectDescriptions::const_iterator it = objectDescs.begin(); it != objectDescs.end(); ++it) {
        QCC_DbgPrintf(("\t%s:\n", it->first.c_str()));
        const vector<String>& vec = it->second;
        for (vector<String>::const_iterator vit = vec.begin(); vit != vec.end(); ++vit) {
            QCC_DbgPrintf(("\t\t[%s]\n", vit->c_str()));
        }
    }

    QCC_DbgPrintf(("AboutData:\n"));
    for (AboutData::const_iterator it = aboutData.begin(); it != aboutData.end(); ++it) {
        QCC_DbgPrintf(("\t%s: [%s]\n", it->first.c_str(), it->second.ToString().c_str()));
    }

    ObjectDescriptions::const_iterator oit = objectDescs.find(LampServiceObjectPath);
    if (oit != objectDescs.end()) {
        QCC_DbgPrintf(("%s found\n", LampServiceObjectPath));
        if (std::find(oit->second.begin(), oit->second.end(), LampServiceInterfaceName) != oit->second.end()) {
            QCC_DbgPrintf(("%s found\n", LampServiceInterfaceName));
            is_lamp = true;
        }
    }

    AboutData::const_iterator ait = aboutData.find("DeviceId");
    if (ait != aboutData.end()) {
        QCC_DbgPrintf(("DeviceId found\n"));
        const char* uniqueId;
        ait->second.Get("s", &uniqueId);
        _lampID = uniqueId;
    } else {
        is_lamp = false;
    }

    if (is_lamp && _object && _object->GetServiceName() == busName) {
        ait = aboutData.find("DeviceName");
        const char* name;
        ait->second.Get("s", &name);
        QCC_DbgPrintf(("LampsNameChanged: [%s]\n", name));
    }

    if (is_lamp && _object == NULL) {
        QCC_DbgPrintf(("Joining session %s\n", busName));
        SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
        SessionId sessionID;
        QStatus status = bus.JoinSession(busName, port, this, sessionID, opts);
        QCC_DbgPrintf(("BusAttachment::JoinSession(%u): %s\n", sessionID, QCC_StatusText(status)));
        _object = new ProxyBusObject(bus, busName, LampServiceObjectPath, sessionID);
        _object->IntrospectRemoteObject();

        _config = new ProxyBusObject(bus, busName, "/Config", sessionID);
        status = _config->IntrospectRemoteObject();
        printf("Config::IntrospectRemoteObject(): %s\n", QCC_StatusText(status));

        const InterfaceDescription* uniqueId = bus.GetInterface(LampServiceInterfaceName);
        const InterfaceDescription::Member* sig = uniqueId->GetMember("LampStateChanged");
        bus.RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&MyTestHandler::LampsStateChangedSignalHandler), sig, "/org/allseen/LSF/Lamp");
    }
}


#define INITIAL_PASSCODE "000000"

int main()
{
    QCC_DbgTrace(("%s", __func__));
    BusAttachment bus("lsftest", true);
    QStatus st = bus.Start();
    QCC_DbgPrintf(("BusAttachment::Start(): %s\n", QCC_StatusText(st)));

    st = bus.Connect();
    QCC_DbgPrintf(("BusAttachment::Connect(): %s\n", QCC_StatusText(st)));
    MyTestHandler handler(bus);

    lsf::LSFKeyListener keyListener;
    keyListener.SetPassCode(INITIAL_PASSCODE);

    st = bus.EnablePeerSecurity("ALLJOYN_PIN_KEYX", &keyListener);
    QCC_DbgPrintf(("EnablePeerSecurity(): %s\n", QCC_StatusText(st)));

    sleep(5);

    QStatus status = ER_OK;
    const int bufSize = 1024;
    char buf[bufSize];
    while ((ER_OK == status) && (get_line(buf, bufSize, stdin))) {
        String line(buf);
        String cmd = NextTok(line);

        if (cmd == "GetLampName") {
            MsgArg arg("s", "en");
            Message reply(bus);
            status = handler._config->MethodCall(
                "org.alljoyn.Config",
                "GetConfigurations",
                &arg,
                1,
                reply);

            if (status == ER_OK) {
                const ajn::MsgArg* args;
                size_t numArgs;
                reply->GetArgs(numArgs, args);

                size_t numEntries;
                MsgArg* entries;

                args[0].Get("a{sv}", &numEntries, &entries);
                for (size_t i = 0; i < numEntries; i++) {
                    char* key;
                    MsgArg* value;
                    entries[i].Get("{sv}", &key, &value);

                    if (0 == strcmp(key, "DeviceName")) {
                        char* name;
                        value->Get("s", &name);
                        printf("LampName = '%s'\n", name);
                    }
                }
            }
        } else if (cmd == "SetLampName") {
            String name = NextTok(line);

            MsgArg args[2];
            args[0].Set("s", "en");
            MsgArg name_arg("s", name.c_str());

            MsgArg arg("{sv}", "DeviceName", &name_arg);
            args[1].Set("a{sv}", 1, &arg);

            status = handler._config->MethodCall(
                "org.alljoyn.Config",
                "UpdateConfigurations",
                args,
                2);
            printf("UpdateConfigurations: %s\n", QCC_StatusText(status));
        } else if (cmd == "GetLampState") {
            MsgArg stateValues;
            QStatus status = handler._object->GetAllProperties(LampServiceStateInterfaceName, stateValues);
            printf("ProxyBusObject::GetAllProperties(org.allseen.LSF.LampState): %s\n", QCC_StatusText(status));
            if (status == ER_OK) {
                size_t numStateEntries;
                MsgArg* stateEntries;
                stateValues.Get("a{sv}", &numStateEntries, &stateEntries);

                for (size_t i = 0; i < numStateEntries; ++i) {
                    char* field;
                    MsgArg* value;
                    stateEntries[i].Get("{sv}", &field, &value);
                    printf("State[%s] = '%s'\n", field, value->ToString().c_str());
                }
            }
        } else if (cmd == "GetLampStateField") {
            String field = NextTok(line);
            MsgArg value;

            QStatus status = handler._object->GetProperty(LampServiceStateInterfaceName, field.c_str(), value);
            printf("ProxyBusObject::GetProperty(org.allseen.LSF.LampState, %s): %s\n", field.c_str(), QCC_StatusText(status));

            if (status == ER_OK) {
                printf("Value = '%s'\n", value.ToString().c_str());
            }
        } else if (cmd == "GetLampDetails") {
            MsgArg stateValues;
            QStatus status = handler._object->GetAllProperties(LampServiceDetailsInterfaceName, stateValues);
            printf("ProxyBusObject::GetAllProperties(org.allseen.LSF.LampDetails): %s\n", QCC_StatusText(status));
            if (status == ER_OK) {
                size_t numStateEntries;
                MsgArg* stateEntries;
                stateValues.Get("a{sv}", &numStateEntries, &stateEntries);

                for (size_t i = 0; i < numStateEntries; ++i) {
                    char* field;
                    MsgArg* value;
                    stateEntries[i].Get("{sv}", &field, &value);
                    printf("Details[%s] = '%s'\n", field, value->ToString().c_str());
                }
            }
        } else if (cmd == "GetLampParameters") {
            MsgArg stateValues;
            QStatus status = handler._object->GetAllProperties(LampServiceParametersInterfaceName, stateValues);
            printf("ProxyBusObject::GetAllProperties(org.allseen.LSF.LampParameters): %s\n", QCC_StatusText(status));
            if (status == ER_OK) {
                size_t numStateEntries;
                MsgArg* stateEntries;
                stateValues.Get("a{sv}", &numStateEntries, &stateEntries);

                for (size_t i = 0; i < numStateEntries; ++i) {
                    char* field;
                    MsgArg* value;
                    stateEntries[i].Get("{sv}", &field, &value);
                    printf("Details[%s] = '%s'\n", field, value->ToString().c_str());
                }
            }
        } else if (cmd == "GetLampParameterField") {
            String field = NextTok(line);
            MsgArg value;

            QStatus status = handler._object->GetProperty(LampServiceParametersInterfaceName, field.c_str(), value);
            printf("ProxyBusObject::GetProperty(org.allseen.LSF.LampParameters, %s): %s\n", field.c_str(), QCC_StatusText(status));

            if (status == ER_OK) {
                printf("LampParameters Field Value = '%s'\n", value.ToString().c_str());
            }
        } else if (cmd == "TransitionLampState") {
            Message replyMsg(bus);

            MsgArg methodArgs[2];
            methodArgs[0].Set("t", (uint64_t) 0);

            lsf::LampState state;
            state.SetField("OnOff", true); // OnOff
            state.SetField("Hue", (uint32_t) 1);
            state.SetField("Saturation", (uint32_t) 2);
            state.SetField("Brightness", (uint32_t) 3);
            state.SetField("ColorTemp", (uint32_t) 4);
            state.Get(methodArgs[1]);

            QStatus status = handler._object->MethodCall(LampServiceStateInterfaceName, "TransitionLampState", methodArgs, 2, replyMsg);
            printf("ProxyBusObject::MethodCall(): %s\n", QCC_StatusText(status));

            if (status == ER_OK) {
                const MsgArg* args;
                size_t numArgs;
                replyMsg->GetArgs(numArgs, args);

                const char* uniqueId;
                args[0].Get("s", &uniqueId);
                uint32_t responseCode;
                args[1].Get("u", &responseCode);
                QCC_DbgPrintf(("TransitionLampState (%s) returns responseCode %u\n", uniqueId, responseCode));
            }
        } else if (cmd == "TestState") {
            Message replyMsg(bus);

            const uint32_t HUE = 3;
            const uint32_t COLOR = 5;
            const uint32_t SATURATION = 6;
            const uint32_t BRIGHTNESS = 7;
            const bool ONOFF = true;

            MsgArg methodArgs[2];
            methodArgs[0].Set("t", (uint64_t) 0);

            lsf::LampState state;
            state.SetField("OnOff", ONOFF); // OnOff
            state.SetField("Hue", (uint32_t) HUE);
            state.SetField("Saturation", (uint32_t) SATURATION);
            state.SetField("Brightness", (uint32_t) BRIGHTNESS);
            state.SetField("ColorTemp", (uint32_t) COLOR);
            state.Get(methodArgs[1]);

            QStatus status = handler._object->MethodCall(LampServiceStateInterfaceName, "TransitionLampState", methodArgs, 2, replyMsg);
            printf("ProxyBusObject::MethodCall(): %s\n", QCC_StatusText(status));
            if (status == ER_OK) {
                const MsgArg* args;
                size_t numArgs;
                replyMsg->GetArgs(numArgs, args);

                uint32_t responseCode;
                args[1].Get("u", &responseCode);
                QCC_DbgPrintf(("TransitionLampState returns responseCode %u\n", responseCode));
            }

            MsgArg stateValues;
            status = handler._object->GetAllProperties(LampServiceStateInterfaceName, stateValues);
            printf("ProxyBusObject::GetAllProperties(org.allseen.LSF.LampState): %s\n", QCC_StatusText(status));
            if (status == ER_OK) {
                lsf::LampState newState(stateValues);

                bool OnOff;
                uint32_t hue, colorTemperature, saturation, brightness;
                newState.GetField("OnOff", OnOff);
                newState.GetField("Hue", hue);
                newState.GetField("ColorTemp", colorTemperature);
                newState.GetField("Saturation", saturation);
                newState.GetField("Brightness", brightness);

                printf("OnOff=%d, HSCB= %u,%u,%u,%u\n", OnOff, hue, saturation, colorTemperature, brightness);
            }
        } else if (cmd == "GetLampFaults") {
            String field = NextTok(line);
            MsgArg value;

            QStatus status = handler._object->GetProperty(LampServiceInterfaceName, "LampFaults", value);
            printf("ProxyBusObject::GetProperty(org.allseen.LSF.LampParameters, %s): %s\n", field.c_str(), QCC_StatusText(status));

            if (status == ER_OK) {
                size_t arraySize;
                MsgArg* array;
                value.Get("au", &arraySize, &array);

                for (size_t i = 0; i < arraySize; ++i) {
                    uint32_t code;
                    array[i].Get("u", &code);
                    printf("FaultCode[%lu] = %u\n", i, code);
                }
            }
        } else {
            QCC_LogError(ER_BUS_BAD_VALUE_TYPE, ("Command '%s' not valid\n", cmd.c_str()));
        }
    }

    bus.Stop();
}


