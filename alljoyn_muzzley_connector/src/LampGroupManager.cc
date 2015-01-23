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

#include <LampGroupManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <utility>

#define QCC_MODULE "LAMP_GROUP_MANAGER"

using namespace lsf;
using namespace ajn;

LampGroupManager::LampGroupManager(ControllerClient& controllerClient, LampGroupManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.lampGroupManagerPtr = this;
}

ControllerClientStatus LampGroupManager::GetAllLampGroupIDs(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerServiceLampGroupInterfaceName,
               "GetAllLampGroupIDs");
}

ControllerClientStatus LampGroupManager::GetLampGroupName(const LSFString& lampGroupID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __func__, lampGroupID.c_str()));
    MsgArg args[2];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerServiceLampGroupInterfaceName,
               "GetLampGroupName",
               args,
               2);
}

ControllerClientStatus LampGroupManager::SetLampGroupName(const LSFString& lampGroupID, const LSFString& lampGroupName, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s lampGroupName=%s language=%s", __func__, lampGroupID.c_str(), lampGroupName.c_str(), language.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", lampGroupName.c_str());
    args[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampGroupInterfaceName,
               "SetLampGroupName",
               args,
               3);
}

ControllerClientStatus LampGroupManager::CreateLampGroup(const LampGroup& lampGroup, const LSFString& lampGroupName, const LSFString& language)
{
    QCC_DbgPrintf(("%s", __func__));
    MsgArg args[4];
    lampGroup.Get(&args[0], &args[1]);
    args[2].Set("s", lampGroupName.c_str());
    args[3].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "CreateLampGroup",
               args,
               4);
}

ControllerClientStatus LampGroupManager::UpdateLampGroup(const LSFString& lampGroupID, const LampGroup& lampGroup)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __func__, lampGroupID.c_str()));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    lampGroup.Get(&args[1], &args[2]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "UpdateLampGroup",
               args,
               3);
}

ControllerClientStatus LampGroupManager::GetLampGroup(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __func__, lampGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampGroupInterfaceName,
               "GetLampGroup",
               this,
               &LampGroupManager::GetLampGroupReply,
               &arg,
               1);
}

void LampGroupManager::GetLampGroupReply(Message& message)
{
    QCC_DbgPrintf(("%s", __func__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 4) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampGroupID = static_cast<LSFString>(args[1].v_string.str);
    LampGroup lampGroup(args[2], args[3]);

    callback.GetLampGroupReplyCB(responseCode, lampGroupID, lampGroup);
}

ControllerClientStatus LampGroupManager::DeleteLampGroup(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __func__, lampGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "DeleteLampGroup",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::ResetLampGroupState(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __func__, lampGroupID.c_str()));
    MsgArg arg("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "ResetLampGroupState",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::ResetLampGroupStateField(const LSFString& lampGroupID, const LSFString& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s\n", __func__, lampGroupID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampGroupInterfaceName,
               "ResetLampGroupStateField",
               args,
               2);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupState(const LSFString& lampGroupID, const LampState& state, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s state=%s transitionPeriod=%d", __func__, lampGroupID.c_str(), state.c_str(), transitionPeriod));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    state.Get(&args[1]);
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "TransitionLampGroupState",
               args,
               3);
}

ControllerClientStatus LampGroupManager::PulseLampGroupWithState(
    const LSFString& lampGroupID,
    const LampState& toLampGroupState,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses,
    const LampState& fromLampGroupState)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    fromLampGroupState.Get(&args[1]);
    toLampGroupState.Get(&args[2]);
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "PulseLampGroupWithState",
               args,
               6);
}

ControllerClientStatus LampGroupManager::PulseLampGroupWithPreset(
    const LSFString& lampGroupID,
    const LSFString& toPresetID,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses,
    const LSFString& fromPresetID)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", fromPresetID.c_str());
    args[2].Set("s", toPresetID.c_str());
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "PulseLampGroupWithPreset",
               args,
               6);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateIntegerField(const LSFString& lampGroupID, const LSFString& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s value=%d transitionPeriod=%d\n", __func__,
                   lampGroupID.c_str(), stateFieldName.c_str(), value, transitionPeriod));

    MsgArg args[4];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("u", value);
    args[2].Set("v", &arg);
    args[3].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampGroupInterfaceName,
               "TransitionLampGroupStateField",
               args,
               4);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateBooleanField(const LSFString& lampGroupID, const LSFString& stateFieldName, const bool& value)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s\n", __func__, lampGroupID.c_str(), stateFieldName.c_str()));

    MsgArg args[4];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("b", value);
    args[2].Set("v", &arg);
    args[3].Set("u", 0);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampGroupInterfaceName,
               "TransitionLampGroupStateField",
               args,
               4);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateToPreset(const LSFString& lampGroupID, const LSFString& presetID, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: %s %s\n", __func__, lampGroupID.c_str(), presetID.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", presetID.c_str());
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampGroupInterfaceName,
               "TransitionLampGroupStateToPreset",
               args,
               3);
}

void LampGroupManager::ResetLampGroupStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __func__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.ResetLampGroupStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.ResetLampGroupStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.ResetLampGroupStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.ResetLampGroupStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.ResetLampGroupStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

void LampGroupManager::TransitionLampGroupStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __func__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.TransitionLampGroupStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.TransitionLampGroupStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.TransitionLampGroupStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.TransitionLampGroupStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.TransitionLampGroupStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

ControllerClientStatus LampGroupManager::GetLampGroupDataSet(const LSFString& lampGroupID, const LSFString& language)
{
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;

    status = GetLampGroup(lampGroupID);

    if (CONTROLLER_CLIENT_OK == status) {
        status = GetLampGroupName(lampGroupID, language);
    }

    return status;
}

void LampGroupManager::LampGroupsNameChanged(LSFStringList& idList) {
    QCC_DbgTrace(("%s", __func__));
    callback.LampGroupsNameChangedCB(idList);
}

void LampGroupManager::LampGroupsCreated(LSFStringList& idList) {
    QCC_DbgTrace(("%s", __func__));
    callback.LampGroupsCreatedCB(idList);
}

void LampGroupManager::LampGroupsDeleted(LSFStringList& idList) {
    QCC_DbgTrace(("%s", __func__));
    callback.LampGroupsDeletedCB(idList);
}

void LampGroupManager::LampGroupsUpdated(LSFStringList& idList) {
    QCC_DbgTrace(("%s", __func__));
    callback.LampGroupsUpdatedCB(idList);
}

void LampGroupManager::GetAllLampGroupIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
    QCC_DbgTrace(("%s", __func__));
    callback.GetAllLampGroupIDsReplyCB(responseCode, idList);
}

void LampGroupManager::GetLampGroupNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& lsfName) {
    QCC_DbgTrace(("%s", __func__));
    callback.GetLampGroupNameReplyCB(responseCode, lsfId, language, lsfName);
}

void LampGroupManager::SetLampGroupNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
    QCC_DbgTrace(("%s", __func__));
    callback.SetLampGroupNameReplyCB(responseCode, lsfId, language);
}

void LampGroupManager::CreateLampGroupReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.CreateLampGroupReplyCB(responseCode, lsfId);
}

void LampGroupManager::UpdateLampGroupReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.UpdateLampGroupReplyCB(responseCode, lsfId);
}

void LampGroupManager::DeleteLampGroupReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.DeleteLampGroupReplyCB(responseCode, lsfId);
}

void LampGroupManager::ResetLampGroupStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.ResetLampGroupStateReplyCB(responseCode, lsfId);
}

void LampGroupManager::TransitionLampGroupStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.TransitionLampGroupStateReplyCB(responseCode, lsfId);
}

void LampGroupManager::PulseLampGroupWithStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.PulseLampGroupWithStateReplyCB(responseCode, lsfId);
}

void LampGroupManager::PulseLampGroupWithPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.PulseLampGroupWithPresetReplyCB(responseCode, lsfId);
}

void LampGroupManager::TransitionLampGroupStateToPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
    QCC_DbgTrace(("%s", __func__));
    callback.TransitionLampGroupStateToPresetReplyCB(responseCode, lsfId);
}
