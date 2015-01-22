#ifndef _CONTROLLER_CLIENT_H_
#define _CONTROLLER_CLIENT_H_
/**
 * \defgroup ControllerClient
 * Client controller code
 */
/**
 * \ingroup ControllerClient
 */
/**
 * \file  lighting_controller_client/inc/ControllerClient.h
 * This file provides definitions for the Lighting Controller Client
 */
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

#include <map>
#include <list>
#include <signal.h>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/Session.h>
#include <alljoyn/MessageReceiver.h>
#include <qcc/platform.h>

#include <ControllerClientDefs.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>
#include <ControllerServiceManager.h>
#include <Rank.h>

namespace lsf {
/**
 * controller client version
 */
#define CONTROLLER_CLIENT_VERSION 1

/**
 * Abstract base class implemented by User Application Developers. \n
 * The callbacks defined in this class allow the User Application
 * to be informed about some Controller Client operations.
 */
class ControllerClientCallback {
  public:
    /**
     * Destructor
     */
    virtual ~ControllerClientCallback() { }

    /**
     * Indicates that the Controller Client has successfully set up an AllJoyn session
     * with a Controller Service
     */
    virtual void ConnectedToControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) { }

    /**
     * Indicates that the Controller Client was unsuccessful in setting up an AllJoyn session
     * with a Controller Service
     */
    virtual void ConnectToControllerServiceFailedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) { }

    /**
     * Indicates that the Controller Client has disconnected from a Controller Service
     */
    virtual void DisconnectedFromControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) { }

    /**
     *  Indicates that an internal error has occurred in the Controller Client
     *
     *  @param errorCodeList   List of Error Codes
     */
    virtual void ControllerClientErrorCB(const ErrorCodeList& errorCodeList) { }
};

/**
 * This class allows the User Application to initialize the Lighting
 * Controller Client operations
 */
class ControllerClient : public ajn::MessageReceiver {

    friend class ControllerServiceManager;
    friend class LampManager;
    friend class LampGroupManager;
    friend class PresetManager;
    friend class SceneManager;
    friend class MasterSceneManager;

    class ControllerClientBusHandler;

  public:

    /**
     * Constructor
     * @param bus - reference to bus attachment object
     * @param clientCB - reference to ControllerClientCallback
     */
    ControllerClient(
        ajn::BusAttachment& bus,
        ControllerClientCallback& clientCB);

    /**
     * Destructor
     */
    ~ControllerClient();

    /**
     *  Get the version of the Lighting Controller Client.
     *
     *  @return The Lighting Controller Client version
     */
    uint32_t GetVersion(void);

    /**
     *  Stop the Lighting Controller Client. This will cause the
     *  Controller Client to tear-down any existent session with
     *  a Controller Service, clear the leadersMap contents and
     *  unregister announce handlers. The user application should
     *  call this API when it detects that the device has disconnected
     *  from Wi-Fi and/or the data network
     */
    ControllerClientStatus Stop(void);

    /**
     *  Start the Lighting Controller Client. This will cause the
     *  Controller Client to register for announce handlers.
     *  The user application should call this API at system start
     *  after confirming that the device is connected to Wi-Fi and/or
     *  data network
     *  OR
     *  when it detects that the device has re-connected to Wi-Fi and/or
     *  the data network after it was disconnected
     */
    ControllerClientStatus Start(void);

  private:

    void DoLeaveSessionAsync(ajn::SessionId sessionId);

    void LeaveSessionAsyncReplyHandler(ajn::Message& message, void* context);

    /**
     * Internal callback invoked when an announcement is received from a Controller
     * Service Leader.
     */
    void OnAnnounced(ajn::SessionPort port, const char* busName, const char* deviceID, const char* deviceName, Rank rank);

    /**
     * Internal callback invoked when a session with a Controller
     * Service Leader is lost.
     */
    void OnSessionLost(ajn::SessionId sessionID);

    void OnSessionMemberRemoved(ajn::SessionId sessionId, const char* uniqueName);

    void SignalWithArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message);

    void NameChangedSignalDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message);

    void StateChangedSignalDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message);

    void SignalWithoutArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message);

    void HandlerForMethodReplyWithResponseCodeAndListOfIDs(ajn::Message& message, void* context);

    void HandlerForMethodReplyWithResponseCodeIDAndName(ajn::Message& message, void* context);

    void HandlerForMethodReplyWithResponseCodeAndID(ajn::Message& message, void* context);

    void HandlerForMethodReplyWithUint32Value(ajn::Message& message, void* context);

    void HandlerForMethodReplyWithResponseCodeIDLanguageAndName(ajn::Message& message, void* context);

    /**
     * Internal callback invoked when a session is joined with a Controller
     * Service Leader.
     */
    void OnSessionJoined(QStatus status, ajn::SessionId sessionID, void* context);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    template <typename OBJ>
    ControllerClientStatus MethodCallAsync(const char* ifaceName,
                                           const char* methodName,
                                           OBJ * receiver,
                                           void (OBJ::* replyFunc)(ajn::Message & message),
                                           const ajn::MsgArg * args = NULL,
                                           size_t numArgs = 0);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
        const char* ifaceName,
        const char* methodName,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncForReplyWithResponseCodeIDAndName(
        const char* ifaceName,
        const char* methodName,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncForReplyWithResponseCodeAndID(
        const char* ifaceName,
        const char* methodName,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncForReplyWithUint32Value(
        const char* ifaceName,
        const char* methodName,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0);

    /**
     * Helper template to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
        const char* ifaceName,
        const char* methodName,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0);

    /**
     * Check if the receivedNumArgs is same as expectedNumArgs and return a response
     * code accordingly
     */
    LSFResponseCode CheckNumArgsInMessage(uint32_t receivedNumArgs, uint32_t expectedNumArgs);

    /**
     * Reference to the AllJoyn BusAttachment received from the
     * User Application.
     */
    ajn::BusAttachment& bus;

    /**
     * Handler used to receive messages from the AllJoyn Bus.
     */
    ControllerClientBusHandler* busHandler;

    /**
     * ControllerClientCallback sends the Controller Client specific callbacks
     * to the User Application.
     */
    ControllerClientCallback& callback;

    /*
     * The ID of the CS we are currently trying to connect to.
     */
    struct ControllerEntry {
        ajn::SessionPort port;
        qcc::String busName;
        Rank rank;
        LSFString deviceID;
        LSFString deviceName;
        uint64_t announcementTimestamp;

        void Clear(void) {
            busName.clear();
            rank = Rank();
            port = 0;
            deviceID.clear();
            deviceName.clear();
            announcementTimestamp = 0;
        }
    };

    typedef std::map<Rank, ControllerEntry> Leaders;

    Leaders leadersMap;
    Mutex leadersMapLock;

    typedef struct _CurrentLeader {
        ControllerEntry controllerDetails;
        ajn::ProxyBusObject proxyObject;
        ajn::SessionId sessionId;

        void Clear(void) {
            controllerDetails.Clear();
            proxyObject = ProxyBusObject();
            sessionId = 0;
        }
    } CurrentLeader;

    CurrentLeader currentLeader;
    Mutex currentLeaderLock;

    bool JoinSessionWithAnotherLeader(Rank currentLeaderRank = Rank(), uint64_t timestamp = 0);

    /**
     * Pointer to the Controller Service Manager
     */
    ControllerServiceManager* controllerServiceManagerPtr;

    /**
     * Pointer to the Lamp Manager
     */
    LampManager* lampManagerPtr;

    /**
     * Pointer to the Lamp Group Manager
     */
    LampGroupManager* lampGroupManagerPtr;

    /**
     * Pointer to the Preset Manager
     */
    PresetManager* presetManagerPtr;

    /**
     * Pointer to the Scene Manager
     */
    SceneManager* sceneManagerPtr;

    /**
     * Pointer to the Scene Manager
     */
    MasterSceneManager* masterSceneManagerPtr;

    /**
     * Constructor
     */
    ControllerClient();

    /**
     * Constructor
     */
    ControllerClient(const ControllerClient&);

    /**
     * Operator
     */
    ControllerClient& operator=(ControllerClient&);

    /**
     * Helper function to invoke MethodCallAsync on the AllJoyn ProxyBusObject.
     */
    ControllerClientStatus MethodCallAsyncHelper(
        const char* ifaceName,
        const char* methodName,
        ajn::MessageReceiver* handler,
        ajn::MessageReceiver::ReplyHandler callback,
        const ajn::MsgArg* args = NULL,
        size_t numArgs = 0,
        void* context = NULL);

    /**
     * Template for AllJoyn MessageReceiver
     */
    template <typename OBJ, typename T>
    class TypeHandler;

    void AddSignalHandlers();

    void RemoveSignalHandlers();

    /**
     * Template for AllJoyn MessageReceiver
     */
    template <typename OBJ>
    class TypeHandler<OBJ, void> : public ajn::MessageReceiver {
        typedef void (OBJ::* ReplyHandler)(ajn::Message& message);
      public:

        TypeHandler(OBJ* receiver, ReplyHandler handler, ControllerClient* controllerClientPtr) :
            receiver(receiver), handler(handler), controllerClientPtr(controllerClientPtr)
        { }

        void MessageHandler(ajn::Message& message, void* context)
        {
            if (!controllerClientPtr->stopped) {
                controllerClientPtr->bus.EnableConcurrentCallbacks();
                if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
                    (receiver->*(handler))(message);
                } else {
                    ErrorCodeList errorList;
                    errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
                    controllerClientPtr->callback.ControllerClientErrorCB(errorList);
                }
            }
            // nothing owns this object so we need to clean it up here
            delete this;
        }

        OBJ* receiver;
        ReplyHandler handler;
        ControllerClient* controllerClientPtr;
    };

    typedef struct {
        const ajn::InterfaceDescription::Member* member;  /**< Pointer to method's member */
        ajn::MessageReceiver::SignalHandler handler;      /**< Signal implementation */
    } SignalEntry;

    template <typename OBJ>
    void AddSignalHandler(const std::string& signalName, OBJ* obj, void (OBJ::* signal)(LSFStringList &))
    {
        SignalHandlerBase* handler = new SignalHandler<OBJ>(obj, signal);
        std::pair<SignalDispatcherMap::iterator, bool> ins = signalHandlers.insert(std::make_pair(signalName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class SignalHandlerBase {
      public:
        virtual ~SignalHandlerBase() { }
        virtual void Handle(LSFStringList& msg) = 0;
    };

    template <typename OBJ>
    class SignalHandler : public SignalHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFStringList&);

      public:
        SignalHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~SignalHandler() { }

        virtual void Handle(LSFStringList& list) {
            (object->*(handler))(list);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, SignalHandlerBase*> SignalDispatcherMap;
    SignalDispatcherMap signalHandlers;

    void DeleteSignalHandlers(void)
    {
        for (SignalDispatcherMap::iterator it = signalHandlers.begin(); it != signalHandlers.end(); it++) {
            delete it->second;
        }
        signalHandlers.clear();
    }

    template <typename OBJ>
    void AddNameChangedSignalHandler(const std::string& nameChangedSignalName, OBJ* obj, void (OBJ::* nameChangedSignal)(LSFString &, LSFString &))
    {
        NameChangedSignalHandlerBase* handler = new NameChangedSignalHandler<OBJ>(obj, nameChangedSignal);
        std::pair<NameChangedSignalDispatcherMap::iterator, bool> ins = nameChangedSignalHandlers.insert(std::make_pair(nameChangedSignalName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class NameChangedSignalHandlerBase {
      public:
        virtual ~NameChangedSignalHandlerBase() { }
        virtual void Handle(LSFString& id, LSFString& name) = 0;
    };

    template <typename OBJ>
    class NameChangedSignalHandler : public NameChangedSignalHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFString&, LSFString&);

      public:
        NameChangedSignalHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~NameChangedSignalHandler() { }

        virtual void Handle(LSFString& id, LSFString& name) {
            (object->*(handler))(id, name);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, NameChangedSignalHandlerBase*> NameChangedSignalDispatcherMap;
    NameChangedSignalDispatcherMap nameChangedSignalHandlers;

    void DeleteNameChangedSignalHandlers(void)
    {
        for (NameChangedSignalDispatcherMap::iterator it = nameChangedSignalHandlers.begin(); it != nameChangedSignalHandlers.end(); it++) {
            delete it->second;
        }
        nameChangedSignalHandlers.clear();
    }

    template <typename OBJ>
    void AddStateChangedSignalHandler(const std::string& stateChangedSignalState, OBJ* obj, void (OBJ::* stateChangedSignal)(LSFString &, LampState &))
    {
        StateChangedSignalHandlerBase* handler = new StateChangedSignalHandler<OBJ>(obj, stateChangedSignal);
        std::pair<StateChangedSignalDispatcherMap::iterator, bool> ins = stateChangedSignalHandlers.insert(std::make_pair(stateChangedSignalState, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class StateChangedSignalHandlerBase {
      public:
        virtual ~StateChangedSignalHandlerBase() { }
        virtual void Handle(LSFString& id, LampState& state) = 0;
    };

    template <typename OBJ>
    class StateChangedSignalHandler : public StateChangedSignalHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFString&, LampState&);

      public:
        StateChangedSignalHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~StateChangedSignalHandler() { }

        virtual void Handle(LSFString& id, LampState& state) {
            (object->*(handler))(id, state);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, StateChangedSignalHandlerBase*> StateChangedSignalDispatcherMap;
    StateChangedSignalDispatcherMap stateChangedSignalHandlers;

    void DeleteStateChangedSignalHandlers(void)
    {
        for (StateChangedSignalDispatcherMap::iterator it = stateChangedSignalHandlers.begin(); it != stateChangedSignalHandlers.end(); it++) {
            delete it->second;
        }
        stateChangedSignalHandlers.clear();
    }

    template <typename OBJ>
    void AddNoArgSignalHandler(const std::string& signalName, OBJ* obj, void (OBJ::* signal)(void))
    {
        NoArgSignalHandlerBase* handler = new NoArgSignalHandler<OBJ>(obj, signal);
        std::pair<NoArgSignalDispatcherMap::iterator, bool> ins = noArgSignalHandlers.insert(std::make_pair(signalName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class NoArgSignalHandlerBase {
      public:
        virtual ~NoArgSignalHandlerBase() { }
        virtual void Handle(void) = 0;
    };

    template <typename OBJ>
    class NoArgSignalHandler : public NoArgSignalHandlerBase {
        typedef void (OBJ::* HandlerFunction)(void);

      public:
        NoArgSignalHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~NoArgSignalHandler() { }

        virtual void Handle(void) {
            (object->*(handler))();
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, NoArgSignalHandlerBase*> NoArgSignalDispatcherMap;
    NoArgSignalDispatcherMap noArgSignalHandlers;

    void DeleteNoArgSignalHandlers(void)
    {
        for (NoArgSignalDispatcherMap::iterator it = noArgSignalHandlers.begin(); it != noArgSignalHandlers.end(); it++) {
            delete it->second;
        }
        noArgSignalHandlers.clear();
    }

    void AddMethodHandlers();

    template <typename OBJ>
    void AddMethodReplyWithResponseCodeAndListOfIDsHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodReply)(LSFResponseCode &, LSFStringList &))
    {
        MethodReplyWithResponseCodeAndListOfIDsHandlerBase* handler = new MethodReplyWithResponseCodeAndListOfIDsHandler<OBJ>(obj, methodReply);
        std::pair<MethodReplyWithResponseCodeAndListOfIDsDispatcherMap::iterator, bool> ins = methodReplyWithResponseCodeAndListOfIDsHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class MethodReplyWithResponseCodeAndListOfIDsHandlerBase {
      public:
        virtual ~MethodReplyWithResponseCodeAndListOfIDsHandlerBase() { }
        virtual void Handle(LSFResponseCode& responseCode, LSFStringList& idList) = 0;
    };

    template <typename OBJ>
    class MethodReplyWithResponseCodeAndListOfIDsHandler : public MethodReplyWithResponseCodeAndListOfIDsHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFResponseCode&, LSFStringList&);

      public:
        MethodReplyWithResponseCodeAndListOfIDsHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodReplyWithResponseCodeAndListOfIDsHandler() { }

        virtual void Handle(LSFResponseCode& responseCode, LSFStringList& idList) {
            (object->*(handler))(responseCode, idList);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodReplyWithResponseCodeAndListOfIDsHandlerBase*> MethodReplyWithResponseCodeAndListOfIDsDispatcherMap;
    MethodReplyWithResponseCodeAndListOfIDsDispatcherMap methodReplyWithResponseCodeAndListOfIDsHandlers;

    template <typename OBJ>
    void AddMethodReplyWithResponseCodeIDAndNameHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodReply)(LSFResponseCode &, LSFString &, LSFString &))
    {
        MethodReplyWithResponseCodeIDAndNameHandlerBase* handler = new MethodReplyWithResponseCodeIDAndNameHandler<OBJ>(obj, methodReply);
        std::pair<MethodReplyWithResponseCodeIDAndNameDispatcherMap::iterator, bool> ins = methodReplyWithResponseCodeIDAndNameHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class MethodReplyWithResponseCodeIDAndNameHandlerBase {
      public:
        virtual ~MethodReplyWithResponseCodeIDAndNameHandlerBase() { }
        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName) = 0;
    };

    template <typename OBJ>
    class MethodReplyWithResponseCodeIDAndNameHandler : public MethodReplyWithResponseCodeIDAndNameHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFResponseCode&, LSFString&, LSFString&);

      public:
        MethodReplyWithResponseCodeIDAndNameHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodReplyWithResponseCodeIDAndNameHandler() { }

        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName) {
            (object->*(handler))(responseCode, lsfId, lsfName);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodReplyWithResponseCodeIDAndNameHandlerBase*> MethodReplyWithResponseCodeIDAndNameDispatcherMap;
    MethodReplyWithResponseCodeIDAndNameDispatcherMap methodReplyWithResponseCodeIDAndNameHandlers;

    template <typename OBJ>
    void AddMethodReplyWithResponseCodeAndIDHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodReply)(LSFResponseCode &, LSFString &))
    {
        MethodReplyWithResponseCodeAndIDHandlerBase* handler = new MethodReplyWithResponseCodeAndIDHandler<OBJ>(obj, methodReply);
        std::pair<MethodReplyWithResponseCodeAndIDDispatcherMap::iterator, bool> ins = methodReplyWithResponseCodeAndIDHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class MethodReplyWithResponseCodeAndIDHandlerBase {
      public:
        virtual ~MethodReplyWithResponseCodeAndIDHandlerBase() { }
        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId) = 0;
    };

    template <typename OBJ>
    class MethodReplyWithResponseCodeAndIDHandler : public MethodReplyWithResponseCodeAndIDHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFResponseCode&, LSFString&);

      public:
        MethodReplyWithResponseCodeAndIDHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodReplyWithResponseCodeAndIDHandler() { }

        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId) {
            (object->*(handler))(responseCode, lsfId);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodReplyWithResponseCodeAndIDHandlerBase*> MethodReplyWithResponseCodeAndIDDispatcherMap;
    MethodReplyWithResponseCodeAndIDDispatcherMap methodReplyWithResponseCodeAndIDHandlers;

    template <typename OBJ>
    void AddMethodReplyWithUint32ValueHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodReply)(uint32_t &))
    {
        MethodReplyWithUint32ValueHandlerBase* handler = new MethodReplyWithUint32ValueHandler<OBJ>(obj, methodReply);
        std::pair<MethodReplyWithUint32ValueDispatcherMap::iterator, bool> ins = methodReplyWithUint32ValueHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class MethodReplyWithUint32ValueHandlerBase {
      public:
        virtual ~MethodReplyWithUint32ValueHandlerBase() { }
        virtual void Handle(uint32_t& value) = 0;
    };

    template <typename OBJ>
    class MethodReplyWithUint32ValueHandler : public MethodReplyWithUint32ValueHandlerBase {
        typedef void (OBJ::* HandlerFunction)(uint32_t& value);

      public:
        MethodReplyWithUint32ValueHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodReplyWithUint32ValueHandler() { }

        virtual void Handle(uint32_t& value) {
            (object->*(handler))(value);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodReplyWithUint32ValueHandlerBase*> MethodReplyWithUint32ValueDispatcherMap;
    MethodReplyWithUint32ValueDispatcherMap methodReplyWithUint32ValueHandlers;

    template <typename OBJ>
    void AddMethodReplyWithResponseCodeIDLanguageAndNameHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodReply)(LSFResponseCode &, LSFString &, LSFString &, LSFString &))
    {
        MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase* handler = new MethodReplyWithResponseCodeIDLanguageAndNameHandler<OBJ>(obj, methodReply);
        std::pair<MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap::iterator, bool> ins = methodReplyWithResponseCodeIDLanguageAndNameHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
        }
    }

    class MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase {
      public:
        virtual ~MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase() { }
        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name) = 0;
    };

    template <typename OBJ>
    class MethodReplyWithResponseCodeIDLanguageAndNameHandler : public MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase {
        typedef void (OBJ::* HandlerFunction)(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name);

      public:
        MethodReplyWithResponseCodeIDLanguageAndNameHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodReplyWithResponseCodeIDLanguageAndNameHandler() { }

        virtual void Handle(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name) {
            (object->*(handler))(responseCode, lsfId, language, name);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase*> MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap;
    MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap methodReplyWithResponseCodeIDLanguageAndNameHandlers;

    void RemoveMethodHandlers() {
        for (MethodReplyWithUint32ValueDispatcherMap::iterator it = methodReplyWithUint32ValueHandlers.begin(); it != methodReplyWithUint32ValueHandlers.end(); it++) {
            delete it->second;
        }
        methodReplyWithUint32ValueHandlers.clear();

        for (MethodReplyWithResponseCodeAndListOfIDsDispatcherMap::iterator it = methodReplyWithResponseCodeAndListOfIDsHandlers.begin(); it != methodReplyWithResponseCodeAndListOfIDsHandlers.end(); it++) {
            delete it->second;
        }
        methodReplyWithResponseCodeAndListOfIDsHandlers.clear();

        for (MethodReplyWithResponseCodeIDAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDAndNameHandlers.begin(); it != methodReplyWithResponseCodeIDAndNameHandlers.end(); it++) {
            delete it->second;
        }
        methodReplyWithResponseCodeIDAndNameHandlers.clear();

        for (MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDLanguageAndNameHandlers.begin(); it != methodReplyWithResponseCodeIDLanguageAndNameHandlers.end(); it++) {
            delete it->second;
        }
        methodReplyWithResponseCodeIDLanguageAndNameHandlers.clear();

        for (MethodReplyWithResponseCodeAndIDDispatcherMap::iterator it = methodReplyWithResponseCodeAndIDHandlers.begin(); it != methodReplyWithResponseCodeAndIDHandlers.end(); it++) {
            delete it->second;
        }
        methodReplyWithResponseCodeAndIDHandlers.clear();
    }

    volatile sig_atomic_t stopped;

    uint64_t timeStopped;
};

template <typename OBJ>
ControllerClientStatus ControllerClient::MethodCallAsync(
    const char* ifaceName,
    const char* methodName,
    OBJ* receiver,
    void (OBJ::* replyFunc)(ajn::Message & message),
    const ajn::MsgArg* args,
    size_t numArgs)
{
    typedef TypeHandler<OBJ, void> HANDLER;
    HANDLER* handler = new HANDLER(receiver, replyFunc, this);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        handler,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&HANDLER::MessageHandler),
        args,
        numArgs);

    if (status != CONTROLLER_CLIENT_OK) {
        delete handler;
        handler = NULL;
    }

    return status;
}

} // namespace lsf

#endif
