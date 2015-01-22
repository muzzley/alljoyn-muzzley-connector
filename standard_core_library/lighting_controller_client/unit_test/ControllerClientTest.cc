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
#include <ControllerClient.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>
#include <ControllerServiceManager.h>

#include <qcc/StringUtil.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Debug.h>

/* Header files included for Google Test Framework */
#include <gtest/gtest.h>

using namespace std;
using namespace qcc;
using namespace ajn;
using namespace lsf;

static bool replyReceivedFlag = false;
static bool signalReceivedFlag = false;
static bool sceneSignalReceivedFlag = false;

static bool getLampDataSetName = false;
static bool getLampDataSetDetails = false;
static bool getLampDataSetState = false;

static bool getLampGroupDataSetGroup = false;
static bool getLampGroupDataSetName = false;

static bool getPresetDataSetPreset = false;
static bool getPresetDataSetName = false;

static bool getSceneDataSetScene = false;
static bool getSceneDataSetName = false;

static bool getMasterSceneDataSetMasterScene = false;
static bool getMasterSceneDataSetName = false;

LSFString nestedLampGroupIDForDependencyCheck;
LSFString lampGroupIDForDependencyCheck;
LSFString presetIDForDependencyCheck;
LSFString sceneIDForDependencyCheck;

class ControllerClientCallbackHandler : public ControllerClientCallback {
  public:

    ControllerClientCallbackHandler() : connectedToControllerServiceCBStatus(LSF_ERR_UNEXPECTED), controllerId(), controllerName(), error(false)  { }

    ~ControllerClientCallbackHandler() { }

    void ConnectedToControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        if (controllerId.empty()) {
            connectedToControllerServiceCBStatus = LSF_OK;
            controllerId = controllerServiceDeviceID;
            controllerName = controllerServiceName;
            replyReceivedFlag = true;
        }
    }

    void ConnectToControllerServiceFailedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        if (controllerId.empty()) {
            connectedToControllerServiceCBStatus = LSF_ERR_FAILURE;
            replyReceivedFlag = true;
        }
    }

    void DisconnectedFromControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        if ((!controllerId.empty()) && (controllerServiceDeviceID == controllerId)) {
            connectedToControllerServiceCBStatus = LSF_ERR_FAILURE;
            controllerId.clear();
            controllerName.clear();
            replyReceivedFlag = true;
        }
    }

    void ControllerClientErrorCB(const ErrorCodeList& errorCodeList) {
        error = true;
        errorList = errorCodeList;
        replyReceivedFlag = true;
    }

    LSFResponseCode connectedToControllerServiceCBStatus;
    LSFString controllerId;
    LSFString controllerName;
    bool error;
    ErrorCodeList errorList;
};

class ControllerServiceManagerCallbackHandler : public ControllerServiceManagerCallback {
  public:
    ControllerServiceManagerCallbackHandler() :
        lightingResetControllerServiceReplyCBStatus(LSF_ERR_UNEXPECTED),
        controllerServiceLightingResetCBStatus(LSF_ERR_UNEXPECTED),
        controllerServiceNameChangedCBStatus(LSF_ERR_UNEXPECTED),
        controllerServiceVersion(0), controllerId(), controllerName() { }

    void GetControllerServiceVersionReplyCB(const uint32_t& version) {
        controllerServiceVersion = version;
        replyReceivedFlag = true;
    }

    void LightingResetControllerServiceReplyCB(const LSFResponseCode& responseCode) {
        lightingResetControllerServiceReplyCBStatus = responseCode;
        replyReceivedFlag = true;
    }

    void ControllerServiceLightingResetCB(void) {
        controllerServiceLightingResetCBStatus = LSF_OK;
        signalReceivedFlag = true;
    }

    void ControllerServiceNameChangedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        controllerServiceNameChangedCBStatus = LSF_OK;
        controllerId = controllerServiceDeviceID;
        controllerName = controllerServiceName;
        replyReceivedFlag = true;
    }

    LSFResponseCode lightingResetControllerServiceReplyCBStatus;
    LSFResponseCode controllerServiceLightingResetCBStatus;
    LSFResponseCode controllerServiceNameChangedCBStatus;
    uint32_t controllerServiceVersion;
    LSFString controllerId;
    LSFString controllerName;
};

class LampManagerCallbackHandler : public LampManagerCallback {
  public:

    LampManagerCallbackHandler() :
        getAllLampIDsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampManufacturerReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampManufacturerReplyCBLampID(),
        getLampManufacturerReplyCBLanguage(),
        getLampManufacturerReplyCBManufacturer(),
        getLampSupportedLanguagesReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampSupportedLanguagesReplyCBLampID(),
        getLampNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampNameReplyCBLampID(),
        getLampNameReplyCBLanguage(),
        getLampNameReplyCBLampName(),
        setLampNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        setLampNameReplyCBLampID(),
        setLampNameReplyCBLanguage(),
        lampNameChangedCBLampID(),
        lampNameChangedCBLampName(),
        getLampStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateReplyCBLampID(),
        getLampStateReplyCBLampState(),
        getLampStateOnOffFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateOnOffFieldReplyCBLampID(),
        getLampStateOnOffFieldReplyCBOnOff(),
        getLampStateHueFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateHueFieldReplyCBLampID(),
        getLampStateHueFieldReplyCBHue(),
        getLampStateSaturationFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateSaturationFieldReplyCBLampID(),
        getLampStateSaturationFieldReplyCBSaturation(),
        getLampStateBrightnessFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateBrightnessFieldReplyCBLampID(),
        getLampStateBrightnessFieldReplyCBBrightness(),
        getLampStateColorTempFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampStateColorTempFieldReplyCBLampID(),
        getLampStateColorTempFieldReplyCBColorTemp(),
        resetLampStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateReplyCBLampID(),
        resetLampStateOnOffFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateOnOffFieldReplyCBLampID(),
        resetLampStateHueFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateHueFieldReplyCBLampID(),
        resetLampStateSaturationFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateSaturationFieldReplyCBLampID(),
        resetLampStateBrightnessFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateBrightnessFieldReplyCBLampID(),
        resetLampStateColorTempFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampStateColorTempFieldReplyCBLampID(),
        lampStateChangedCBLampID(),
        lampStateChangedCBLampState(),
        transitionLampStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateReplyCBLampID(),
        transitionLampStateOnOffFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateOnOffFieldReplyCBLampID(),
        transitionLampStateHueFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateHueFieldReplyCBLampID(),
        transitionLampStateSaturationFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateSaturationFieldReplyCBLampID(),
        transitionLampStateBrightnessFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateBrightnessFieldReplyCBLampID(),
        transitionLampStateColorTempFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateColorTempFieldReplyCBLampID(),
        pulseLampWithStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        pulseLampWithStateReplyCBLampID(),
        pulseLampWithPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        pulseLampWithPresetReplyCBLampID(),
        transitionLampStateToPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampStateToPresetReplyCBLampID(),
        getLampFaultsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampFaultsReplyCBLampID(),
        faultsList(),
        clearLampFaultReplyCBStatus(LSF_ERR_UNEXPECTED),
        clearLampFaultReplyCBLampID(),
        lampFaultCode(0),
        getLampServiceVersionReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampServiceVersionReplyCBLampID(),
        lampVersion(0),
        getLampDetailsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampDetailsReplyCBLampID(),
        getLampDetailsReplyCBLampDetails(),
        getLampParametersReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampParametersReplyCBLampID(),
        getLampParametersReplyCBLampParameters(),
        getLampParametersEnergyUsageMilliwattsFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampParametersEnergyUsageMilliwattsFieldReplyCBLampID(),
        getLampParametersEnergyUsageMilliwattsFieldReplyCBLampParametersEnergyUsageMilliwattsField(),
        getLampParametersLumensFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampParametersLumensFieldReplyCBLampID(),
        getLampParametersLumensFieldReplyCBLampParametersLumensField() { }

    void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampIDs) {
        getAllLampIDsReplyCBStatus = responseCode;
        lampList = lampIDs;
        replyReceivedFlag = true;
    }

    void GetLampSupportedLanguagesReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFStringList& supportedLanguages) {
        getLampSupportedLanguagesReplyCBStatus = responseCode;
        getLampSupportedLanguagesReplyCBLampID = lampID;
        getLampSupportedLanguagesReplyCBSupportedLanguages = supportedLanguages;
        replyReceivedFlag = true;
    }

    void GetLampManufacturerReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& manufacturer) {
        getLampManufacturerReplyCBStatus = responseCode;
        getLampManufacturerReplyCBLampID = lampID;
        getLampManufacturerReplyCBLanguage = language;
        getLampManufacturerReplyCBManufacturer = manufacturer;
        replyReceivedFlag = true;
    }

    void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& lampName) {
        getLampNameReplyCBStatus = responseCode;
        getLampNameReplyCBLampID = lampID;
        getLampNameReplyCBLanguage = language;
        getLampNameReplyCBLampName = lampName;
        replyReceivedFlag = true;
        getLampDataSetName = true;
    }

    void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language) {
        setLampNameReplyCBStatus = responseCode;
        setLampNameReplyCBLampID = lampID;
        setLampNameReplyCBLanguage = language;
        replyReceivedFlag = true;
    }

    void LampNameChangedCB(const LSFString& lampID, const LSFString& lampName) {
        lampNameChangedCBLampID = lampID;
        lampNameChangedCBLampName = lampName;
        signalReceivedFlag = true;
    }

    void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampDetails& lampDetails) {
        getLampDetailsReplyCBStatus = responseCode;
        getLampDetailsReplyCBLampID = lampID;
        getLampDetailsReplyCBLampDetails = lampDetails;
        replyReceivedFlag = true;
        getLampDataSetDetails = true;
    }

    void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampParameters& lampParameters) {
        getLampParametersReplyCBStatus = responseCode;
        getLampParametersReplyCBLampID = lampID;
        getLampParametersReplyCBLampParameters = lampParameters;
        replyReceivedFlag = true;
    }

    void GetLampParametersLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& value) {
        getLampParametersLumensFieldReplyCBStatus = responseCode;
        getLampParametersLumensFieldReplyCBLampID = lampID;
        getLampParametersLumensFieldReplyCBLampParametersLumensField = value;
        replyReceivedFlag = true;
    }

    void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& value) {
        getLampParametersEnergyUsageMilliwattsFieldReplyCBStatus = responseCode;
        getLampParametersEnergyUsageMilliwattsFieldReplyCBLampID = lampID;
        getLampParametersEnergyUsageMilliwattsFieldReplyCBLampParametersEnergyUsageMilliwattsField = value;
        replyReceivedFlag = true;
    }

    void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampState& lampState) {
        getLampStateReplyCBStatus = responseCode;
        getLampStateReplyCBLampID = lampID;
        getLampStateReplyCBLampState = lampState;
        replyReceivedFlag = true;
        getLampDataSetState = true;
    }

    void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const bool& onOff) {
        getLampStateOnOffFieldReplyCBStatus = responseCode;
        getLampStateOnOffFieldReplyCBLampID = lampID;
        getLampStateOnOffFieldReplyCBOnOff = onOff;
        replyReceivedFlag = true;
    }

    void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& hue) {
        getLampStateHueFieldReplyCBStatus = responseCode;
        getLampStateHueFieldReplyCBLampID = lampID;
        getLampStateHueFieldReplyCBHue = hue;
        replyReceivedFlag = true;
    }

    void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& saturation) {
        getLampStateSaturationFieldReplyCBStatus = responseCode;
        getLampStateSaturationFieldReplyCBLampID = lampID;
        getLampStateSaturationFieldReplyCBSaturation = saturation;
        replyReceivedFlag = true;
    }

    void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightness) {
        getLampStateBrightnessFieldReplyCBStatus = responseCode;
        getLampStateBrightnessFieldReplyCBLampID = lampID;
        getLampStateBrightnessFieldReplyCBBrightness = brightness;
        replyReceivedFlag = true;
    }

    void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& colorTemp) {
        getLampStateColorTempFieldReplyCBStatus = responseCode;
        getLampStateColorTempFieldReplyCBLampID = lampID;
        getLampStateColorTempFieldReplyCBColorTemp = colorTemp;
        replyReceivedFlag = true;
    }

    void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateReplyCBStatus = responseCode;
        resetLampStateReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void LampStateChangedCB(const LSFString& lampID, const LampState& lampState) {
        lampStateChangedCBLampID = lampID;
        lampStateChangedCBLampState = lampState;
        signalReceivedFlag = true;
    }

    void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateReplyCBStatus = responseCode;
        transitionLampStateReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void PulseLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        pulseLampWithStateReplyCBStatus = responseCode;
        pulseLampWithStateReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void PulseLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        pulseLampWithPresetReplyCBStatus = responseCode;
        pulseLampWithPresetReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateOnOffFieldReplyCBStatus = responseCode;
        transitionLampStateOnOffFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateHueFieldReplyCBStatus = responseCode;
        transitionLampStateHueFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateSaturationFieldReplyCBStatus = responseCode;
        transitionLampStateSaturationFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateBrightnessFieldReplyCBStatus = responseCode;
        transitionLampStateBrightnessFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateColorTempFieldReplyCBStatus = responseCode;
        transitionLampStateColorTempFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCodeList& faultCodes) {
        getLampFaultsReplyCBStatus = responseCode;
        getLampFaultsReplyCBLampID = lampID;
        faultsList = faultCodes;
        replyReceivedFlag = true;
    }

    void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCode& faultCode) {
        clearLampFaultReplyCBStatus = responseCode;
        clearLampFaultReplyCBLampID = lampID;
        lampFaultCode = faultCode;
        replyReceivedFlag = true;
    }

    void GetLampServiceVersionReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& lampServiceVersion) {
        getLampServiceVersionReplyCBStatus = responseCode;
        getLampServiceVersionReplyCBLampID = lampID;
        lampVersion = lampServiceVersion;
        replyReceivedFlag = true;
    }

    void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateOnOffFieldReplyCBStatus = responseCode;
        resetLampStateOnOffFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateHueFieldReplyCBStatus = responseCode;
        resetLampStateHueFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateSaturationFieldReplyCBStatus = responseCode;
        resetLampStateSaturationFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateBrightnessFieldReplyCBStatus = responseCode;
        resetLampStateBrightnessFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampStateColorTempFieldReplyCBStatus = responseCode;
        resetLampStateColorTempFieldReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampStateToPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampStateToPresetReplyCBStatus = responseCode;
        transitionLampStateToPresetReplyCBLampID = lampID;
        replyReceivedFlag = true;
    }

    void LampsFoundCB(const LSFStringList& lampIDs) {
        signalReceivedFlag = true;
    }

    void LampsLostCB(const LSFStringList& lampIDs) {
        signalReceivedFlag = true;
    }

    LSFResponseCode getAllLampIDsReplyCBStatus;
    LSFResponseCode getLampManufacturerReplyCBStatus;
    LSFString getLampManufacturerReplyCBLampID;
    LSFString getLampManufacturerReplyCBLanguage;
    LSFString getLampManufacturerReplyCBManufacturer;
    LSFResponseCode getLampSupportedLanguagesReplyCBStatus;
    LSFString getLampSupportedLanguagesReplyCBLampID;
    LSFStringList getLampSupportedLanguagesReplyCBSupportedLanguages;
    LSFResponseCode getLampNameReplyCBStatus;
    LSFString getLampNameReplyCBLampID;
    LSFString getLampNameReplyCBLanguage;
    LSFString getLampNameReplyCBLampName;
    LSFResponseCode setLampNameReplyCBStatus;
    LSFString setLampNameReplyCBLampID;
    LSFString setLampNameReplyCBLanguage;
    LSFString lampNameChangedCBLampID;
    LSFString lampNameChangedCBLampName;
    LSFResponseCode getLampStateReplyCBStatus;
    LSFString getLampStateReplyCBLampID;
    LampState getLampStateReplyCBLampState;
    LSFResponseCode getLampStateOnOffFieldReplyCBStatus;
    LSFString getLampStateOnOffFieldReplyCBLampID;
    bool getLampStateOnOffFieldReplyCBOnOff;
    LSFResponseCode getLampStateHueFieldReplyCBStatus;
    LSFString getLampStateHueFieldReplyCBLampID;
    bool getLampStateHueFieldReplyCBHue;
    LSFResponseCode getLampStateSaturationFieldReplyCBStatus;
    LSFString getLampStateSaturationFieldReplyCBLampID;
    bool getLampStateSaturationFieldReplyCBSaturation;
    LSFResponseCode getLampStateBrightnessFieldReplyCBStatus;
    LSFString getLampStateBrightnessFieldReplyCBLampID;
    bool getLampStateBrightnessFieldReplyCBBrightness;
    LSFResponseCode getLampStateColorTempFieldReplyCBStatus;
    LSFString getLampStateColorTempFieldReplyCBLampID;
    bool getLampStateColorTempFieldReplyCBColorTemp;
    LSFResponseCode resetLampStateReplyCBStatus;
    LSFString resetLampStateReplyCBLampID;
    LSFResponseCode resetLampStateOnOffFieldReplyCBStatus;
    LSFString resetLampStateOnOffFieldReplyCBLampID;
    LSFResponseCode resetLampStateHueFieldReplyCBStatus;
    LSFString resetLampStateHueFieldReplyCBLampID;
    LSFResponseCode resetLampStateSaturationFieldReplyCBStatus;
    LSFString resetLampStateSaturationFieldReplyCBLampID;
    LSFResponseCode resetLampStateBrightnessFieldReplyCBStatus;
    LSFString resetLampStateBrightnessFieldReplyCBLampID;
    LSFResponseCode resetLampStateColorTempFieldReplyCBStatus;
    LSFString resetLampStateColorTempFieldReplyCBLampID;
    LSFString lampStateChangedCBLampID;
    LampState lampStateChangedCBLampState;
    LSFResponseCode transitionLampStateReplyCBStatus;
    LSFString transitionLampStateReplyCBLampID;
    LSFResponseCode transitionLampStateOnOffFieldReplyCBStatus;
    LSFString transitionLampStateOnOffFieldReplyCBLampID;
    LSFResponseCode transitionLampStateHueFieldReplyCBStatus;
    LSFString transitionLampStateHueFieldReplyCBLampID;
    LSFResponseCode transitionLampStateSaturationFieldReplyCBStatus;
    LSFString transitionLampStateSaturationFieldReplyCBLampID;
    LSFResponseCode transitionLampStateBrightnessFieldReplyCBStatus;
    LSFString transitionLampStateBrightnessFieldReplyCBLampID;
    LSFResponseCode transitionLampStateColorTempFieldReplyCBStatus;
    LSFString transitionLampStateColorTempFieldReplyCBLampID;
    LSFResponseCode pulseLampWithStateReplyCBStatus;
    LSFString pulseLampWithStateReplyCBLampID;
    LSFResponseCode pulseLampWithPresetReplyCBStatus;
    LSFString pulseLampWithPresetReplyCBLampID;
    LSFResponseCode transitionLampStateToPresetReplyCBStatus;
    LSFString transitionLampStateToPresetReplyCBLampID;
    LSFResponseCode getLampFaultsReplyCBStatus;
    LSFString getLampFaultsReplyCBLampID;
    LampFaultCodeList faultsList;
    LSFResponseCode clearLampFaultReplyCBStatus;
    LSFString clearLampFaultReplyCBLampID;
    LampFaultCode lampFaultCode;
    LSFResponseCode getLampServiceVersionReplyCBStatus;
    LSFString getLampServiceVersionReplyCBLampID;
    uint32_t lampVersion;
    LSFResponseCode getLampDetailsReplyCBStatus;
    LSFString getLampDetailsReplyCBLampID;
    LampDetails getLampDetailsReplyCBLampDetails;
    LSFResponseCode getLampParametersReplyCBStatus;
    LSFString getLampParametersReplyCBLampID;
    LampParameters getLampParametersReplyCBLampParameters;
    LSFResponseCode getLampParametersEnergyUsageMilliwattsFieldReplyCBStatus;
    LSFString getLampParametersEnergyUsageMilliwattsFieldReplyCBLampID;
    uint32_t getLampParametersEnergyUsageMilliwattsFieldReplyCBLampParametersEnergyUsageMilliwattsField;
    LSFResponseCode getLampParametersLumensFieldReplyCBStatus;
    LSFString getLampParametersLumensFieldReplyCBLampID;
    uint32_t getLampParametersLumensFieldReplyCBLampParametersLumensField;
    LSFStringList lampList;
};

class LampGroupManagerCallbackHandler : public LampGroupManagerCallback {
  public:

    LampGroupManagerCallbackHandler() :
        createLampGroupReplyCBStatus(LSF_ERR_UNEXPECTED),
        createLampGroupReplyCBID(),
        updateLampGroupReplyCBStatus(LSF_ERR_UNEXPECTED),
        updateLampGroupReplyCBID(),
        deleteLampGroupReplyCBStatus(LSF_ERR_UNEXPECTED),
        deleteLampGroupReplyCBID(),
        getAllLampGroupIDsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampGroupNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampGroupNameReplyCBLampGroupID(),
        getLampGroupNameReplyCBLanguage(),
        getLampGroupNameReplyCBLampGroupName(),
        setLampGroupNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        setLampGroupNameReplyCBLampGroupID(),
        setLampGroupNameReplyCBLanguage(),
        getLampGroupReplyCBStatus(LSF_ERR_UNEXPECTED),
        getLampGroupReplyCBLampGroupID(),
        getLampGroupReplyCBLampGroup(),
        lampGroupList(),
        lampGroupNameChangedList(),
        lampGroupUpdatedList(),
        lampGroupDeletedList(),
        resetLampGroupStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateReplyCBLampGroupID(),
        resetLampGroupStateOnOffFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateOnOffFieldReplyCBLampGroupID(),
        resetLampGroupStateHueFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateHueFieldReplyCBLampGroupID(),
        resetLampGroupStateSaturationFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateSaturationFieldReplyCBLampGroupID(),
        resetLampGroupStateBrightnessFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateBrightnessFieldReplyCBLampGroupID(),
        resetLampGroupStateColorTempFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        resetLampGroupStateColorTempFieldReplyCBLampGroupID(),
        transitionLampGroupStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateReplyCBLampGroupID(),
        transitionLampGroupStateOnOffFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateOnOffFieldReplyCBLampGroupID(),
        transitionLampGroupStateHueFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateHueFieldReplyCBLampGroupID(),
        transitionLampGroupStateSaturationFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateSaturationFieldReplyCBLampGroupID(),
        transitionLampGroupStateBrightnessFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateBrightnessFieldReplyCBLampGroupID(),
        transitionLampGroupStateColorTempFieldReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateColorTempFieldReplyCBLampGroupID(),
        pulseLampGroupWithStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        pulseLampGroupWithStateReplyCBLampGroupID(),
        pulseLampGroupWithPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        pulseLampGroupWithPresetReplyCBLampGroupID(),
        transitionLampGroupStateToPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        transitionLampGroupStateToPresetReplyCBLampGroupID() { }

    void GetLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LampGroup& lampGroup) {
        getLampGroupReplyCBStatus = responseCode;
        getLampGroupReplyCBLampGroupID = lampGroupID;
        getLampGroupReplyCBLampGroup = lampGroup;
        replyReceivedFlag = true;
        getLampGroupDataSetGroup = true;
    }

    void GetAllLampGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampGroupIDs) {
        getAllLampGroupIDsReplyCBStatus = responseCode;
        lampGroupList = lampGroupIDs;
        replyReceivedFlag = true;
    }

    void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LSFString& language, const LSFString& lampGroupName) {
        getLampGroupNameReplyCBStatus = responseCode;
        getLampGroupNameReplyCBLampGroupID = lampGroupID;
        getLampGroupNameReplyCBLanguage = language;
        getLampGroupNameReplyCBLampGroupName = lampGroupName;
        replyReceivedFlag = true;
        getLampGroupDataSetName = true;
    }

    void SetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LSFString& language) {
        setLampGroupNameReplyCBStatus = responseCode;
        setLampGroupNameReplyCBLampGroupID = lampGroupID;
        setLampGroupNameReplyCBLanguage = language;
        replyReceivedFlag = true;
    }

    void LampGroupsNameChangedCB(const LSFStringList& lampGroupIDs) {
        lampGroupNameChangedList = lampGroupIDs;
        signalReceivedFlag = true;
    }

    void CreateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        createLampGroupReplyCBStatus = responseCode;
        createLampGroupReplyCBID = lampGroupID;
        replyReceivedFlag = true;
    }

    void LampGroupsCreatedCB(const LSFStringList& lampGroupIDs) {
        lampGroupList = lampGroupIDs;
        signalReceivedFlag = true;
    }

    void UpdateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        updateLampGroupReplyCBStatus = responseCode;
        updateLampGroupReplyCBID = lampGroupID;
        replyReceivedFlag = true;
    }

    void LampGroupsUpdatedCB(const LSFStringList& lampGroupIDs) {
        lampGroupUpdatedList = lampGroupIDs;
        signalReceivedFlag = true;
    }

    void DeleteLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        deleteLampGroupReplyCBStatus = responseCode;
        deleteLampGroupReplyCBID = lampGroupID;
        replyReceivedFlag = true;
    }

    void LampGroupsDeletedCB(const LSFStringList& lampGroupIDs) {
        lampGroupDeletedList = lampGroupIDs;
        signalReceivedFlag = true;
    }

    void TransitionLampGroupStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateReplyCBStatus = responseCode;
        transitionLampGroupStateReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void PulseLampGroupWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        pulseLampGroupWithStateReplyCBStatus = responseCode;
        pulseLampGroupWithStateReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void PulseLampGroupWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        pulseLampGroupWithPresetReplyCBStatus = responseCode;
        pulseLampGroupWithPresetReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateOnOffFieldReplyCBStatus = responseCode;
        transitionLampGroupStateOnOffFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateHueFieldReplyCBStatus = responseCode;
        transitionLampGroupStateHueFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateSaturationFieldReplyCBStatus = responseCode;
        transitionLampGroupStateSaturationFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateBrightnessFieldReplyCBStatus = responseCode;
        transitionLampGroupStateBrightnessFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateColorTempFieldReplyCBStatus = responseCode;
        transitionLampGroupStateColorTempFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void TransitionLampGroupStateToPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        transitionLampGroupStateToPresetReplyCBStatus = responseCode;
        transitionLampGroupStateToPresetReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateReplyCBStatus = responseCode;
        resetLampGroupStateReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateOnOffFieldReplyCBStatus = responseCode;
        resetLampGroupStateOnOffFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateHueFieldReplyCBStatus = responseCode;
        resetLampGroupStateHueFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateSaturationFieldReplyCBStatus = responseCode;
        resetLampGroupStateSaturationFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateBrightnessFieldReplyCBStatus = responseCode;
        resetLampGroupStateBrightnessFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    void ResetLampGroupStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        resetLampGroupStateColorTempFieldReplyCBStatus = responseCode;
        resetLampGroupStateColorTempFieldReplyCBLampGroupID = lampID;
        replyReceivedFlag = true;
    }

    LSFResponseCode createLampGroupReplyCBStatus;
    LSFString createLampGroupReplyCBID;
    LSFResponseCode updateLampGroupReplyCBStatus;
    LSFString updateLampGroupReplyCBID;
    LSFResponseCode deleteLampGroupReplyCBStatus;
    LSFString deleteLampGroupReplyCBID;
    LSFResponseCode getAllLampGroupIDsReplyCBStatus;
    LSFResponseCode getLampGroupNameReplyCBStatus;
    LSFString getLampGroupNameReplyCBLampGroupID;
    LSFString getLampGroupNameReplyCBLanguage;
    LSFString getLampGroupNameReplyCBLampGroupName;
    LSFResponseCode setLampGroupNameReplyCBStatus;
    LSFString setLampGroupNameReplyCBLampGroupID;
    LSFString setLampGroupNameReplyCBLanguage;
    LSFResponseCode getLampGroupReplyCBStatus;
    LSFString getLampGroupReplyCBLampGroupID;
    LampGroup getLampGroupReplyCBLampGroup;
    LSFStringList lampGroupList;
    LSFStringList lampGroupNameChangedList;
    LSFStringList lampGroupUpdatedList;
    LSFStringList lampGroupDeletedList;
    LSFResponseCode resetLampGroupStateReplyCBStatus;
    LSFString resetLampGroupStateReplyCBLampGroupID;
    LSFResponseCode resetLampGroupStateOnOffFieldReplyCBStatus;
    LSFString resetLampGroupStateOnOffFieldReplyCBLampGroupID;
    LSFResponseCode resetLampGroupStateHueFieldReplyCBStatus;
    LSFString resetLampGroupStateHueFieldReplyCBLampGroupID;
    LSFResponseCode resetLampGroupStateSaturationFieldReplyCBStatus;
    LSFString resetLampGroupStateSaturationFieldReplyCBLampGroupID;
    LSFResponseCode resetLampGroupStateBrightnessFieldReplyCBStatus;
    LSFString resetLampGroupStateBrightnessFieldReplyCBLampGroupID;
    LSFResponseCode resetLampGroupStateColorTempFieldReplyCBStatus;
    LSFString resetLampGroupStateColorTempFieldReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateReplyCBStatus;
    LSFString transitionLampGroupStateReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateOnOffFieldReplyCBStatus;
    LSFString transitionLampGroupStateOnOffFieldReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateHueFieldReplyCBStatus;
    LSFString transitionLampGroupStateHueFieldReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateSaturationFieldReplyCBStatus;
    LSFString transitionLampGroupStateSaturationFieldReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateBrightnessFieldReplyCBStatus;
    LSFString transitionLampGroupStateBrightnessFieldReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateColorTempFieldReplyCBStatus;
    LSFString transitionLampGroupStateColorTempFieldReplyCBLampGroupID;
    LSFResponseCode pulseLampGroupWithStateReplyCBStatus;
    LSFString pulseLampGroupWithStateReplyCBLampGroupID;
    LSFResponseCode pulseLampGroupWithPresetReplyCBStatus;
    LSFString pulseLampGroupWithPresetReplyCBLampGroupID;
    LSFResponseCode transitionLampGroupStateToPresetReplyCBStatus;
    LSFString transitionLampGroupStateToPresetReplyCBLampGroupID;
};

class PresetManagerCallbackHandler : public PresetManagerCallback {
  public:

    PresetManagerCallbackHandler() :
        createPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        createPresetReplyCBID(),
        updatePresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        updatePresetReplyCBID(),
        deletePresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        deletePresetReplyCBID(),
        getDefaultLampStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        getDefaultLampStateReplyCBDefaultLampState(),
        setDefaultLampStateReplyCBStatus(LSF_ERR_UNEXPECTED),
        defaultLampStateChangedCBStatus(LSF_ERR_UNEXPECTED),
        getAllPresetIDsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getPresetNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        getPresetNameReplyCBPresetID(),
        getPresetNameReplyCBLanguage(),
        getPresetNameReplyCBPresetName(),
        setPresetNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        setPresetNameReplyCBPresetID(),
        setPresetNameReplyCBLanguage(),
        getPresetReplyCBStatus(LSF_ERR_UNEXPECTED),
        getPresetReplyCBPresetID(),
        getPresetReplyCBPreset(),
        presetList(),
        presetNameChangedList(),
        presetUpdatedList(),
        presetDeletedList() { }

    void GetPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LampState& preset) {
        getPresetReplyCBStatus = responseCode;
        getPresetReplyCBPresetID = presetID;
        getPresetReplyCBPreset = preset;
        replyReceivedFlag = true;
        getPresetDataSetPreset = true;
    }

    void GetAllPresetIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& presetIDs) {
        getAllPresetIDsReplyCBStatus = responseCode;
        presetList = presetIDs;
        replyReceivedFlag = true;
    }

    void GetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language, const LSFString& presetName) {
        getPresetNameReplyCBStatus = responseCode;
        getPresetNameReplyCBPresetID = presetID;
        getPresetNameReplyCBLanguage = language;
        getPresetNameReplyCBPresetName = presetName;
        replyReceivedFlag = true;
        getPresetDataSetName = true;
    }

    void SetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language) {
        setPresetNameReplyCBStatus = responseCode;
        setPresetNameReplyCBPresetID = presetID;
        setPresetNameReplyCBLanguage = language;
        replyReceivedFlag = true;
    }

    void PresetsNameChangedCB(const LSFStringList& presetIDs) {
        presetNameChangedList = presetIDs;
        signalReceivedFlag = true;
    }

    void CreatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        createPresetReplyCBStatus = responseCode;
        createPresetReplyCBID = presetID;
        replyReceivedFlag = true;
    }

    void PresetsCreatedCB(const LSFStringList& presetIDs) {
        presetList = presetIDs;
        signalReceivedFlag = true;
    }

    void UpdatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        updatePresetReplyCBStatus = responseCode;
        updatePresetReplyCBID = presetID;
        replyReceivedFlag = true;
    }

    void PresetsUpdatedCB(const LSFStringList& presetIDs) {
        presetUpdatedList = presetIDs;
        signalReceivedFlag = true;
    }

    void DeletePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        deletePresetReplyCBStatus = responseCode;
        deletePresetReplyCBID = presetID;
        replyReceivedFlag = true;
    }

    void PresetsDeletedCB(const LSFStringList& presetIDs) {
        presetDeletedList = presetIDs;
        signalReceivedFlag = true;
    }

    void GetDefaultLampStateReplyCB(const LSFResponseCode& responseCode, const LampState& defaultState) {
        getDefaultLampStateReplyCBStatus = responseCode;
        getDefaultLampStateReplyCBDefaultLampState = defaultState;
        replyReceivedFlag = true;
    }

    void SetDefaultLampStateReplyCB(const LSFResponseCode& responseCode) {
        setDefaultLampStateReplyCBStatus = responseCode;
        replyReceivedFlag = true;
    }

    void DefaultLampStateChangedCB(void) {
        defaultLampStateChangedCBStatus = LSF_OK;
        signalReceivedFlag = true;
    }

    LSFResponseCode createPresetReplyCBStatus;
    LSFString createPresetReplyCBID;
    LSFResponseCode updatePresetReplyCBStatus;
    LSFString updatePresetReplyCBID;
    LSFResponseCode deletePresetReplyCBStatus;
    LSFString deletePresetReplyCBID;
    LSFResponseCode getDefaultLampStateReplyCBStatus;
    LampState getDefaultLampStateReplyCBDefaultLampState;
    LSFResponseCode setDefaultLampStateReplyCBStatus;
    LSFResponseCode defaultLampStateChangedCBStatus;
    LSFResponseCode getAllPresetIDsReplyCBStatus;
    LSFResponseCode getPresetNameReplyCBStatus;
    LSFString getPresetNameReplyCBPresetID;
    LSFString getPresetNameReplyCBLanguage;
    LSFString getPresetNameReplyCBPresetName;
    LSFResponseCode setPresetNameReplyCBStatus;
    LSFString setPresetNameReplyCBPresetID;
    LSFString setPresetNameReplyCBLanguage;
    LSFResponseCode getPresetReplyCBStatus;
    LSFString getPresetReplyCBPresetID;
    LampState getPresetReplyCBPreset;
    LSFStringList presetList;
    LSFStringList presetNameChangedList;
    LSFStringList presetUpdatedList;
    LSFStringList presetDeletedList;
};

class SceneManagerCallbackHandler : public SceneManagerCallback {
  public:

    SceneManagerCallbackHandler() :
        createSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        createSceneReplyCBID(),
        updateSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        updateSceneReplyCBID(),
        deleteSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        deleteSceneReplyCBID(),
        getAllSceneIDsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getSceneNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        getSceneNameReplyCBSceneID(),
        getSceneNameReplyCBLanguage(),
        getSceneNameReplyCBSceneName(),
        setSceneNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        setSceneNameReplyCBSceneID(),
        setSceneNameReplyCBLanguage(),
        getSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        getSceneReplyCBSceneID(),
        getSceneReplyCBScene(),
        applySceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        applySceneReplyCBSceneID(),
        sceneList(),
        sceneNameChangedList(),
        sceneUpdatedList(),
        sceneDeletedList(),
        sceneAppliedList() { }

    void GetSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const Scene& scene) {
        getSceneReplyCBStatus = responseCode;
        getSceneReplyCBSceneID = sceneID;
        getSceneReplyCBScene = scene;
        replyReceivedFlag = true;
        getSceneDataSetScene = true;
    }

    void GetAllSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& sceneIDs) {
        getAllSceneIDsReplyCBStatus = responseCode;
        sceneList = sceneIDs;
        replyReceivedFlag = true;
    }

    void GetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language, const LSFString& sceneName) {
        getSceneNameReplyCBStatus = responseCode;
        getSceneNameReplyCBSceneID = sceneID;
        getSceneNameReplyCBLanguage = language;
        getSceneNameReplyCBSceneName = sceneName;
        replyReceivedFlag = true;
        getSceneDataSetName = true;
    }

    void SetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language) {
        setSceneNameReplyCBStatus = responseCode;
        setSceneNameReplyCBSceneID = sceneID;
        setSceneNameReplyCBLanguage = language;
        replyReceivedFlag = true;
    }

    void ScenesNameChangedCB(const LSFStringList& sceneIDs) {
        sceneNameChangedList = sceneIDs;
        signalReceivedFlag = true;
    }

    void CreateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        createSceneReplyCBStatus = responseCode;
        createSceneReplyCBID = sceneID;
        replyReceivedFlag = true;
    }

    void ScenesCreatedCB(const LSFStringList& sceneIDs) {
        sceneList = sceneIDs;
        signalReceivedFlag = true;
    }

    void UpdateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        updateSceneReplyCBStatus = responseCode;
        updateSceneReplyCBID = sceneID;
        replyReceivedFlag = true;
    }

    void ScenesUpdatedCB(const LSFStringList& sceneIDs) {
        sceneUpdatedList = sceneIDs;
        signalReceivedFlag = true;
    }

    void DeleteSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        deleteSceneReplyCBStatus = responseCode;
        deleteSceneReplyCBID = sceneID;
        replyReceivedFlag = true;
    }

    void ScenesDeletedCB(const LSFStringList& sceneIDs) {
        sceneDeletedList = sceneIDs;
        signalReceivedFlag = true;
    }

    void ApplySceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        applySceneReplyCBStatus = responseCode;
        applySceneReplyCBSceneID = sceneID;
        replyReceivedFlag = true;
    }

    void ScenesAppliedCB(const LSFStringList& sceneIDs) {
        sceneAppliedList = sceneIDs;
        sceneSignalReceivedFlag = true;
    }

    LSFResponseCode createSceneReplyCBStatus;
    LSFString createSceneReplyCBID;
    LSFResponseCode updateSceneReplyCBStatus;
    LSFString updateSceneReplyCBID;
    LSFResponseCode deleteSceneReplyCBStatus;
    LSFString deleteSceneReplyCBID;
    LSFResponseCode getAllSceneIDsReplyCBStatus;
    LSFResponseCode getSceneNameReplyCBStatus;
    LSFString getSceneNameReplyCBSceneID;
    LSFString getSceneNameReplyCBLanguage;
    LSFString getSceneNameReplyCBSceneName;
    LSFResponseCode setSceneNameReplyCBStatus;
    LSFString setSceneNameReplyCBSceneID;
    LSFString setSceneNameReplyCBLanguage;
    LSFResponseCode getSceneReplyCBStatus;
    LSFString getSceneReplyCBSceneID;
    Scene getSceneReplyCBScene;
    LSFResponseCode applySceneReplyCBStatus;
    LSFString applySceneReplyCBSceneID;
    LSFStringList sceneList;
    LSFStringList sceneNameChangedList;
    LSFStringList sceneUpdatedList;
    LSFStringList sceneDeletedList;
    LSFStringList sceneAppliedList;
};

class MasterSceneManagerCallbackHandler : public MasterSceneManagerCallback {
  public:

    MasterSceneManagerCallbackHandler() :
        createMasterSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        createMasterSceneReplyCBID(),
        updateMasterSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        updateMasterSceneReplyCBID(),
        deleteMasterSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        deleteMasterSceneReplyCBID(),
        getAllMasterSceneIDsReplyCBStatus(LSF_ERR_UNEXPECTED),
        getMasterSceneNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        getMasterSceneNameReplyCBMasterSceneID(),
        getMasterSceneNameReplyCBLanguage(),
        getMasterSceneNameReplyCBMasterSceneName(),
        setMasterSceneNameReplyCBStatus(LSF_ERR_UNEXPECTED),
        setMasterSceneNameReplyCBMasterSceneID(),
        setMasterSceneNameReplyCBLanguage(),
        getMasterSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        getMasterSceneReplyCBMasterSceneID(),
        getMasterSceneReplyCBMasterScene(),
        applyMasterSceneReplyCBStatus(LSF_ERR_UNEXPECTED),
        applyMasterSceneReplyCBMasterSceneID(),
        masterSceneList(),
        masterSceneNameChangedList(),
        masterSceneUpdatedList(),
        masterSceneDeletedList(),
        masterSceneAppliedList() { }

    void GetMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const MasterScene& masterScene) {
        getMasterSceneReplyCBStatus = responseCode;
        getMasterSceneReplyCBMasterSceneID = masterSceneID;
        getMasterSceneReplyCBMasterScene = masterScene;
        replyReceivedFlag = true;
        getMasterSceneDataSetMasterScene = true;
    }

    void GetAllMasterSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& masterSceneIDs) {
        getAllMasterSceneIDsReplyCBStatus = responseCode;
        masterSceneList = masterSceneIDs;
        replyReceivedFlag = true;
    }

    void GetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language, const LSFString& masterSceneName) {
        getMasterSceneNameReplyCBStatus = responseCode;
        getMasterSceneNameReplyCBMasterSceneID = masterSceneID;
        getMasterSceneNameReplyCBLanguage = language;
        getMasterSceneNameReplyCBMasterSceneName = masterSceneName;
        replyReceivedFlag = true;
        getMasterSceneDataSetName = true;
    }

    void SetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language) {
        setMasterSceneNameReplyCBStatus = responseCode;
        setMasterSceneNameReplyCBMasterSceneID = masterSceneID;
        setMasterSceneNameReplyCBLanguage = language;
        replyReceivedFlag = true;
    }

    void MasterScenesNameChangedCB(const LSFStringList& masterSceneIDs) {
        masterSceneNameChangedList = masterSceneIDs;
        signalReceivedFlag = true;
    }

    void CreateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        createMasterSceneReplyCBStatus = responseCode;
        createMasterSceneReplyCBID = masterSceneID;
        replyReceivedFlag = true;
    }

    void MasterScenesCreatedCB(const LSFStringList& masterSceneIDs) {
        masterSceneList = masterSceneIDs;
        signalReceivedFlag = true;
    }

    void UpdateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        updateMasterSceneReplyCBStatus = responseCode;
        updateMasterSceneReplyCBID = masterSceneID;
        replyReceivedFlag = true;
    }

    void MasterScenesUpdatedCB(const LSFStringList& masterSceneIDs) {
        masterSceneUpdatedList = masterSceneIDs;
        signalReceivedFlag = true;
    }

    void DeleteMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        deleteMasterSceneReplyCBStatus = responseCode;
        deleteMasterSceneReplyCBID = masterSceneID;
        replyReceivedFlag = true;
    }

    void MasterScenesDeletedCB(const LSFStringList& masterSceneIDs) {
        masterSceneDeletedList = masterSceneIDs;
        signalReceivedFlag = true;
    }

    void ApplyMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        applyMasterSceneReplyCBStatus = responseCode;
        applyMasterSceneReplyCBMasterSceneID = masterSceneID;
        replyReceivedFlag = true;
    }

    void MasterScenesAppliedCB(const LSFStringList& masterSceneIDs) {
        masterSceneAppliedList = masterSceneIDs;
        sceneSignalReceivedFlag = true;
    }

    LSFResponseCode createMasterSceneReplyCBStatus;
    LSFString createMasterSceneReplyCBID;
    LSFResponseCode updateMasterSceneReplyCBStatus;
    LSFString updateMasterSceneReplyCBID;
    LSFResponseCode deleteMasterSceneReplyCBStatus;
    LSFString deleteMasterSceneReplyCBID;
    LSFResponseCode getAllMasterSceneIDsReplyCBStatus;
    LSFResponseCode getMasterSceneNameReplyCBStatus;
    LSFString getMasterSceneNameReplyCBMasterSceneID;
    LSFString getMasterSceneNameReplyCBLanguage;
    LSFString getMasterSceneNameReplyCBMasterSceneName;
    LSFResponseCode setMasterSceneNameReplyCBStatus;
    LSFString setMasterSceneNameReplyCBMasterSceneID;
    LSFString setMasterSceneNameReplyCBLanguage;
    LSFResponseCode getMasterSceneReplyCBStatus;
    LSFString getMasterSceneReplyCBMasterSceneID;
    MasterScene getMasterSceneReplyCBMasterScene;
    LSFResponseCode applyMasterSceneReplyCBStatus;
    LSFString applyMasterSceneReplyCBMasterSceneID;
    LSFStringList masterSceneList;
    LSFStringList masterSceneNameChangedList;
    LSFStringList masterSceneUpdatedList;
    LSFStringList masterSceneDeletedList;
    LSFStringList masterSceneAppliedList;
};

class ControllerClientTest : public testing::Test {
  public:
    BusAttachment bus;

    ControllerClientTest() : bus("LSFUnitTest", true),
        controllerClientCBHandler(),
        controllerServiceManagerCBHandler(),
        lampManagerCBHandler(),
        lampGroupManagerCBHandler(),
        presetManagerCBHandler(),
        sceneManagerCBHandler(),
        masterSceneManagerCBHandler(),
        client(bus, controllerClientCBHandler),
        controllerServiceManager(client, controllerServiceManagerCBHandler),
        lampManager(client, lampManagerCBHandler),
        lampGroupManager(client, lampGroupManagerCBHandler),
        presetManager(client, presetManagerCBHandler),
        sceneManager(client, sceneManagerCBHandler),
        masterSceneManager(client, masterSceneManagerCBHandler) { }

    virtual void SetUp() {
        QStatus status = ER_OK;
        status = bus.Start();
        ASSERT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
        ASSERT_FALSE(bus.IsConnected());
        status = bus.Connect();
        ASSERT_EQ(ER_OK, status) << "  Actual Status: " << QCC_StatusText(status);
        ASSERT_TRUE(bus.IsConnected());
    }

    virtual void TearDown() {
        bus.Stop();
        bus.Join();
    }

    ControllerClientCallbackHandler controllerClientCBHandler;
    ControllerServiceManagerCallbackHandler controllerServiceManagerCBHandler;
    LampManagerCallbackHandler lampManagerCBHandler;
    LampGroupManagerCallbackHandler lampGroupManagerCBHandler;
    PresetManagerCallbackHandler presetManagerCBHandler;
    SceneManagerCallbackHandler sceneManagerCBHandler;
    MasterSceneManagerCallbackHandler masterSceneManagerCBHandler;

    ControllerClient client;
    ControllerServiceManager controllerServiceManager;
    LampManager lampManager;
    LampGroupManager lampGroupManager;
    PresetManager presetManager;
    SceneManager sceneManager;
    MasterSceneManager masterSceneManager;
};

TEST_F(ControllerClientTest, Controller_Client_Start_And_Connected) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);
}

TEST_F(ControllerClientTest, Controller_Client_GetControllerServiceVersion) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = controllerServiceManager.GetControllerServiceVersion();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    uint32_t expectedVersion = 1;
    EXPECT_EQ(expectedVersion, controllerServiceManagerCBHandler.controllerServiceVersion);
}

TEST_F(ControllerClientTest, Controller_Client_LightingResetControllerService) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = controllerServiceManager.LightingResetControllerService();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerServiceManagerCBHandler.lightingResetControllerServiceReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerServiceManagerCBHandler.controllerServiceLightingResetCBStatus);
}

TEST_F(ControllerClientTest, Controller_Client_GetAllLampIDs) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
}

TEST_F(ControllerClientTest, Controller_Client_GetLampDataSet) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    getLampDataSetState = false;
    getLampDataSetDetails = false;
    getLampDataSetName = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampDataSet(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getLampDataSetState) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateReplyCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.getLampStateReplyCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.getLampStateReplyCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.getLampStateReplyCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.getLampStateReplyCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.getLampStateReplyCBLampState.brightness);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getLampDataSetName) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampNameReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampNameReplyCBLampID);

    LSFString language("en");
    EXPECT_EQ(language, lampManagerCBHandler.getLampNameReplyCBLanguage);

    LSFString name("Device Name");
    EXPECT_EQ(name, lampManagerCBHandler.getLampNameReplyCBLampName);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getLampDataSetDetails) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampDetailsReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampDetailsReplyCBLampID);

    LampDetails details;
    details.make = MAKE_LIFX;
    details.model = MODEL_LED;
    details.type = TYPE_LAMP;
    details.lampType = LAMPTYPE_PAR30;
    details.lampBaseType = BASETYPE_E17;
    details.lampBeamAngle = 100;
    details.dimmable = true;
    details.color = true;
    details.variableColorTemp = true;
    details.hasEffects = true;
    details.maxVoltage = 120;
    details.minVoltage = 120;
    details.wattage = 9;
    details.incandescentEquivalent = 75;
    details.maxLumens = 900;
    details.minTemperature = 2700;
    details.maxTemperature = 5000;
    details.colorRenderingIndex = 0;
    details.lampID = lampID;

    EXPECT_EQ(details.make, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.make);
    EXPECT_EQ(details.model, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.model);
    EXPECT_EQ(details.type, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.type);
    EXPECT_EQ(details.lampType, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampType);
    EXPECT_EQ(details.lampBaseType, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampBaseType);
    EXPECT_EQ(details.lampBeamAngle, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampBeamAngle);
    EXPECT_EQ(details.color, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.color);
    EXPECT_EQ(details.variableColorTemp, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.variableColorTemp);
    EXPECT_EQ(details.hasEffects, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.hasEffects);
    EXPECT_EQ(details.maxVoltage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxVoltage);
    EXPECT_EQ(details.minVoltage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.minVoltage);
    EXPECT_EQ(details.wattage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.wattage);
    EXPECT_EQ(details.incandescentEquivalent, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.incandescentEquivalent);
    EXPECT_EQ(details.maxLumens, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxLumens);
    EXPECT_EQ(details.minTemperature, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.minTemperature);
    EXPECT_EQ(details.maxTemperature, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxTemperature);
    EXPECT_EQ(details.colorRenderingIndex, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.colorRenderingIndex);
    EXPECT_EQ(details.lampID, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampID);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampManufacturer) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampManufacturer(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampManufacturerReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampManufacturerReplyCBLampID);

    LSFString language("en");
    EXPECT_EQ(language, lampManagerCBHandler.getLampManufacturerReplyCBLanguage);

    LSFString manufacturer("Company A(EN)");
    EXPECT_EQ(manufacturer, lampManagerCBHandler.getLampManufacturerReplyCBManufacturer);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampSupportedLanguages) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampSupportedLanguages(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampSupportedLanguagesReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampSupportedLanguagesReplyCBLampID);

    LSFString language1("en");
    EXPECT_EQ(language1, lampManagerCBHandler.getLampSupportedLanguagesReplyCBSupportedLanguages.front());

    lampManagerCBHandler.getLampSupportedLanguagesReplyCBSupportedLanguages.pop_front();

    LSFString language2("de-AT");
    EXPECT_EQ(language2, lampManagerCBHandler.getLampSupportedLanguagesReplyCBSupportedLanguages.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetLampName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampName(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampNameReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampNameReplyCBLampID);

    LSFString language("en");
    EXPECT_EQ(language, lampManagerCBHandler.getLampNameReplyCBLanguage);

    LSFString name("Device Name");
    EXPECT_EQ(name, lampManagerCBHandler.getLampNameReplyCBLampName);
}

TEST_F(ControllerClientTest, Controller_Client_SetLampName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LSFString newName("New Name");
    localStatus = lampManager.SetLampName(lampID, newName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.setLampNameReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.setLampNameReplyCBLampID);

    LSFString language("en");
    EXPECT_EQ(language, lampManagerCBHandler.setLampNameReplyCBLanguage);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampNameChangedCBLampID);
    EXPECT_EQ(newName, lampManagerCBHandler.lampNameChangedCBLampName);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampState(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateReplyCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.getLampStateReplyCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.getLampStateReplyCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.getLampStateReplyCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.getLampStateReplyCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.getLampStateReplyCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampStateOnOffField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampStateOnOffField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateOnOffFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateOnOffFieldReplyCBLampID);

    bool onOff = true;
    EXPECT_EQ(onOff, lampManagerCBHandler.getLampStateOnOffFieldReplyCBOnOff);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampStateHueField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampStateHueField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateHueFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateHueFieldReplyCBLampID);

    uint32_t hue = 0;
    EXPECT_EQ(hue, lampManagerCBHandler.getLampStateHueFieldReplyCBHue);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampStateSaturationField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampStateSaturationField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateSaturationFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateSaturationFieldReplyCBLampID);

    uint32_t saturation = 0;
    EXPECT_EQ(saturation, lampManagerCBHandler.getLampStateSaturationFieldReplyCBSaturation);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampStateBrightnessField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampStateBrightnessField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateBrightnessFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateBrightnessFieldReplyCBLampID);

    uint32_t brightness = 0;
    EXPECT_EQ(brightness, lampManagerCBHandler.getLampStateBrightnessFieldReplyCBBrightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampStateColorTempField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampStateColorTempField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampStateColorTempFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampStateColorTempFieldReplyCBLampID);

    uint32_t colorTemp = 0;
    EXPECT_EQ(colorTemp, lampManagerCBHandler.getLampStateColorTempFieldReplyCBColorTemp);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampState(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampStateOnOffField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampStateOnOffField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateOnOffFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateOnOffFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampStateHueField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampStateHueField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateHueFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateHueFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampStateSaturationField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampStateSaturationField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateSaturationFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateSaturationFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampStateBrightnessField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampStateBrightnessField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateBrightnessFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateBrightnessFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampStateColorTempField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.ResetLampStateColorTempField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.resetLampStateColorTempFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.resetLampStateColorTempFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LampState state(false, 5, 5, 5, 5);
    localStatus = lampManager.TransitionLampState(lampID, state);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateOnOffField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    bool onOff = true;
    localStatus = lampManager.TransitionLampStateOnOffField(lampID, onOff);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateOnOffFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateOnOffFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateHueField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    uint32_t hue = 6;
    localStatus = lampManager.TransitionLampStateHueField(lampID, hue);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateHueFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateHueFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateSaturationField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    uint32_t saturation = 6;
    localStatus = lampManager.TransitionLampStateSaturationField(lampID, saturation);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateSaturationFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateSaturationFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateBrightnessField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    uint32_t brightness = 6;
    localStatus = lampManager.TransitionLampStateBrightnessField(lampID, brightness);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateBrightnessFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateBrightnessFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 5, 6);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateColorTempField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    uint32_t colorTemp = 6;
    localStatus = lampManager.TransitionLampStateColorTempField(lampID, colorTemp);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateColorTempFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateColorTempFieldReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 6, 6);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_PulseLampWithState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampManager.PulseLampWithState(lampID, toState, 100, 10, 20, fromState);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.pulseLampWithStateReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.pulseLampWithStateReplyCBLampID);
}

TEST_F(ControllerClientTest, Controller_Client_PulseLampWithPreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampManager.PulseLampWithPreset(lampID, presetManagerCBHandler.createPresetReplyCBID, 100, 10, 20);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.pulseLampWithPresetReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.pulseLampWithPresetReplyCBLampID);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampStateToPreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampManager.TransitionLampStateToPreset(lampID, presetManagerCBHandler.createPresetReplyCBID, 25);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.transitionLampStateToPresetReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.transitionLampStateToPresetReplyCBLampID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(preset.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(preset.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(preset.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(preset.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(preset.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampFaults) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampFaults(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampFaultsReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampFaultsReplyCBLampID);

    LampFaultCode fault = 1;
    EXPECT_EQ(fault, lampManagerCBHandler.faultsList.front());
    lampManagerCBHandler.faultsList.pop_front();
    fault = 4;
    EXPECT_EQ(fault, lampManagerCBHandler.faultsList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ClearLampFault) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    LampFaultCode fault = 1;
    localStatus = lampManager.ClearLampFault(lampID, fault);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.clearLampFaultReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.clearLampFaultReplyCBLampID);

    EXPECT_EQ(fault, lampManagerCBHandler.lampFaultCode);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampServiceVersion) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampServiceVersion(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampServiceVersionReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampServiceVersionReplyCBLampID);

    uint32_t version = 1;
    EXPECT_EQ(version, lampManagerCBHandler.lampVersion);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampDetails) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampDetails(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampDetailsReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampDetailsReplyCBLampID);

    LampDetails details;
    details.make = MAKE_LIFX;
    details.model = MODEL_LED;
    details.type = TYPE_LAMP;
    details.lampType = LAMPTYPE_PAR30;
    details.lampBaseType = BASETYPE_E17;
    details.lampBeamAngle = 100;
    details.dimmable = true;
    details.color = true;
    details.variableColorTemp = true;
    details.hasEffects = true;
    details.maxVoltage = 120;
    details.minVoltage = 120;
    details.wattage = 9;
    details.incandescentEquivalent = 75;
    details.maxLumens = 900;
    details.minTemperature = 2700;
    details.maxTemperature = 5000;
    details.colorRenderingIndex = 0;
    details.lampID = lampID;

    EXPECT_EQ(details.make, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.make);
    EXPECT_EQ(details.model, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.model);
    EXPECT_EQ(details.type, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.type);
    EXPECT_EQ(details.lampType, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampType);
    EXPECT_EQ(details.lampBaseType, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampBaseType);
    EXPECT_EQ(details.lampBeamAngle, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampBeamAngle);
    EXPECT_EQ(details.color, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.color);
    EXPECT_EQ(details.variableColorTemp, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.variableColorTemp);
    EXPECT_EQ(details.hasEffects, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.hasEffects);
    EXPECT_EQ(details.maxVoltage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxVoltage);
    EXPECT_EQ(details.minVoltage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.minVoltage);
    EXPECT_EQ(details.wattage, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.wattage);
    EXPECT_EQ(details.incandescentEquivalent, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.incandescentEquivalent);
    EXPECT_EQ(details.maxLumens, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxLumens);
    EXPECT_EQ(details.minTemperature, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.minTemperature);
    EXPECT_EQ(details.maxTemperature, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.maxTemperature);
    EXPECT_EQ(details.colorRenderingIndex, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.colorRenderingIndex);
    EXPECT_EQ(details.lampID, lampManagerCBHandler.getLampDetailsReplyCBLampDetails.lampID);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampParameters) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampParameters(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampParametersReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampParametersReplyCBLampID);

    LampParameters parameters;
    parameters.energyUsageMilliwatts = 15;
    parameters.lumens = 100;
    EXPECT_EQ(parameters.energyUsageMilliwatts, lampManagerCBHandler.getLampParametersReplyCBLampParameters.energyUsageMilliwatts);
    EXPECT_EQ(parameters.lumens, lampManagerCBHandler.getLampParametersReplyCBLampParameters.lumens);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampParametersEnergyUsageMilliwattsField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampParametersEnergyUsageMilliwattsField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampParametersEnergyUsageMilliwattsFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampParametersEnergyUsageMilliwattsFieldReplyCBLampID);

    uint32_t energyUsageMilliwatts = 15;
    EXPECT_EQ(energyUsageMilliwatts, lampManagerCBHandler.getLampParametersEnergyUsageMilliwattsFieldReplyCBLampParametersEnergyUsageMilliwattsField);
}

TEST_F(ControllerClientTest, Controller_Client_GetLampParametersLumensField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampID = lampManagerCBHandler.lampList.front();
    localStatus = lampManager.GetLampParametersLumensField(lampID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getLampParametersLumensFieldReplyCBStatus);
    EXPECT_EQ(lampID, lampManagerCBHandler.getLampParametersLumensFieldReplyCBLampID);

    uint32_t lumens = 100;
    EXPECT_EQ(lumens, lampManagerCBHandler.getLampParametersLumensFieldReplyCBLampParametersLumensField);
}

TEST_F(ControllerClientTest, Controller_Client_GetDefaultLampState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetDefaultLampState();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getDefaultLampStateReplyCBStatus);

    LampState state(1, 0, 0, 0, 0);
    EXPECT_EQ(state.onOff, presetManagerCBHandler.getDefaultLampStateReplyCBDefaultLampState.onOff);
    EXPECT_EQ(state.hue, presetManagerCBHandler.getDefaultLampStateReplyCBDefaultLampState.hue);
    EXPECT_EQ(state.saturation, presetManagerCBHandler.getDefaultLampStateReplyCBDefaultLampState.saturation);
    EXPECT_EQ(state.colorTemp, presetManagerCBHandler.getDefaultLampStateReplyCBDefaultLampState.colorTemp);
    EXPECT_EQ(state.brightness, presetManagerCBHandler.getDefaultLampStateReplyCBDefaultLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_SetDefaultLampState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState state(false, 5, 5, 5, 5);
    localStatus = presetManager.SetDefaultLampState(state);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.setDefaultLampStateReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.defaultLampStateChangedCBStatus);
}

TEST_F(ControllerClientTest, Controller_Client_GetAllPresetIDs) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());
}

TEST_F(ControllerClientTest, Controller_Client_GetPresetDataSet) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    getPresetDataSetName = false;
    getPresetDataSetPreset = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString presetID = presetManagerCBHandler.presetList.front();
    localStatus = presetManager.GetPresetDataSet(presetID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getPresetDataSetName) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getPresetNameReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.getPresetNameReplyCBPresetID);

    LSFString language("en");
    EXPECT_EQ(language, presetManagerCBHandler.getPresetNameReplyCBLanguage);

    LSFString name("SamplePreset");
    EXPECT_EQ(name, presetManagerCBHandler.getPresetNameReplyCBPresetName);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getPresetDataSetPreset) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getPresetReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.getPresetReplyCBPresetID);

    LampState state(true, 10, 10, 10, 10);
    EXPECT_EQ(state.onOff, presetManagerCBHandler.getPresetReplyCBPreset.onOff);
    EXPECT_EQ(state.hue, presetManagerCBHandler.getPresetReplyCBPreset.hue);
    EXPECT_EQ(state.saturation, presetManagerCBHandler.getPresetReplyCBPreset.saturation);
    EXPECT_EQ(state.colorTemp, presetManagerCBHandler.getPresetReplyCBPreset.colorTemp);
    EXPECT_EQ(state.brightness, presetManagerCBHandler.getPresetReplyCBPreset.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetPresetName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString presetID = presetManagerCBHandler.presetList.front();
    localStatus = presetManager.GetPresetName(presetID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getPresetNameReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.getPresetNameReplyCBPresetID);

    LSFString language("en");
    EXPECT_EQ(language, presetManagerCBHandler.getPresetNameReplyCBLanguage);

    LSFString name("SamplePreset");
    EXPECT_EQ(name, presetManagerCBHandler.getPresetNameReplyCBPresetName);
}

TEST_F(ControllerClientTest, Controller_Client_SetPresetName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString presetID = presetManagerCBHandler.presetList.front();
    LSFString newName("New Name");
    localStatus = presetManager.SetPresetName(presetID, newName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.setPresetNameReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.setPresetNameReplyCBPresetID);

    LSFString language("en");
    EXPECT_EQ(language, presetManagerCBHandler.setPresetNameReplyCBLanguage);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetID, presetManagerCBHandler.presetNameChangedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_CreatePreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());
}

TEST_F(ControllerClientTest, Controller_Client_UpdatePreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 20, 20, 20, 20);
    LSFString presetID = presetManagerCBHandler.presetList.front();
    localStatus = presetManager.UpdatePreset(presetID, preset);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.updatePresetReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.updatePresetReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.updatePresetReplyCBID, presetManagerCBHandler.presetUpdatedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_DeletePreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString presetID = presetManagerCBHandler.presetList.front();
    localStatus = presetManager.DeletePreset(presetID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.deletePresetReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.deletePresetReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.deletePresetReplyCBID, presetManagerCBHandler.presetDeletedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetPreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.GetAllPresetIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getAllPresetIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LT(listSize, presetManagerCBHandler.presetList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString presetID = presetManagerCBHandler.presetList.front();
    localStatus = presetManager.GetPreset(presetID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.getPresetReplyCBStatus);
    EXPECT_EQ(presetID, presetManagerCBHandler.getPresetReplyCBPresetID);

    LampState state(true, 10, 10, 10, 10);
    EXPECT_EQ(state.onOff, presetManagerCBHandler.getPresetReplyCBPreset.onOff);
    EXPECT_EQ(state.hue, presetManagerCBHandler.getPresetReplyCBPreset.hue);
    EXPECT_EQ(state.saturation, presetManagerCBHandler.getPresetReplyCBPreset.saturation);
    EXPECT_EQ(state.colorTemp, presetManagerCBHandler.getPresetReplyCBPreset.colorTemp);
    EXPECT_EQ(state.brightness, presetManagerCBHandler.getPresetReplyCBPreset.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_CreateLampGroup) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampManagerCBHandler.lampList.front());
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupState(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupStateOnOffField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupStateOnOffField(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateOnOffFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateOnOffFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupStateHueField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupStateHueField(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateHueFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateHueFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupStateSaturationField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupStateSaturationField(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateSaturationFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateSaturationFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupStateBrightnessField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupStateBrightnessField(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateBrightnessFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateBrightnessFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ResetLampGroupStateColorTempField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.ResetLampGroupStateColorTempField(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.resetLampGroupStateColorTempFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.resetLampGroupStateColorTempFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(false, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    LampState state(false, 5, 5, 5, 5);
    localStatus = lampGroupManager.TransitionLampGroupState(lampGroupID, state);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateOnOffField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    bool onOff = true;
    localStatus = lampGroupManager.TransitionLampGroupStateOnOffField(lampGroupID, onOff);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateOnOffFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateOnOffFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 5, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateHueField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    uint32_t hue = 6;
    localStatus = lampGroupManager.TransitionLampGroupStateHueField(lampGroupID, hue);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateHueFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateHueFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 5, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateSaturationField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    uint32_t saturation = 6;
    localStatus = lampGroupManager.TransitionLampGroupStateSaturationField(lampGroupID, saturation);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateSaturationFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateSaturationFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 5, 5);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateBrightnessField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    uint32_t brightness = 6;
    localStatus = lampGroupManager.TransitionLampGroupStateBrightnessField(lampGroupID, brightness);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateBrightnessFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateBrightnessFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 5, 6);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateColorTempField) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    uint32_t colorTemp = 6;
    localStatus = lampGroupManager.TransitionLampGroupStateColorTempField(lampGroupID, colorTemp);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateColorTempFieldReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateColorTempFieldReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    LampState state(true, 6, 6, 6, 6);
    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_PulseLampGroupWithState) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampGroupManager.PulseLampGroupWithState(lampGroupID, toState, 100, 10, 20, fromState);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.pulseLampGroupWithStateReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.pulseLampGroupWithStateReplyCBLampGroupID);
}

TEST_F(ControllerClientTest, Controller_Client_PulseLampGroupWithPreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampGroupManager.PulseLampGroupWithPreset(lampGroupID, presetManagerCBHandler.createPresetReplyCBID, 100, 10, 20);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.pulseLampGroupWithPresetReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.pulseLampGroupWithPresetReplyCBLampGroupID);
}

TEST_F(ControllerClientTest, Controller_Client_TransitionLampGroupStateToPreset) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_LE(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    LampState toState(false, 5, 5, 5, 5);
    LampState fromState(false, 7, 7, 7, 7);
    localStatus = lampGroupManager.TransitionLampGroupStateToPreset(lampGroupID, presetManagerCBHandler.createPresetReplyCBID, 25);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.transitionLampGroupStateToPresetReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.transitionLampGroupStateToPresetReplyCBLampGroupID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(preset.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(preset.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(preset.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(preset.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(preset.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetAllLampGroupIDs) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());
}

TEST_F(ControllerClientTest, Controller_Client_GetLampGroupDataSet) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    getLampGroupDataSetName = false;
    getLampGroupDataSetGroup = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.GetLampGroupDataSet(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getLampGroupDataSetName) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getLampGroupNameReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.getLampGroupNameReplyCBLampGroupID);

    LSFString language("en");
    EXPECT_EQ(language, lampGroupManagerCBHandler.getLampGroupNameReplyCBLanguage);

    LSFString name("SampleLampGroup");
    EXPECT_EQ(name, lampGroupManagerCBHandler.getLampGroupNameReplyCBLampGroupName);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getLampGroupDataSetGroup) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroupID);

    size_t size = 0;
    EXPECT_EQ(size, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroup.lampGroups.size());

    EXPECT_EQ(lampID, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroup.lamps.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetLampGroupName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.GetLampGroupName(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getLampGroupNameReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.getLampGroupNameReplyCBLampGroupID);

    LSFString language("en");
    EXPECT_EQ(language, lampGroupManagerCBHandler.getLampGroupNameReplyCBLanguage);

    LSFString name("SampleLampGroup");
    EXPECT_EQ(name, lampGroupManagerCBHandler.getLampGroupNameReplyCBLampGroupName);
}

TEST_F(ControllerClientTest, Controller_Client_SetLampGroupName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    LSFString newName("New Name");
    localStatus = lampGroupManager.SetLampGroupName(lampGroupID, newName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.setLampGroupNameReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.setLampGroupNameReplyCBLampGroupID);

    LSFString language("en");
    EXPECT_EQ(language, lampGroupManagerCBHandler.setLampGroupNameReplyCBLanguage);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.lampGroupNameChangedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_UpdateLampGroup) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampGroupList.push_back("abc");
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.UpdateLampGroup(lampGroupID, lampGroup);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.updateLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.updateLampGroupReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.updateLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupUpdatedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetLampGroup) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.GetLampGroup(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroupID);

    LSFString test("abc");
    EXPECT_EQ(test, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroup.lampGroups.front());

    size_t size = 0;
    EXPECT_EQ(size, lampGroupManagerCBHandler.getLampGroupReplyCBLampGroup.lamps.size());
}

TEST_F(ControllerClientTest, Controller_Client_DeleteLampGroup) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.GetAllLampGroupIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.getAllLampGroupIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampGroupManagerCBHandler.lampGroupList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString lampGroupID = lampGroupManagerCBHandler.lampGroupList.front();
    localStatus = lampGroupManager.DeleteLampGroup(lampGroupID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.deleteLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupID, lampGroupManagerCBHandler.deleteLampGroupReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.deleteLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupDeletedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_CreateScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lamps.push_back(lampManagerCBHandler.lampList.front());
    LampState state(true, 3, 3, 3, 3);
    uint32_t transPeriod = 35;
    TransitionLampsLampGroupsToState transitionToStateComponent(lamps, lampGroups, state, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    transitionToStateList.push_back(transitionToStateComponent);

    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString name = LSFString("SampleScene");

    localStatus = sceneManager.CreateScene(scene, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.createSceneReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.createSceneReplyCBID, sceneManagerCBHandler.sceneList.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetSceneDataSet) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();


    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    getSceneDataSetScene = false;
    getSceneDataSetName = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.GetSceneDataSet(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getSceneDataSetScene) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.getSceneReplyCBSceneID);

    EXPECT_EQ(lampID, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().lamps.front());

    size_t size = 0;
    EXPECT_EQ(size, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().lampGroups.size());

    LampState state(true, 3, 3, 3, 3);
    EXPECT_EQ(state.onOff, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.onOff);
    EXPECT_EQ(state.hue, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.hue);
    EXPECT_EQ(state.saturation, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.saturation);
    EXPECT_EQ(state.colorTemp, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.colorTemp);
    EXPECT_EQ(state.brightness, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.brightness);

    uint32_t transPeriod = 35;
    EXPECT_EQ(transPeriod, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().transitionPeriod);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getSceneDataSetName) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getSceneNameReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.getSceneNameReplyCBSceneID);

    LSFString language("en");
    EXPECT_EQ(language, sceneManagerCBHandler.getSceneNameReplyCBLanguage);

    LSFString name("SampleScene");
    EXPECT_EQ(name, sceneManagerCBHandler.getSceneNameReplyCBSceneName);
}

TEST_F(ControllerClientTest, Controller_Client_GetScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();


    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.GetScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.getSceneReplyCBSceneID);

    EXPECT_EQ(lampID, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().lamps.front());

    size_t size = 0;
    EXPECT_EQ(size, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().lampGroups.size());

    LampState state(true, 3, 3, 3, 3);
    EXPECT_EQ(state.onOff, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.onOff);
    EXPECT_EQ(state.hue, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.hue);
    EXPECT_EQ(state.saturation, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.saturation);
    EXPECT_EQ(state.colorTemp, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.colorTemp);
    EXPECT_EQ(state.brightness, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().state.brightness);

    uint32_t transPeriod = 35;
    EXPECT_EQ(transPeriod, sceneManagerCBHandler.getSceneReplyCBScene.transitionToStateComponent.front().transitionPeriod);
}

TEST_F(ControllerClientTest, Controller_Client_GetAllSceneIDs) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());
}

TEST_F(ControllerClientTest, Controller_Client_GetSceneName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.GetSceneName(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getSceneNameReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.getSceneNameReplyCBSceneID);

    LSFString language("en");
    EXPECT_EQ(language, sceneManagerCBHandler.getSceneNameReplyCBLanguage);

    LSFString name("SampleScene");
    EXPECT_EQ(name, sceneManagerCBHandler.getSceneNameReplyCBSceneName);
}

TEST_F(ControllerClientTest, Controller_Client_SetSceneName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    LSFString newName("New Name");
    localStatus = sceneManager.SetSceneName(sceneID, newName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.setSceneNameReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.setSceneNameReplyCBSceneID);

    LSFString language("en");
    EXPECT_EQ(language, sceneManagerCBHandler.setSceneNameReplyCBLanguage);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneNameChangedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyTransitionLampsLampGroupsToState_Lamps) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    LampState state(true, 3, 3, 3, 3);
    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_UpdateScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    LampState state(false, 4, 4, 4, 4);
    uint32_t transPeriod = 50;
    TransitionLampsLampGroupsToState transitionToStateComponent(lamps, lampGroups, state, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    transitionToStateList.push_back(transitionToStateComponent);

    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyTransitionLampsLampGroupsToState_LampGroups) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    LampState state(false, 4, 4, 4, 4);
    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyTransitionLampsLampGroupsToPreset_Lamps) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(true, 10, 10, 10, 10);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lamps.push_back(lampID);
    uint32_t transPeriod = 50;
    TransitionLampsLampGroupsToPreset transitionToPresetComponent(lamps, lampGroups, presetManagerCBHandler.createPresetReplyCBID, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    transitionToPresetList.push_back(transitionToPresetComponent);
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(preset.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(preset.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(preset.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(preset.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(preset.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyTransitionLampsLampGroupsToPreset_LampGroups) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(false, 11, 11, 11, 11);
    name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    uint32_t transPeriod = 50;
    TransitionLampsLampGroupsToPreset transitionToPresetComponent(lamps, lampGroups, presetManagerCBHandler.createPresetReplyCBID, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    transitionToPresetList.push_back(transitionToPresetComponent);
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(preset.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(preset.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(preset.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(preset.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(preset.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyTransitionLampsLampGroupsToPreset_NestedLampGroups) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    lampList.clear();
    lampGroupList.clear();
    lampGroupList.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    LampGroup lampGroup1(lampList, lampGroupList);
    localStatus = lampGroupManager.CreateLampGroup(lampGroup1, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(false, 11, 11, 11, 11);
    name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    uint32_t transPeriod = 50;
    TransitionLampsLampGroupsToPreset transitionToPresetComponent(lamps, lampGroups, presetManagerCBHandler.createPresetReplyCBID, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    transitionToPresetList.push_back(transitionToPresetComponent);
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());

    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(preset.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(preset.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(preset.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(preset.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(preset.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyPulseLampsLampGroupsWithState_Lamps) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lamps.push_back(lampID);
    uint32_t period = 100;
    uint32_t duration = 20;
    uint32_t numPulses = 25;
    LampState state(true, 3, 3, 3, 3);
    PulseLampsLampGroupsWithState pulseWithStateComponent(lamps, lampGroups, state, period, duration, numPulses);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    pulseWithStateList.push_back(pulseWithStateComponent);
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyPulseLampsLampGroupsWithState_LampGroup) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    uint32_t period = 100;
    uint32_t duration = 20;
    uint32_t numPulses = 25;
    LampState state(false, 4, 4, 4, 4);
    PulseLampsLampGroupsWithState pulseWithStateComponent(lamps, lampGroups, state, period, duration, numPulses);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    pulseWithStateList.push_back(pulseWithStateComponent);
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyPulseLampsLampGroupsWithPreset_Lamps) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(false, 11, 11, 11, 11);
    LSFString name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lamps.push_back(lampID);
    uint32_t period = 100;
    uint32_t duration = 20;
    uint32_t numPulses = 25;
    PulseLampsLampGroupsWithPreset pulseWithPresetComponent(lamps, lampGroups, presetManagerCBHandler.createPresetReplyCBID, period, duration, numPulses);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;
    pulseWithPresetList.push_back(pulseWithPresetComponent);

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyScene_WithOnlyPulseLampsLampGroupsWithPreset_LampGroups) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name = LSFString("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(false, 11, 11, 11, 11);
    name = LSFString("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupManagerCBHandler.createLampGroupReplyCBID);
    uint32_t period = 100;
    uint32_t duration = 20;
    uint32_t numPulses = 25;
    PulseLampsLampGroupsWithPreset pulseWithPresetComponent(lamps, lampGroups, presetManagerCBHandler.createPresetReplyCBID, period, duration, numPulses);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;
    pulseWithPresetList.push_back(pulseWithPresetComponent);

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.UpdateScene(sceneID, scene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.updateSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.updateSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.updateSceneReplyCBID, sceneManagerCBHandler.sceneUpdatedList.front());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.ApplyScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.applySceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.applySceneReplyCBSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneID, sceneManagerCBHandler.sceneAppliedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_DeleteScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.GetAllSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.getAllSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, sceneManagerCBHandler.sceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString sceneID = sceneManagerCBHandler.sceneList.front();
    localStatus = sceneManager.DeleteScene(sceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.deleteSceneReplyCBStatus);
    EXPECT_EQ(sceneID, sceneManagerCBHandler.deleteSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.deleteSceneReplyCBID, sceneManagerCBHandler.sceneDeletedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_CreateMasterScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList, lampGroupList;
    lampList.push_back(lampID);
    LampGroup lampGroup(lampList, lampGroupList);
    LSFString name("SampleLampGroup");
    localStatus = lampGroupManager.CreateLampGroup(lampGroup, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());
    nestedLampGroupIDForDependencyCheck = lampGroupManagerCBHandler.createLampGroupReplyCBID;

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lampList1, lampGroupList1;
    lampGroupList1.push_back(nestedLampGroupIDForDependencyCheck);
    LampGroup lampGroup1(lampList1, lampGroupList1);
    localStatus = lampGroupManager.CreateLampGroup(lampGroup1, name);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.createLampGroupReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(lampGroupManagerCBHandler.createLampGroupReplyCBID, lampGroupManagerCBHandler.lampGroupList.front());
    lampGroupIDForDependencyCheck = lampGroupManagerCBHandler.createLampGroupReplyCBID;

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LampState preset(false, 11, 11, 11, 11);
    LSFString presetName("SamplePreset");
    localStatus = presetManager.CreatePreset(preset, presetName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.createPresetReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(presetManagerCBHandler.createPresetReplyCBID, presetManagerCBHandler.presetList.front());
    presetIDForDependencyCheck = presetManagerCBHandler.createPresetReplyCBID;

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList lamps, lampGroups;
    lampGroups.push_back(lampGroupIDForDependencyCheck);
    uint32_t transPeriod = 50;
    TransitionLampsLampGroupsToPreset transitionToPresetComponent(lamps, lampGroups, presetIDForDependencyCheck, transPeriod);

    TransitionLampsLampGroupsToStateList transitionToStateList;
    TransitionLampsLampGroupsToPresetList transitionToPresetList;
    transitionToPresetList.push_back(transitionToPresetComponent);
    PulseLampsLampGroupsWithStateList pulseWithStateList;
    PulseLampsLampGroupsWithPresetList pulseWithPresetList;

    Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList);
    LSFString sceneName("SampleScene");
    localStatus = sceneManager.CreateScene(scene, sceneName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.createSceneReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(sceneManagerCBHandler.createSceneReplyCBID, sceneManagerCBHandler.sceneList.front());
    sceneIDForDependencyCheck = sceneManagerCBHandler.createSceneReplyCBID;

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList sceneList;
    sceneList.push_back(sceneIDForDependencyCheck);
    MasterScene masterScene(sceneList);
    LSFString masterSceneName("SampleMasterScene");
    localStatus = masterSceneManager.CreateMasterScene(masterScene, masterSceneName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.createMasterSceneReplyCBStatus);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneManagerCBHandler.createMasterSceneReplyCBID, masterSceneManagerCBHandler.masterSceneList.front());
}

TEST_F(ControllerClientTest, Controller_Client_ApplyMasterScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampManager.GetAllLampIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampManagerCBHandler.getAllLampIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, lampManagerCBHandler.lampList.size());
    LSFString lampID = lampManagerCBHandler.lampList.front();

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;
    sceneSignalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.ApplyMasterScene(masterSceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.applyMasterSceneReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.applyMasterSceneReplyCBMasterSceneID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (sceneSignalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.masterSceneAppliedList.front());

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.masterSceneAppliedList.front());

    LampState state(false, 11, 11, 11, 11);
    EXPECT_EQ(lampID, lampManagerCBHandler.lampStateChangedCBLampID);

    EXPECT_EQ(state.onOff, lampManagerCBHandler.lampStateChangedCBLampState.onOff);
    EXPECT_EQ(state.hue, lampManagerCBHandler.lampStateChangedCBLampState.hue);
    EXPECT_EQ(state.saturation, lampManagerCBHandler.lampStateChangedCBLampState.saturation);
    EXPECT_EQ(state.colorTemp, lampManagerCBHandler.lampStateChangedCBLampState.colorTemp);
    EXPECT_EQ(state.brightness, lampManagerCBHandler.lampStateChangedCBLampState.brightness);
}

TEST_F(ControllerClientTest, Controller_Client_GetAllMasterSceneIDs) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());
}

TEST_F(ControllerClientTest, Controller_Client_GetMasterSceneName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.GetMasterSceneName(masterSceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getMasterSceneNameReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.getMasterSceneNameReplyCBMasterSceneID);

    LSFString language("en");
    EXPECT_EQ(language, masterSceneManagerCBHandler.getMasterSceneNameReplyCBLanguage);

    LSFString name("SampleMasterScene");
    EXPECT_EQ(name, masterSceneManagerCBHandler.getMasterSceneNameReplyCBMasterSceneName);
}

TEST_F(ControllerClientTest, Controller_Client_SetMasterSceneName) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    LSFString newName("New Name");
    localStatus = masterSceneManager.SetMasterSceneName(masterSceneID, newName);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.setMasterSceneNameReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.setMasterSceneNameReplyCBMasterSceneID);

    LSFString language("en");
    EXPECT_EQ(language, masterSceneManagerCBHandler.setMasterSceneNameReplyCBLanguage);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.masterSceneNameChangedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_DependencyCheckError) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.DeleteScene(sceneIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_ERR_DEPENDENCY, sceneManagerCBHandler.deleteSceneReplyCBStatus);
    EXPECT_EQ(sceneIDForDependencyCheck, sceneManagerCBHandler.deleteSceneReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.DeleteLampGroup(lampGroupIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_ERR_DEPENDENCY, lampGroupManagerCBHandler.deleteLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupIDForDependencyCheck, lampGroupManagerCBHandler.deleteLampGroupReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.DeleteLampGroup(nestedLampGroupIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_ERR_DEPENDENCY, lampGroupManagerCBHandler.deleteLampGroupReplyCBStatus);
    EXPECT_EQ(nestedLampGroupIDForDependencyCheck, lampGroupManagerCBHandler.deleteLampGroupReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.DeletePreset(presetIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_ERR_DEPENDENCY, presetManagerCBHandler.deletePresetReplyCBStatus);
    EXPECT_EQ(presetIDForDependencyCheck, presetManagerCBHandler.deletePresetReplyCBID);
}

TEST_F(ControllerClientTest, Controller_Client_UpdateMasterScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFStringList sceneList;
    sceneList.push_back("abc");
    MasterScene masterScene(sceneList);
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.UpdateMasterScene(masterSceneID, masterScene);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.updateMasterSceneReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.updateMasterSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneManagerCBHandler.updateMasterSceneReplyCBID, masterSceneManagerCBHandler.masterSceneUpdatedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_GetMasterSceneDataSet) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    getMasterSceneDataSetMasterScene = false;
    getMasterSceneDataSetName = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.GetMasterSceneDataSet(masterSceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getMasterSceneDataSetMasterScene) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getMasterSceneReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.getMasterSceneReplyCBMasterSceneID);

    LSFString test("abc");
    EXPECT_EQ(test, masterSceneManagerCBHandler.getMasterSceneReplyCBMasterScene.scenes.front());

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (getMasterSceneDataSetName) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getMasterSceneNameReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.getMasterSceneNameReplyCBMasterSceneID);

    LSFString language("en");
    EXPECT_EQ(language, masterSceneManagerCBHandler.getMasterSceneNameReplyCBLanguage);

    LSFString name("New Name");
    EXPECT_EQ(name, masterSceneManagerCBHandler.getMasterSceneNameReplyCBMasterSceneName);
}

TEST_F(ControllerClientTest, Controller_Client_GetMasterScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.GetMasterScene(masterSceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getMasterSceneReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.getMasterSceneReplyCBMasterSceneID);

    LSFString test("abc");
    EXPECT_EQ(test, masterSceneManagerCBHandler.getMasterSceneReplyCBMasterScene.scenes.front());
}

TEST_F(ControllerClientTest, Controller_Client_DeleteMasterScene) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = masterSceneManager.GetAllMasterSceneIDs();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.getAllMasterSceneIDsReplyCBStatus);

    size_t listSize = 1;
    EXPECT_EQ(listSize, masterSceneManagerCBHandler.masterSceneList.size());

    replyReceivedFlag = false;
    signalReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    LSFString masterSceneID = masterSceneManagerCBHandler.masterSceneList.front();
    localStatus = masterSceneManager.DeleteMasterScene(masterSceneID);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, masterSceneManagerCBHandler.deleteMasterSceneReplyCBStatus);
    EXPECT_EQ(masterSceneID, masterSceneManagerCBHandler.deleteMasterSceneReplyCBID);

    //wait to receive signal
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (signalReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(masterSceneManagerCBHandler.deleteMasterSceneReplyCBID, masterSceneManagerCBHandler.masterSceneDeletedList.front());
}

TEST_F(ControllerClientTest, Controller_Client_DependencyCheckSuccess) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = sceneManager.DeleteScene(sceneIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, sceneManagerCBHandler.deleteSceneReplyCBStatus);
    EXPECT_EQ(sceneIDForDependencyCheck, sceneManagerCBHandler.deleteSceneReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.DeleteLampGroup(lampGroupIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.deleteLampGroupReplyCBStatus);
    EXPECT_EQ(lampGroupIDForDependencyCheck, lampGroupManagerCBHandler.deleteLampGroupReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = lampGroupManager.DeleteLampGroup(nestedLampGroupIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, lampGroupManagerCBHandler.deleteLampGroupReplyCBStatus);
    EXPECT_EQ(nestedLampGroupIDForDependencyCheck, lampGroupManagerCBHandler.deleteLampGroupReplyCBID);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = presetManager.DeletePreset(presetIDForDependencyCheck);
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive reply
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, presetManagerCBHandler.deletePresetReplyCBStatus);
    EXPECT_EQ(presetIDForDependencyCheck, presetManagerCBHandler.deletePresetReplyCBID);
}

TEST_F(ControllerClientTest, Controller_Client_Stop) {
    replyReceivedFlag = false;

    ControllerClientStatus localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Stop();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_ERR_FAILURE, controllerClientCBHandler.connectedToControllerServiceCBStatus);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_ERR_RETRY, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    sleep(40);

    replyReceivedFlag = false;

    localStatus = CONTROLLER_CLIENT_OK;
    localStatus = client.Start();
    ASSERT_EQ(CONTROLLER_CLIENT_OK, localStatus) << "  Actual Status: " << ControllerClientStatusText(localStatus);

    //wait to receive a callback from the controller client
    for (size_t msecs = 0; msecs < 2100; msecs += 5) {
        if (replyReceivedFlag) {
            break;
        }
        sleep(2);
    }

    EXPECT_EQ(LSF_OK, controllerClientCBHandler.connectedToControllerServiceCBStatus);
}
