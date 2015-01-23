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

#include <SceneManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <utility>

#define QCC_MODULE "LAMP_GROUP_MANAGER"

using namespace lsf;
using namespace ajn;

SceneManager::SceneManager(ControllerClient& controllerClient, SceneManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.sceneManagerPtr = this;
}

ControllerClientStatus SceneManager::GetAllSceneIDs(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerServiceSceneInterfaceName,
               "GetAllSceneIDs");
}

ControllerClientStatus SceneManager::GetSceneName(const LSFString& sceneID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __func__, sceneID.c_str()));
    MsgArg args[2];
    args[0].Set("s", sceneID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerServiceSceneInterfaceName,
               "GetSceneName",
               args,
               2);
}

ControllerClientStatus SceneManager::SetSceneName(const LSFString& sceneID, const LSFString& sceneName, const LSFString& language)
{
    QCC_DbgPrintf(("%s: sceneID=%s sceneName=%s language=%s", __func__, sceneID.c_str(), sceneName.c_str(), language.c_str()));

    MsgArg args[3];
    args[0].Set("s", sceneID.c_str());
    args[1].Set("s", sceneName.c_str());
    args[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceSceneInterfaceName,
               "SetSceneName",
               args,
               3);
}

ControllerClientStatus SceneManager::CreateScene(const Scene& scene, const LSFString& sceneName, const LSFString& language)
{
    QCC_DbgPrintf(("%s", __func__));
    MsgArg args[6];
    scene.Get(&args[0], &args[1], &args[2], &args[3]);
    args[4].Set("s", sceneName.c_str());
    args[5].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceSceneInterfaceName,
               "CreateScene",
               args,
               6);
}

ControllerClientStatus SceneManager::UpdateScene(const LSFString& sceneID, const Scene& scene)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __func__, sceneID.c_str()));
    MsgArg args[5];
    args[0].Set("s", sceneID.c_str());
    scene.Get(&args[1], &args[2], &args[3], &args[4]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceSceneInterfaceName,
               "UpdateScene",
               args,
               5);
}

ControllerClientStatus SceneManager::GetScene(const LSFString& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __func__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceSceneInterfaceName,
               "GetScene",
               this,
               &SceneManager::GetSceneReply,
               &arg,
               1);
}

ControllerClientStatus SceneManager::ApplyScene(const LSFString& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __func__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceSceneInterfaceName,
               "ApplyScene",
               &arg,
               1);
}

void SceneManager::GetSceneReply(Message& message)
{
    QCC_DbgPrintf(("%s", __func__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 6) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString sceneID = static_cast<LSFString>(args[1].v_string.str);
    Scene scene(args[2], args[3], args[4], args[5]);

    callback.GetSceneReplyCB(responseCode, sceneID, scene);
}

ControllerClientStatus SceneManager::DeleteScene(const LSFString& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __func__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceSceneInterfaceName,
               "DeleteScene",
               &arg,
               1);
}

ControllerClientStatus SceneManager::GetSceneDataSet(const LSFString& sceneID, const LSFString& language)
{
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;

    status = GetScene(sceneID);

    if (CONTROLLER_CLIENT_OK == status) {
        status = GetSceneName(sceneID, language);
    }

    return status;
}
