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

#include <MasterSceneManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <utility>

#define QCC_MODULE "LAMP_GROUP_MANAGER"

using namespace lsf;
using namespace ajn;

MasterSceneManager::MasterSceneManager(ControllerClient& controllerClient, MasterSceneManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    QCC_DbgTrace(("%s", __func__));
    controllerClient.masterSceneManagerPtr = this;
}

ControllerClientStatus MasterSceneManager::GetAllMasterSceneIDs(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerServiceMasterSceneInterfaceName,
               "GetAllMasterSceneIDs");
}

ControllerClientStatus MasterSceneManager::GetMasterSceneName(const LSFString& masterSceneID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s", __func__, masterSceneID.c_str()));
    MsgArg args[2];
    args[0].Set("s", masterSceneID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerServiceMasterSceneInterfaceName,
               "GetMasterSceneName",
               args,
               2);
}

ControllerClientStatus MasterSceneManager::SetMasterSceneName(const LSFString& masterSceneID, const LSFString& masterSceneName, const LSFString& language)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s masterSceneName=%s language=%s", __func__, masterSceneID.c_str(), masterSceneName.c_str(), language.c_str()));

    MsgArg args[3];
    args[0].Set("s", masterSceneID.c_str());
    args[1].Set("s", masterSceneName.c_str());
    args[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceMasterSceneInterfaceName,
               "SetMasterSceneName",
               args,
               3);
}

ControllerClientStatus MasterSceneManager::CreateMasterScene(const MasterScene& masterScene, const LSFString& masterSceneName, const LSFString& language)
{
    QCC_DbgPrintf(("%s", __func__));
    MsgArg arg[3];
    masterScene.Get(&arg[0]);
    arg[1].Set("s", masterSceneName.c_str());
    arg[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceMasterSceneInterfaceName,
               "CreateMasterScene",
               arg,
               3);
}

ControllerClientStatus MasterSceneManager::UpdateMasterScene(const LSFString& masterSceneID, const MasterScene& masterScene)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s", __func__, masterSceneID.c_str()));
    MsgArg args[2];
    args[0].Set("s", masterSceneID.c_str());
    masterScene.Get(&args[1]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceMasterSceneInterfaceName,
               "UpdateMasterScene",
               args,
               2);
}

ControllerClientStatus MasterSceneManager::GetMasterScene(const LSFString& masterSceneID)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s", __func__, masterSceneID.c_str()));
    MsgArg arg;
    arg.Set("s", masterSceneID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceMasterSceneInterfaceName,
               "GetMasterScene",
               this,
               &MasterSceneManager::GetMasterSceneReply,
               &arg,
               1);
}

void MasterSceneManager::GetMasterSceneReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString masterSceneID = static_cast<LSFString>(args[1].v_string.str);
    MasterScene masterScene(args[2]);

    callback.GetMasterSceneReplyCB(responseCode, masterSceneID, masterScene);
}

ControllerClientStatus MasterSceneManager::DeleteMasterScene(const LSFString& masterSceneID)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s", __func__, masterSceneID.c_str()));
    MsgArg arg;
    arg.Set("s", masterSceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceMasterSceneInterfaceName,
               "DeleteMasterScene",
               &arg,
               1);
}

ControllerClientStatus MasterSceneManager::ApplyMasterScene(const LSFString& masterSceneID)
{
    QCC_DbgPrintf(("%s: masterSceneID=%s", __func__, masterSceneID.c_str()));
    MsgArg arg;
    arg.Set("s", masterSceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceMasterSceneInterfaceName,
               "ApplyMasterScene",
               &arg,
               1);
}

ControllerClientStatus MasterSceneManager::GetMasterSceneDataSet(const LSFString& masterSceneID, const LSFString& language)
{
    QCC_DbgTrace(("%s", __func__));
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;

    status = GetMasterScene(masterSceneID);

    if (CONTROLLER_CLIENT_OK == status) {
        status = GetMasterSceneName(masterSceneID, language);
    }

    return status;
}
