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

#include <LampManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <vector>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_MANAGER"

LampManager::LampManager(ControllerClient& controllerClient, LampManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.lampManagerPtr = this;
}

ControllerClientStatus LampManager::GetAllLampIDs(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerServiceLampInterfaceName,
               "GetAllLampIDs");
}

ControllerClientStatus LampManager::GetLampManufacturer(const LSFString& lampID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampID=%s language=%s", __func__, lampID.c_str(), language.c_str()));
    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerServiceLampInterfaceName,
               "GetLampManufacturer",
               args,
               2);
}

ControllerClientStatus LampManager::GetLampName(const LSFString& lampID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampID=%s", __func__, lampID.c_str()));
    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerServiceLampInterfaceName,
               "GetLampName",
               args,
               2);
}

ControllerClientStatus LampManager::SetLampName(const LSFString& lampID, const LSFString& lampName, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampID=%s lampName=%s language=%s", __func__, lampID.c_str(), lampName.c_str(), language.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", lampName.c_str());
    args[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampInterfaceName,
               "SetLampName",
               args,
               3);
}

ControllerClientStatus LampManager::GetLampDetails(const LSFString& lampID)
{
    QCC_DbgPrintf(("%s: lampID=%s", __func__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampDetails",
               this,
               &LampManager::GetLampDetailsReply,
               &arg,
               1);
}

void LampManager::GetLampDetailsReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LampDetails details(args[2]);

    callback.GetLampDetailsReplyCB(responseCode, lampID, details);
}

ControllerClientStatus LampManager::GetLampParameters(const LSFString& lampID)
{
    QCC_DbgPrintf(("\n%s: lampID=%s\n", __func__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampParameters",
               this,
               &LampManager::GetLampParametersReply,
               &arg,
               1);
}

void LampManager::GetLampParametersReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LampParameters parameters(args[2]);

    callback.GetLampParametersReplyCB(responseCode, lampID, parameters);
}

ControllerClientStatus LampManager::GetLampState(const LSFString& lampID)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampState",
               this,
               &LampManager::GetLampStateReply,
               &arg,
               1);
}

void LampManager::GetLampStateReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LampState state(args[2]);

    callback.GetLampStateReplyCB(responseCode, lampID, state);
}

ControllerClientStatus LampManager::ResetLampState(const LSFString& lampID)
{
    QCC_DbgPrintf(("\n%s: %s\n", __func__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampInterfaceName,
               "ResetLampState",
               &arg,
               1);
}

ControllerClientStatus LampManager::TransitionLampState(
    const LSFString& lampID,
    const LampState& state,
    const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg args[3];
    args[0].Set("s", lampID.c_str());
    state.Get(&args[1]);
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampInterfaceName,
               "TransitionLampState",
               args,
               3);
}

ControllerClientStatus LampManager::PulseLampWithState(
    const LSFString& lampID,
    const LampState& toLampState,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses,
    const LampState& fromLampState)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg args[6];
    args[0].Set("s", lampID.c_str());
    fromLampState.Get(&args[1]);
    toLampState.Get(&args[2]);
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampInterfaceName,
               "PulseLampWithState",
               args,
               6);
}

ControllerClientStatus LampManager::PulseLampWithPreset(
    const LSFString& lampID,
    const LSFString& toPresetID,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses,
    const LSFString& fromPresetID)
{
    QCC_DbgPrintf(("%s", __func__));

    MsgArg args[6];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", fromPresetID.c_str());
    args[2].Set("s", toPresetID.c_str());
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampInterfaceName,
               "PulseLampWithPreset",
               args,
               6);
}

ControllerClientStatus LampManager::TransitionLampStateToPreset(const LSFString& lampID, const LSFString& presetID, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: %s %s\n", __func__, lampID.c_str(), presetID.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", presetID.c_str());
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerServiceLampInterfaceName,
               "TransitionLampStateToPreset",
               args,
               3);
}

ControllerClientStatus LampManager::GetLampFaults(const LSFString& lampID)
{
    QCC_DbgPrintf(("\n%s: %s\n", __func__, lampID.c_str()));

    MsgArg arg("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampFaults",
               this,
               &LampManager::GetLampFaultsReply,
               &arg,
               1);
}

void LampManager::GetLampFaultsReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);

    LampFaultCodeList codes;
    uint32_t* codeList;
    size_t numCodes;
    args[2].Get("au", &numCodes, &codeList);

    for (size_t i = 0; i < numCodes; i++) {
        codes.push_back(codeList[i]);
        QCC_DbgPrintf(("%s: code=%d", __func__, codeList[i]));
    }

    callback.GetLampFaultsReplyCB(responseCode, lampID, codes);
}

ControllerClientStatus LampManager::GetLampServiceVersion(const LSFString& lampID)
{
    QCC_DbgPrintf(("\n%s: %s\n", __func__, lampID.c_str()));

    MsgArg arg("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampServiceVersion",
               this,
               &LampManager::GetLampServiceVersionReply,
               &arg,
               1);
}

void LampManager::GetLampServiceVersionReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    uint32_t lampServiceVersion = args[2].v_uint32;

    callback.GetLampServiceVersionReplyCB(responseCode, lampID, lampServiceVersion);
}

ControllerClientStatus LampManager::ClearLampFault(const LSFString& lampID, const LampFaultCode& faultCode)
{
    QCC_DbgPrintf(("\n%s: lampID=%s faultCode=%d\n", __func__, lampID.c_str(), faultCode));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("u", faultCode);

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "ClearLampFault",
               this,
               &LampManager::ClearLampFaultReply,
               args,
               2);
}

void LampManager::ClearLampFaultReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LampFaultCode code = static_cast<LampFaultCode>(args[2].v_uint32);

    callback.ClearLampFaultReplyCB(responseCode, lampID, code);
}

ControllerClientStatus LampManager::GetLampStateField(const LSFString& lampID, const LSFString& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __func__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampStateField",
               this,
               &LampManager::GetLampStateFieldReply,
               args,
               2);
}

void LampManager::GetLampStateFieldReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 4) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[2].v_string.str);

    MsgArg* varArg;
    args[3].Get("v", &varArg);

    if (0 == strcmp("OnOff", fieldName.c_str())) {
        bool onOff = static_cast<bool>(varArg->v_bool);
        callback.GetLampStateOnOffFieldReplyCB(responseCode, lampID, onOff);
    } else {
        uint32_t value = static_cast<uint32_t>(varArg->v_uint32);

        if (0 == strcmp("Hue", fieldName.c_str())) {
            callback.GetLampStateHueFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("Saturation", fieldName.c_str())) {
            callback.GetLampStateSaturationFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("Brightness", fieldName.c_str())) {
            callback.GetLampStateBrightnessFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("ColorTemp", fieldName.c_str())) {
            callback.GetLampStateColorTempFieldReplyCB(responseCode, lampID, value);
        }
    }
}

ControllerClientStatus LampManager::ResetLampStateField(const LSFString& lampID, const LSFString& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __func__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampInterfaceName,
               "ResetLampStateField",
               args,
               2);
}

void LampManager::ResetLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __func__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.ResetLampStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.ResetLampStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.ResetLampStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.ResetLampStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.ResetLampStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

ControllerClientStatus LampManager::TransitionLampStateIntegerField(
    const LSFString& lampID,
    const LSFString& stateFieldName,
    const uint32_t& value,
    const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s value=%d transitionPeriod=%d\n", __func__,
                   lampID.c_str(), stateFieldName.c_str(), value, transitionPeriod));

    MsgArg args[4];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("u", value);
    args[2].Set("v", &arg);
    args[3].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampInterfaceName,
               "TransitionLampStateField",
               args,
               4);
}

ControllerClientStatus LampManager::TransitionLampStateBooleanField(
    const LSFString& lampID,
    const LSFString& stateFieldName,
    const bool& value)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __func__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[4];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("b", value);
    args[2].Set("v", &arg);
    args[3].Set("u", 0);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerServiceLampInterfaceName,
               "TransitionLampStateField",
               args,
               4);
}

void LampManager::TransitionLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __func__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.TransitionLampStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.TransitionLampStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.TransitionLampStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.TransitionLampStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.TransitionLampStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

ControllerClientStatus LampManager::GetLampParametersField(const LSFString& lampID, const LSFString& fieldName)
{
    QCC_DbgPrintf(("\n%s\n", __func__));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", fieldName.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampParametersField",
               this,
               &LampManager::GetLampParametersFieldReply,
               args,
               2);
}

void LampManager::GetLampParametersFieldReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 4) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[2].v_string.str);

    MsgArg* varArg;
    args[3].Get("v", &varArg);
    uint32_t value = static_cast<uint32_t>(varArg->v_uint32);

    if (0 == strcmp("Energy_Usage_Milliwatts", fieldName.c_str())) {
        callback.GetLampParametersEnergyUsageMilliwattsFieldReplyCB(responseCode, lampID, value);
    } else if (0 == strcmp("Brightness_Lumens", fieldName.c_str())) {
        callback.GetLampParametersLumensFieldReplyCB(responseCode, lampID, value);
    }
}

ControllerClientStatus LampManager::GetLampSupportedLanguages(const LSFString& lampID)
{
    QCC_DbgPrintf(("\n%s: lampID=%s\n", __func__, lampID.c_str()));

    MsgArg arg("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerServiceLampInterfaceName,
               "GetLampSupportedLanguages",
               this,
               &LampManager::GetLampSupportedLanguagesReply,
               &arg,
               1);
}

void LampManager::GetLampSupportedLanguagesReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controllerClient.CheckNumArgsInMessage(numArgs, 3) != LSF_OK) {
        return;
    }

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampID = static_cast<LSFString>(args[1].v_string.str);

    LSFStringList languages;
    MsgArg* languageList;
    size_t numLanguages;
    args[2].Get("as", &numLanguages, &languageList);

    for (size_t i = 0; i < numLanguages; i++) {
        char* lang;
        languageList[i].Get("s", &lang);
        languages.push_back(lang);
        QCC_DbgPrintf(("%s: language=%d", __func__, lang));
    }

    callback.GetLampSupportedLanguagesReplyCB(responseCode, lampID, languages);
}

ControllerClientStatus LampManager::GetLampDataSet(const LSFString& lampID, const LSFString& language)
{
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;

    status = GetLampDetails(lampID);

    if (CONTROLLER_CLIENT_OK == status) {
        status = GetLampName(lampID, language);
    }

    if (CONTROLLER_CLIENT_OK == status) {
        status = GetLampState(lampID);
    }

    return status;
}
