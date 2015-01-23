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

#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <alljoyn/Status.h>
#include <qcc/Debug.h>

#include <ControllerClient.h>
#include <AllJoynStd.h>

using namespace qcc;
using namespace ajn;

#define QCC_MODULE "CONTROLLER_CLIENT"

namespace lsf {

/**
 * Controller Service Object Path
 */

/**
 * Handler class for some standard AllJoyn signals and callbacks
 */
class ControllerClient::ControllerClientBusHandler :
    public SessionListener, public BusAttachment::JoinSessionAsyncCB,
    public services::AnnounceHandler, public BusListener {

  public:
    /**
     * Constructor
     */
    ControllerClientBusHandler(ControllerClient& client) : controllerClient(client) { }

    /**
     * Destructor
     */
    virtual ~ControllerClientBusHandler() { }

    /**
     * Session Lost signal handler
     */
    virtual void SessionLost(SessionId sessionId, SessionLostReason reason);

    /**
     * JoinSession callback handler
     */
    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context);

    /**
     * Announce signal handler
     */
    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

    virtual void SessionMemberRemoved(SessionId sessionId, const char* uniqueName);

    virtual void BusDisconnected() {
        QCC_DbgPrintf(("BusDisconnected!"));
        ErrorCodeList errors;
        errors.push_back(ERROR_DISCONNECTED_FROM_BUS);
        QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB\n", __func__));
        controllerClient.callback.ControllerClientErrorCB(errors);
    }

  private:

    /**
     * Reference to the Controller Client instance
     */
    ControllerClient& controllerClient;
};

void ControllerClient::DoLeaveSessionAsync(ajn::SessionId sessionId)
{
    QCC_DbgPrintf(("%s: sessionId(%d)", __func__, sessionId));
    MsgArg arg("u", sessionId);

    bus.GetAllJoynProxyObj().MethodCallAsync(
        org::alljoyn::Bus::InterfaceName,
        "LeaveSession",
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::LeaveSessionAsyncReplyHandler),
        &arg,
        1);
}

void ControllerClient::LeaveSessionAsyncReplyHandler(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply for LeaveSessionAsync:%s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
        uint32_t disposition;
        QStatus status = message->GetArgs("u", &disposition);
        if (status == ER_OK) {
            switch (disposition) {
            case ALLJOYN_LEAVESESSION_REPLY_SUCCESS:
                QCC_DbgPrintf(("%s: Leave Session successful", __func__));
                break;

            case ALLJOYN_LEAVESESSION_REPLY_NO_SESSION:
                QCC_DbgPrintf(("%s: No Session", __func__));
                break;

            case ALLJOYN_LEAVESESSION_REPLY_FAILED:
                QCC_DbgPrintf(("%s: Leave Session reply failed", __func__));
                break;

            default:
                QCC_DbgPrintf(("%s: Leave Session unexpected disposition", __func__));
                break;
            }
        }
    }
}


void ControllerClient::ControllerClientBusHandler::JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("%s", __func__));
    controllerClient.OnSessionJoined(status, sessionId, context);
}

void ControllerClient::ControllerClientBusHandler::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("SessionLost(%u)", sessionId));
    controllerClient.OnSessionLost(sessionId);
}

void ControllerClient::ControllerClientBusHandler::SessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("SessionMemberRemoved(%u,%s)", sessionId, uniqueName));
    controllerClient.OnSessionMemberRemoved(sessionId, uniqueName);
}

void ControllerClient::ControllerClientBusHandler::Announce(
    uint16_t version,
    SessionPort port,
    const char* busName,
    const ObjectDescriptions& objectDescs,
    const AboutData& aboutData)
{
    QCC_DbgPrintf(("%s", __func__));

    if (controllerClient.stopped) {
        QCC_DbgPrintf(("%s: Controller Client is stopped. Returning without processing announcement.", __func__));
        return;
    }

    AboutData::const_iterator ait;
    const char* deviceID = NULL;
    const char* deviceName = NULL;

    controllerClient.bus.EnableConcurrentCallbacks();

    ObjectDescriptions::const_iterator oit = objectDescs.find(ControllerServiceObjectPath);
    if (oit != objectDescs.end()) {
        QCC_DbgPrintf(("%s: About Data Dump", __func__));
        for (ait = aboutData.begin(); ait != aboutData.end(); ait++) {
            QCC_DbgPrintf(("%s: %s", ait->first.c_str(), ait->second.ToString().c_str()));
        }

        ait = aboutData.find("DeviceId");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: DeviceId missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("s", &deviceID);

        ait = aboutData.find("DeviceName");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: DeviceName missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("s", &deviceName);

        uint64_t higherBits = 0, lowerBits = 0;
        ait = aboutData.find("RankHigherBits");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: RankHigherBits missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &higherBits);

        ait = aboutData.find("RankLowerBits");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: RankLowerBits missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &lowerBits);


        Rank rank(higherBits, lowerBits);

        bool isLeader;
        ait = aboutData.find("IsLeader");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: IsLeader missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("b", &isLeader);

        QCC_DbgPrintf(("%s: Received Announce: busName=%s port=%u deviceID=%s deviceName=%s rank=%s isLeader=%d", __func__,
                       busName, port, deviceID, deviceName, rank.c_str(), isLeader));
        if (isLeader) {
            controllerClient.OnAnnounced(port, busName, deviceID, deviceName, rank);
        } else {
            QCC_DbgPrintf(("%s: Received a non-leader announcement", __func__));
        }
    }
}

static const char* interfaces[] =
{
    ControllerServiceInterfaceName,
    ControllerServiceLampInterfaceName,
    ControllerServiceLampGroupInterfaceName,
    ControllerServicePresetInterfaceName,
    ControllerServiceSceneInterfaceName,
    ControllerServiceMasterSceneInterfaceName,
    ConfigServiceInterfaceName,
    AboutInterfaceName
};

ControllerClient::ControllerClient(
    ajn::BusAttachment& bus,
    ControllerClientCallback& clientCB) :
    bus(bus),
    busHandler(new ControllerClientBusHandler(*this)),
    callback(clientCB),
    controllerServiceManagerPtr(NULL),
    lampManagerPtr(NULL),
    lampGroupManagerPtr(NULL),
    presetManagerPtr(NULL),
    sceneManagerPtr(NULL),
    masterSceneManagerPtr(NULL),
    stopped(true),
    timeStopped(0)
{
    currentLeader.Clear();
    bus.RegisterBusListener(*busHandler);
}

ControllerClientStatus ControllerClient::Start(void)
{
    ControllerClientStatus clientStatus = CONTROLLER_CLIENT_OK;

    if (timeStopped != 0) {
        if ((GetTimestampInMs() - timeStopped) <= (40 * 1000)) {
            QCC_DbgPrintf(("%s: Start() was called within 40s from when Stop() was called", __func__));
            return CONTROLLER_CLIENT_ERR_RETRY;
        }
    }

    QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *busHandler, interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    QCC_DbgPrintf(("%s: RegisterAnnounceHandler: %s\n", __func__, QCC_StatusText(status)));
    if (status == ER_OK) {
        stopped = false;
    } else {
        clientStatus = CONTROLLER_CLIENT_ERR_FAILURE;
    }
    return clientStatus;
}

ControllerClient::~ControllerClient()
{
    Stop();

    services::AnnouncementRegistrar::UnRegisterAllAnnounceHandlers(bus);

    bus.UnregisterBusListener(*busHandler);

    if (busHandler) {
        delete busHandler;
        busHandler = NULL;
    }

    RemoveSignalHandlers();
    RemoveMethodHandlers();
}

uint32_t ControllerClient::GetVersion(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return CONTROLLER_CLIENT_VERSION;
}

bool ControllerClient::JoinSessionWithAnotherLeader(Rank currentLeaderRank, uint64_t timestamp)
{
    QCC_DbgPrintf(("%s: Current Leader Rank %s timestamp = %llu", __func__, currentLeaderRank.c_str(), timestamp));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client is stopped. Returning.", __func__));
        return true;
    }

    ControllerEntry entry;
    entry.Clear();

    leadersMapLock.Lock();
    if (currentLeaderRank.IsInitialized()) {
        Leaders::iterator it = leadersMap.find(currentLeaderRank);
        if ((it != leadersMap.end()) && (it->second.announcementTimestamp == timestamp)) {
            QCC_DbgPrintf(("%s: Removing entry for rank %s from leadersMap", __func__, currentLeaderRank.c_str()));
            leadersMap.erase(it);
        } else {
            QCC_DbgPrintf(("%s: Entry not found / not removed due to timestamp not matching", __func__));
        }
    }
    for (Leaders::reverse_iterator rit = leadersMap.rbegin(); rit != leadersMap.rend(); rit++) {
        entry = rit->second;
        QCC_DbgPrintf(("%s: Found a leader with rank %s", __func__, rit->second.rank.c_str()));
        break;
    }
    leadersMapLock.Unlock();

    if (entry.rank.IsInitialized()) {
        ControllerEntry* context = new ControllerEntry;
        if (!context) {
            QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
            QCC_DbgPrintf(("%s: Returning true", __func__));
            return true;
        }
        *context = entry;
        SessionOpts opts;
        opts.isMultipoint = true;
        QStatus status = bus.JoinSessionAsync(entry.busName.c_str(), entry.port, busHandler, opts, busHandler, context);
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: JoinSessionAsync failed", __func__));
            delete context;
            QCC_DbgPrintf(("%s: calling to ConnectToControllerServiceFailedCB(deviceID=%s,deviceName=%s)\n", __func__, entry.deviceID.c_str(), entry.deviceName.c_str()));
            callback.ConnectToControllerServiceFailedCB(entry.deviceID, entry.deviceName);

            bool emptyList = false;

            leadersMapLock.Lock();
            Leaders::iterator it = leadersMap.find(entry.rank);
            if (it != leadersMap.end()) {
                leadersMap.erase(it);
            }
            if (leadersMap.empty()) {
                emptyList = true;
            }
            leadersMapLock.Unlock();

            if (emptyList) {
                QCC_DbgPrintf(("%s: Returning true", __func__));
                return true;
            } else {
                QCC_DbgPrintf(("%s: Returning false", __func__));
                return false;
            }
        } else {
            QCC_DbgPrintf(("%s: JoinSessionAsync request successful with leader of rank %s", __func__, entry.rank.c_str()));
            currentLeaderLock.Lock();
            currentLeader.controllerDetails = entry;
            currentLeaderLock.Unlock();
            QCC_DbgPrintf(("%s: Returning true", __func__));
            return true;
        }
    } else {
        QCC_DbgPrintf(("%s: Returning true", __func__));
        return true;
    }
}

void ControllerClient::OnSessionMemberRemoved(ajn::SessionId sessionId, const char* uniqueName)
{
    bool leaveSession = false;

    currentLeaderLock.Lock();
    QCC_DbgPrintf(("%s: Received sessionId(%d), uniqueName(%s)", __func__, sessionId, uniqueName));
    QCC_DbgPrintf(("%s: Current Leader sessionId(%d), currentLeader.controllerDetails.busName(%s)", __func__, currentLeader.sessionId, currentLeader.controllerDetails.busName.c_str()));
    if ((currentLeader.controllerDetails.busName == uniqueName) && (currentLeader.sessionId == sessionId)) {
        QCC_DbgPrintf(("%s: Setting leaveSession", __func__));
        leaveSession = true;
    } else {
        QCC_DbgPrintf(("%s: Ignoring spurious OnSessionMemberRemoved", __func__));
    }
    currentLeaderLock.Unlock();

    if (leaveSession) {
        QCC_DbgPrintf(("%s: Attempting DoLeaveSessionAsync", __func__));
        DoLeaveSessionAsync(sessionId);
        QCC_DbgPrintf(("%s: After DoLeaveSessionAsync", __func__));
        OnSessionLost(sessionId);
    }
}


void ControllerClient::OnSessionLost(ajn::SessionId sessionID)
{
    QCC_DbgPrintf(("OnSessionLost(%u)\n", sessionID));

    LSFString deviceName;
    LSFString deviceID;
    Rank currentLeaderRank;
    uint64_t timestamp = 0;

    deviceName.clear();
    deviceID.clear();

    currentLeaderLock.Lock();
    if (currentLeader.sessionId == sessionID) {
        deviceName = currentLeader.controllerDetails.deviceName;
        deviceID = currentLeader.controllerDetails.deviceID;
        currentLeaderRank = currentLeader.controllerDetails.rank;
        timestamp = currentLeader.controllerDetails.announcementTimestamp;
        currentLeader.Clear();
    }
    currentLeaderLock.Unlock();

    if (!deviceID.empty()) {
        QCC_DbgPrintf(("%s: calling to DisconnectedFromControllerServiceCB(deviceID=%s,deviceName=%s)\n", __func__,  deviceID.c_str(), deviceName.c_str()));
        callback.DisconnectedFromControllerServiceCB(deviceID, deviceName);
    }

    if (currentLeaderRank.IsInitialized()) {
        while (!(JoinSessionWithAnotherLeader(currentLeaderRank, timestamp))) ;
        QCC_DbgPrintf(("%s: Exiting JoinSessionWithAnotherLeader cycle", __func__));
    }
}

void ControllerClient::SignalWithArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }

    SignalDispatcherMap::iterator it = signalHandlers.find(message->GetMemberName());
    if (it != signalHandlers.end()) {
        SignalHandlerBase* handler = it->second;

        size_t numInputArgs;
        const MsgArg* inputArgs;
        message->GetArgs(numInputArgs, inputArgs);

        if (CheckNumArgsInMessage(numInputArgs, 1) != LSF_OK) {
            return;
        }

        LSFStringList idList;

        MsgArg* idArgs;
        size_t numIds;
        inputArgs[0].Get("as", &numIds, &idArgs);

        for (size_t i = 0; i < numIds; ++i) {
            char* tempId;
            idArgs[i].Get("s", &tempId);
            idList.push_back(LSFString(tempId));
        }

        handler->Handle(idList);
    }
}

void ControllerClient::NameChangedSignalDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }

    NameChangedSignalDispatcherMap::iterator it = nameChangedSignalHandlers.find(message->GetMemberName());
    if (it != nameChangedSignalHandlers.end()) {
        NameChangedSignalHandlerBase* handler = it->second;

        size_t numInputArgs;
        const MsgArg* inputArgs;
        message->GetArgs(numInputArgs, inputArgs);

        if (CheckNumArgsInMessage(numInputArgs, 2) != LSF_OK) {
            return;
        }

        char* id;
        char* name;
        inputArgs[0].Get("s", &id);
        inputArgs[1].Get("s", &name);

        LSFString lampId = LSFString(id);
        LSFString lampName = LSFString(name);

        handler->Handle(lampId, lampName);
    }
}

void ControllerClient::StateChangedSignalDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }

    StateChangedSignalDispatcherMap::iterator it = stateChangedSignalHandlers.find(message->GetMemberName());
    if (it != stateChangedSignalHandlers.end()) {
        StateChangedSignalHandlerBase* handler = it->second;

        size_t numInputArgs;
        const MsgArg* inputArgs;
        message->GetArgs(numInputArgs, inputArgs);

        if (CheckNumArgsInMessage(numInputArgs, 2) != LSF_OK) {
            return;
        }

        char* id;
        inputArgs[0].Get("s", &id);

        LampState state(inputArgs[1]);

        LSFString lampId = LSFString(id);

        handler->Handle(lampId, state);
    }
}

void ControllerClient::SignalWithoutArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }

    NoArgSignalDispatcherMap::iterator it = noArgSignalHandlers.find(message->GetMemberName());
    if (it != noArgSignalHandlers.end()) {
        NoArgSignalHandlerBase* handler = it->second;
        handler->Handle();
    }
}

void ControllerClient::OnSessionJoined(QStatus status, ajn::SessionId sessionId, void* context)
{
    ErrorCodeList errorList;

    bus.EnableConcurrentCallbacks();

    bool leaveSession = false;

    ControllerEntry* joined = static_cast<ControllerEntry*>(context);

    LSFString deviceName;
    deviceName.clear();

    if (joined) {
        QCC_DbgPrintf(("%s: sessionId= %u status=%s\n", __func__, sessionId, QCC_StatusText(status)));
        currentLeaderLock.Lock();

        if ((joined->deviceID == currentLeader.controllerDetails.deviceID) && (joined->announcementTimestamp == currentLeader.controllerDetails.announcementTimestamp)) {
            /*
             * This is to account for the case when the device name of Controller Service changes when a
             * Join Session with the Controller Service is in progress
             */
            deviceName = currentLeader.controllerDetails.deviceName;

            QCC_DbgPrintf(("%s: Response from the current leader", __func__));
            if (status == ER_OK) {
                currentLeader.proxyObject = ProxyBusObject(bus, currentLeader.controllerDetails.busName.c_str(), ControllerServiceObjectPath, sessionId);
                status = currentLeader.proxyObject.IntrospectRemoteObject();
                if (status == ER_OK) {
                    currentLeader.sessionId = sessionId;
                    AddMethodHandlers();
                    AddSignalHandlers();
                } else {
                    QCC_LogError(status, ("%s: IntrospectRemoteObject failed", __func__));
                    leaveSession = true;
                }
            }
        }

        currentLeaderLock.Unlock();

        if (!deviceName.empty()) {
            if (ER_OK == status) {
                uint32_t linkTimeout = LSF_MIN_LINK_TIMEOUT_IN_SECONDS;
                QStatus tempStatus = bus.SetLinkTimeout(sessionId, linkTimeout);
                if (tempStatus != ER_OK) {
                    QCC_LogError(tempStatus, ("%s: SetLinkTimeout failed", __func__));
                    bus.LeaveSession(sessionId);
                    currentLeaderLock.Lock();
                    currentLeader.Clear();
                    currentLeaderLock.Unlock();
                    while (!(JoinSessionWithAnotherLeader())) ;
                    QCC_DbgPrintf(("%s: Exiting JoinSessionWithAnotherLeader cycle", __func__));
                } else {
                    QCC_DbgPrintf(("%s: calling to ConnectedToControllerServiceCB(deviceId=%s,deviceName=%s)\n", __func__, joined->deviceID.c_str(), deviceName.c_str()));
                    callback.ConnectedToControllerServiceCB(joined->deviceID, deviceName);
                }
            } else {
                QCC_DbgPrintf(("%s:calling to ConnectToControllerServiceFailedCB(deviceId=%s,deviceName=%s)\n", __func__, joined->deviceID.c_str(), deviceName.c_str()));
                callback.ConnectToControllerServiceFailedCB(joined->deviceID, deviceName);
                currentLeaderLock.Lock();
                currentLeader.Clear();
                currentLeaderLock.Unlock();
                if (leaveSession) {
                    DoLeaveSessionAsync(sessionId);
                }
                if (status == ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED) {
                    QCC_DbgPrintf(("%s: Got %s. Retrying Join Session.", __func__, QCC_StatusText(status)));
                    while (!(JoinSessionWithAnotherLeader())) ;
                } else {
                    while (!(JoinSessionWithAnotherLeader(joined->rank, joined->announcementTimestamp))) ;
                }
                QCC_DbgPrintf(("%s: Exiting JoinSessionWithAnotherLeader cycle", __func__));
            }
        } else {
            if (status == ER_OK) {
                QCC_DbgPrintf(("%s: Got a JoinSessionCB from someone whom we don't think is the current leader", __func__));
                DoLeaveSessionAsync(sessionId);
            }
        }

        delete joined;
    }
}

void ControllerClient::OnAnnounced(SessionPort port, const char* busName, const char* deviceID, const char* deviceName, Rank rank)
{
    QCC_DbgPrintf(("%s: port=%u, busName=%s, deviceID=%s, deviceName=%s, rank=%s", __func__, port, busName, deviceID, deviceName, rank.c_str()));

    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client is stopped. Returning without processing announcement.", __func__));
        return;
    }

    bool nameChanged = false;
    ajn::SessionId sessionId = 0;

    Rank currentLeaderRank;
    uint64_t timestamp = 0;

    ControllerEntry entry;
    entry.port = port;
    entry.deviceID = deviceID;
    entry.deviceName = deviceName;
    entry.rank = rank;
    entry.busName = busName;
    entry.announcementTimestamp = GetTimestampInMs();

    currentLeaderLock.Lock();
    // if the name of the current CS has changed...
    currentLeaderRank = currentLeader.controllerDetails.rank;
    timestamp = currentLeader.controllerDetails.announcementTimestamp;
    sessionId = currentLeader.sessionId;
    if (deviceID == currentLeader.controllerDetails.deviceID) {
        QCC_DbgPrintf(("%s: Same deviceID as current leader %s", __func__, deviceID));
        if (deviceName != currentLeader.controllerDetails.deviceName) {
            currentLeader.controllerDetails.deviceName = deviceName;
            if (currentLeader.sessionId != 0) {
                nameChanged = true;
            }
            QCC_DbgPrintf(("%s: Current leader name change", __func__));
        }
    }
    currentLeaderLock.Unlock();

    if (nameChanged) {
        QCC_DbgPrintf(("%s: calling to ControllerServiceNameChangedCB(%s, %s)\n", __func__, entry.deviceID.c_str(), entry.deviceName.c_str()));
        controllerServiceManagerPtr->callback.ControllerServiceNameChangedCB(entry.deviceID, entry.deviceName);
    }

    leadersMapLock.Lock();
    std::pair<Leaders::iterator, bool> ins = leadersMap.insert(std::make_pair(rank, entry));
    if (ins.second == false) {
        ins.first->second = entry;
    }
    leadersMapLock.Unlock();

    if (!currentLeaderRank.IsInitialized()) {
        while (!(JoinSessionWithAnotherLeader())) ;
        QCC_DbgPrintf(("%s: Exiting JoinSessionWithAnotherLeader cycle", __func__));
    } else if (currentLeaderRank < rank) {
        if (sessionId) {
            DoLeaveSessionAsync(sessionId);
            OnSessionLost(sessionId);
        } else {
            currentLeaderLock.Lock();
            currentLeader.Clear();
            currentLeaderLock.Unlock();
            while (!(JoinSessionWithAnotherLeader(currentLeaderRank, timestamp))) ;
            QCC_DbgPrintf(("%s: Exiting JoinSessionWithAnotherLeader cycle", __func__));
        }
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncHelper(
    const char* ifaceName,
    const char* methodName,
    ajn::MessageReceiver* handler,
    ajn::MessageReceiver::ReplyHandler callback,
    const ajn::MsgArg* args,
    size_t numArgs,
    void* context)
{
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;
    currentLeaderLock.Lock();

    if (currentLeader.sessionId) {
        QStatus ajStatus = currentLeader.proxyObject.MethodCallAsync(
            ifaceName,
            methodName,
            handler,
            callback,
            args,
            numArgs,
            context);
        if (ajStatus != ER_OK) {
            status = CONTROLLER_CLIENT_ERR_FAILURE;
            QCC_LogError(ajStatus, ("%s method call to Controller Service failed", methodName));
        }
    } else {
        // this is no longer available
        status = CONTROLLER_CLIENT_ERR_NOT_CONNECTED;
    }

    currentLeaderLock.Unlock();
    return status;
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
    if (!methodNameContext) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return CONTROLLER_CLIENT_ERR_FAILURE;
    }
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeAndListOfIDs),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeAndListOfIDs(Message& message, void* context)
{
    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeAndListOfIDsDispatcherMap::iterator it = methodReplyWithResponseCodeAndListOfIDsHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeAndListOfIDsHandlers.end()) {
                MethodReplyWithResponseCodeAndListOfIDsHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                if (CheckNumArgsInMessage(numInputArgs, 2) != LSF_OK) {
                    return;
                }

                LSFStringList idList;
                LSFResponseCode responseCode;

                inputArgs[0].Get("u", &responseCode);

                MsgArg* idArgs;
                size_t numIds;
                inputArgs[1].Get("as", &numIds, &idArgs);

                for (size_t i = 0; i < numIds; ++i) {
                    char* tempId;
                    idArgs[i].Get("s", &tempId);
                    idList.push_back(LSFString(tempId));
                }

                handler->Handle(responseCode, idList);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            QCC_DbgPrintf(("%s:calling to ControllerClientErrorCB()\n", __func__));
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeIDAndName(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
    if (!methodNameContext) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return CONTROLLER_CLIENT_ERR_FAILURE;
    }
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeIDAndName),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeIDAndName(Message& message, void* context)
{
    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeIDAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDAndNameHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeIDAndNameHandlers.end()) {
                MethodReplyWithResponseCodeIDAndNameHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                if (CheckNumArgsInMessage(numInputArgs, 3) != LSF_OK) {
                    return;
                }

                char* uniqueId = NULL;
                char* name = NULL;

                LSFResponseCode responseCode;
                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &uniqueId);
                inputArgs[2].Get("s", &name);

                LSFString lsfId = LSFString(uniqueId);
                LSFString lsfName = LSFString(name);

                handler->Handle(responseCode, lsfId, lsfName);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB()\n", __func__));
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndID(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    LSFString* methodNameContext = new LSFString(methodName);
    if (!methodNameContext) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return CONTROLLER_CLIENT_ERR_FAILURE;
    }
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeAndID),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeAndID(Message& message, void* context)
{
    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeAndIDDispatcherMap::iterator it = methodReplyWithResponseCodeAndIDHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeAndIDHandlers.end()) {
                MethodReplyWithResponseCodeAndIDHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                if (CheckNumArgsInMessage(numInputArgs, 2) != LSF_OK) {
                    return;
                }

                char* id;

                LSFResponseCode responseCode;
                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &id);

                LSFString lsfId = LSFString(id);

                handler->Handle(responseCode, lsfId);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB()\n", __func__));
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithUint32Value(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
    if (!methodNameContext) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return CONTROLLER_CLIENT_ERR_FAILURE;
    }
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithUint32Value),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithUint32Value(Message& message, void* context)
{
    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithUint32ValueDispatcherMap::iterator it = methodReplyWithUint32ValueHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithUint32ValueHandlers.end()) {
                MethodReplyWithUint32ValueHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                if (CheckNumArgsInMessage(numInputArgs, 1) != LSF_OK) {
                    return;
                }

                uint32_t value;
                inputArgs[0].Get("u", &value);
                handler->Handle(value);
            } else {
                QCC_LogError(ER_FAIL, ("%s: Did not find handler", __func__));
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB()\n", __func__));
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
    if (!methodNameContext) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return CONTROLLER_CLIENT_ERR_FAILURE;
    }
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeIDLanguageAndName),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeIDLanguageAndName(Message& message, void* context)
{
    if (stopped) {
        QCC_DbgPrintf(("%s: Controller Client stopped", __func__));
        return;
    }
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDLanguageAndNameHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeIDLanguageAndNameHandlers.end()) {
                MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                if (CheckNumArgsInMessage(numInputArgs, 4) != LSF_OK) {
                    return;
                }

                char* id;
                char* lang;
                char* name;

                LSFResponseCode responseCode;
                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &id);
                inputArgs[2].Get("s", &lang);
                inputArgs[3].Get("s", &name);

                LSFString lsfId = LSFString(id);
                LSFString language = LSFString(lang);
                LSFString lsfName = LSFString(name);

                handler->Handle(responseCode, lsfId, language, lsfName);
            } else {
                QCC_LogError(ER_FAIL, ("%s: Did not find handler", __func__));
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB()\n", __func__));
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::Stop(void)
{
    QCC_DbgPrintf(("%s", __func__));

    stopped = true;
    timeStopped = GetTimestampInMs();

    LSFString deviceName;
    LSFString deviceID;
    SessionId sessionId = 0;

    deviceName.clear();
    deviceID.clear();

    currentLeaderLock.Lock();
    sessionId = currentLeader.sessionId;
    deviceName = currentLeader.controllerDetails.deviceName;
    deviceID = currentLeader.controllerDetails.deviceID;
    currentLeader.Clear();
    currentLeaderLock.Unlock();

    bus.UnregisterAllHandlers(this);

    if (sessionId) {
        DoLeaveSessionAsync(sessionId);
        QCC_DbgPrintf(("%s: calling to DisconnectedFromControllerServiceCB(%s,%s)\n", __func__, deviceID.c_str(), deviceName.c_str()));
        callback.DisconnectedFromControllerServiceCB(deviceID, deviceName);
    }

    leadersMapLock.Lock();
    leadersMap.clear();
    leadersMapLock.Unlock();

    ControllerClientStatus clientStatus = CONTROLLER_CLIENT_OK;
    QStatus status = services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *busHandler, interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    QCC_DbgPrintf(("%s: UnRegisterAnnounceHandler: %s\n", __func__, QCC_StatusText(status)));
    if (status != ER_OK) {
        clientStatus = CONTROLLER_CLIENT_ERR_FAILURE;
    }

    return clientStatus;
}

void ControllerClient::AddMethodHandlers()
{
    if (controllerServiceManagerPtr) {
        AddNoArgSignalHandler("ControllerServiceLightingReset", controllerServiceManagerPtr, &ControllerServiceManager::ControllerServiceLightingReset);

        AddMethodReplyWithUint32ValueHandler("GetControllerServiceVersion", controllerServiceManagerPtr, &ControllerServiceManager::GetControllerServiceVersionReply);
        AddMethodReplyWithUint32ValueHandler("LightingResetControllerService", controllerServiceManagerPtr, &ControllerServiceManager::LightingResetControllerServiceReply);
    }

    if (lampManagerPtr) {
        AddNameChangedSignalHandler("LampNameChanged", lampManagerPtr, &LampManager::LampNameChanged);
        AddStateChangedSignalHandler("LampStateChanged", lampManagerPtr, &LampManager::LampStateChanged);
        AddSignalHandler("LampsFound", lampManagerPtr, &LampManager::LampsFound);
        AddSignalHandler("LampsLost", lampManagerPtr, &LampManager::LampsLost);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampIDs", lampManagerPtr, &LampManager::GetAllLampIDsReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampStateField", lampManagerPtr, &LampManager::TransitionLampStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampStateField", lampManagerPtr, &LampManager::ResetLampStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("SetLampName", lampManagerPtr, &LampManager::SetLampNameReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampName", lampManagerPtr, &LampManager::GetLampNameReply);
        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampManufacturer", lampManagerPtr, &LampManager::GetLampManufacturerReply);

        AddMethodReplyWithResponseCodeAndIDHandler("ResetLampState", lampManagerPtr, &LampManager::ResetLampStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampState", lampManagerPtr, &LampManager::TransitionLampStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampStateToPreset", lampManagerPtr, &LampManager::TransitionLampStateToPresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampWithState", lampManagerPtr, &LampManager::PulseLampWithStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampWithPreset", lampManagerPtr, &LampManager::PulseLampWithPresetReply);
    }

    if (lampGroupManagerPtr) {
        AddSignalHandler("LampGroupsNameChanged", lampGroupManagerPtr, &LampGroupManager::LampGroupsNameChanged);
        AddSignalHandler("LampGroupsCreated", lampGroupManagerPtr, &LampGroupManager::LampGroupsCreated);
        AddSignalHandler("LampGroupsUpdated", lampGroupManagerPtr, &LampGroupManager::LampGroupsUpdated);
        AddSignalHandler("LampGroupsDeleted", lampGroupManagerPtr, &LampGroupManager::LampGroupsDeleted);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampGroupIDs", lampGroupManagerPtr, &LampGroupManager::GetAllLampGroupIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampGroupName", lampGroupManagerPtr, &LampGroupManager::GetLampGroupNameReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampGroupStateField", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampGroupStateField", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupStateFieldReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetLampGroupName", lampGroupManagerPtr, &LampGroupManager::SetLampGroupNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ResetLampGroupState", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupState", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupStateToPreset", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateToPresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateLampGroup", lampGroupManagerPtr, &LampGroupManager::CreateLampGroupReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateLampGroup", lampGroupManagerPtr, &LampGroupManager::UpdateLampGroupReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteLampGroup", lampGroupManagerPtr, &LampGroupManager::DeleteLampGroupReply);

        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampGroupWithState", lampGroupManagerPtr, &LampGroupManager::PulseLampGroupWithStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampGroupWithPreset", lampGroupManagerPtr, &LampGroupManager::PulseLampGroupWithPresetReply);
    }

    if (presetManagerPtr) {
        AddNoArgSignalHandler("DefaultLampStateChanged", presetManagerPtr, &PresetManager::DefaultLampStateChanged);
        AddSignalHandler("PresetsNameChanged", presetManagerPtr, &PresetManager::PresetsNameChanged);
        AddSignalHandler("PresetsCreated", presetManagerPtr, &PresetManager::PresetsCreated);
        AddSignalHandler("PresetsUpdated", presetManagerPtr, &PresetManager::PresetsUpdated);
        AddSignalHandler("PresetsDeleted", presetManagerPtr, &PresetManager::PresetsDeleted);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllPresetIDs", presetManagerPtr, &PresetManager::GetAllPresetIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetPresetName", presetManagerPtr, &PresetManager::GetPresetNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetPresetName", presetManagerPtr, &PresetManager::SetPresetNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreatePreset", presetManagerPtr, &PresetManager::CreatePresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdatePreset", presetManagerPtr, &PresetManager::UpdatePresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeletePreset", presetManagerPtr, &PresetManager::DeletePresetReply);

        AddMethodReplyWithUint32ValueHandler("SetDefaultLampState", presetManagerPtr, &PresetManager::SetDefaultLampStateReply);
    }

    if (sceneManagerPtr) {
        AddSignalHandler("ScenesNameChanged", sceneManagerPtr, &SceneManager::ScenesNameChanged);
        AddSignalHandler("ScenesCreated", sceneManagerPtr, &SceneManager::ScenesCreated);
        AddSignalHandler("ScenesUpdated", sceneManagerPtr, &SceneManager::ScenesUpdated);
        AddSignalHandler("ScenesDeleted", sceneManagerPtr, &SceneManager::ScenesDeleted);
        AddSignalHandler("ScenesApplied", sceneManagerPtr, &SceneManager::ScenesApplied);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllSceneIDs", sceneManagerPtr, &SceneManager::GetAllSceneIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetSceneName", sceneManagerPtr, &SceneManager::GetSceneNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetSceneName", sceneManagerPtr, &SceneManager::SetSceneNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateScene", sceneManagerPtr, &SceneManager::CreateSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateScene", sceneManagerPtr, &SceneManager::UpdateSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteScene", sceneManagerPtr, &SceneManager::DeleteSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ApplyScene", sceneManagerPtr, &SceneManager::ApplySceneReply);
    }

    if (masterSceneManagerPtr) {
        AddSignalHandler("MasterScenesNameChanged", masterSceneManagerPtr, &MasterSceneManager::MasterScenesNameChanged);
        AddSignalHandler("MasterScenesCreated", masterSceneManagerPtr, &MasterSceneManager::MasterScenesCreated);
        AddSignalHandler("MasterScenesUpdated", masterSceneManagerPtr, &MasterSceneManager::MasterScenesUpdated);
        AddSignalHandler("MasterScenesDeleted", masterSceneManagerPtr, &MasterSceneManager::MasterScenesDeleted);
        AddSignalHandler("MasterScenesApplied", masterSceneManagerPtr, &MasterSceneManager::MasterScenesApplied);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllMasterSceneIDs", masterSceneManagerPtr, &MasterSceneManager::GetAllMasterSceneIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetMasterSceneName", masterSceneManagerPtr, &MasterSceneManager::GetMasterSceneNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetMasterSceneName", masterSceneManagerPtr, &MasterSceneManager::SetMasterSceneNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateMasterScene", masterSceneManagerPtr, &MasterSceneManager::CreateMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateMasterScene", masterSceneManagerPtr, &MasterSceneManager::UpdateMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteMasterScene", masterSceneManagerPtr, &MasterSceneManager::DeleteMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ApplyMasterScene", masterSceneManagerPtr, &MasterSceneManager::ApplyMasterSceneReply);
    }
}

void ControllerClient::AddSignalHandlers()
{
    const InterfaceDescription* controllerServiceInterface = currentLeader.proxyObject.GetInterface(ControllerServiceInterfaceName);
    const InterfaceDescription* controllerServiceLampInterface = currentLeader.proxyObject.GetInterface(ControllerServiceLampInterfaceName);
    const InterfaceDescription* controllerServiceLampGroupInterface = currentLeader.proxyObject.GetInterface(ControllerServiceLampGroupInterfaceName);
    const InterfaceDescription* controllerServicePresetInterface = currentLeader.proxyObject.GetInterface(ControllerServicePresetInterfaceName);
    const InterfaceDescription* controllerServiceSceneInterface = currentLeader.proxyObject.GetInterface(ControllerServiceSceneInterfaceName);
    const InterfaceDescription* controllerServiceMasterSceneInterface = currentLeader.proxyObject.GetInterface(ControllerServiceMasterSceneInterfaceName);

    const SignalEntry signalEntries[] = {
        { controllerServiceInterface->GetMember("ControllerServiceLightingReset"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
        { controllerServiceLampInterface->GetMember("LampNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::NameChangedSignalDispatcher) },
        { controllerServiceLampInterface->GetMember("LampStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::StateChangedSignalDispatcher) },
        { controllerServiceLampInterface->GetMember("LampsFound"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampInterface->GetMember("LampsLost"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("DefaultLampStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) }
    };

    // must call UnregisterAllHandlers in order to prevent multiple registrations for the same signal
    bus.UnregisterAllHandlers(this);

    for (size_t i = 0; i < (sizeof(signalEntries) / sizeof(SignalEntry)); ++i) {
        bus.RegisterSignalHandler(
            this,
            signalEntries[i].handler,
            signalEntries[i].member,
            ControllerServiceObjectPath);
    }
}

void ControllerClient::RemoveSignalHandlers()
{
    DeleteSignalHandlers();
    DeleteNoArgSignalHandlers();
    DeleteNameChangedSignalHandlers();
    DeleteStateChangedSignalHandlers();
}

LSFResponseCode ControllerClient::CheckNumArgsInMessage(uint32_t receivedNumArgs, uint32_t expectedNumArgs)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;
    if (receivedNumArgs != expectedNumArgs) {
        QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply or signal", __func__));
        responseCode = LSF_ERR_REPLY_WITH_INVALID_ARGS;
        /*
         * Send an error callback to the application if the response was not LSF_OK
         */
        ErrorCodeList errors;
        errors.push_back(ERROR_MESSAGE_WITH_INVALID_ARGS);
        QCC_DbgPrintf(("%s: calling to ControllerClientErrorCB\n", __func__));
        callback.ControllerClientErrorCB(errors);
    }
    return responseCode;
}

}
