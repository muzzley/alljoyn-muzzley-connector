#ifndef _LAMP_MANAGER_H_
#define _LAMP_MANAGER_H_
/**
 * \ingroup ControllerClient
 */
/**
 * \file  lighting_controller_client/inc/LampManager.h
 * This file provides definitions for lamp manager
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

#include <LSFTypes.h>
#include <Manager.h>
#include <ControllerClientDefs.h>

#include <alljoyn/InterfaceDescription.h>


#include <LSFResponseCodes.h>

namespace lsf {

class ControllerClient;

/**
 * Abstract base class implemented by User Application Developers.
 * The callbacks defined in this class allow the User Application
 * to be informed when Lamps specific AllJoyn method
 * replies or signals are received from the Lighting Controller
 * Service.
 */
class LampManagerCallback {
  public:

    /**
     * Destructor
     */
    virtual ~LampManagerCallback() { }

    /**
     * Indicates that a reply has been received for the GetAllLampIDs method call
     *
     * @param responseCode  The response code
     * @param lampIDs       The Lamp IDs
     */
    virtual void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     * @param lampName     The required Lamp Name
     */
    virtual void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& lampName) { }

    /**
     * Indicates that a reply has been received for the GetLampManufacturer method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     * @param manufacturer The required manufacturer details
     */
    virtual void GetLampManufacturerReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& manufacturer) { }

    /**
     * Indicates that a reply has been received for the SetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     */
    virtual void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language) { }

    /**
     *  Indicates that the signal LampNameChanged has been received
     *
     *  @param lampID   The Lamp ID
     *  @param lampName The new name of the Lamp
     */
    virtual void LampNameChangedCB(const LSFString& lampID, const LSFString& lampName) { }

    /**
     *  Indicates that the signal LampsFound has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsFoundCB(const LSFStringList& lampIDs) { }

    /**
     *  Indicates that the signal LampsLost has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsLostCB(const LSFStringList& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampDetails method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param lampDetails  The required Lamp Details
     */
    virtual void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampDetails& lampDetails) { }

    /**
     * Indicates that a reply has been received for the GetLampParameters method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampParameters  The required Lamp Parameters
     */
    virtual void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampParameters& lampParameters) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersEnergyUsageMilliwattsField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param energyUsageMilliwatts   The required energyUsageMilliwatts
     */
    virtual void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& energyUsageMilliwatts) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersLumensField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param brightnessLumens    The required brightnessLumens
     */
    virtual void GetLampParametersLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightnessLumens) { }

    /**
     * Indicates that a reply has been received for the GetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampState       The required Lamp State
     */
    virtual void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampState& lampState) { }

    /**
     * Indicates that a reply has been received for the GetLampStateOnOffField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param onOff           The required onOff field
     */
    virtual void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const bool& onOff) { }

    /**
     * Indicates that a reply has been received for the GetLampStateHueField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param hue             The required hue field
     */
    virtual void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& hue) { }

    /**
     * Indicates that a reply has been received for the GetLampStateSaturationField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param saturation      The required saturation field
     */
    virtual void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& saturation) { }

    /**
     * Indicates that a reply has been received for the GetLampStateBrightnessField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param brightness      The required brightness field
     */
    virtual void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightness) { }

    /**
     * Indicates that a reply has been received for the GetLampStateColorTempField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param colorTemp       The required colorTemp field
     */
    virtual void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& colorTemp) { }

    /**
     * Indicates that a reply has been received for the ResetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     *  Indicates that the signal LampStateChanged has been received
     *
     *  @param lampID      The Lamp ID
     *  @param lampState   The Lamp State
     */
    virtual void LampStateChangedCB(const LSFString& lampID, const LampState& lampState) { }

    /**
     * Indicates that a reply has been received for the TransitionLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the PulseLampWithState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void PulseLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the PulseLampWithPreset method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void PulseLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateOnOffField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the  TransitionLampStateHueField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateSaturationField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateBrightnessField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateColorTempField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the GetLampFaults method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCodes      List of Lamp Fault Codes
     */
    virtual void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCodeList& faultCodes) { }

    /**
     * Indicates that a reply has been received for the GetLampServiceVersion method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampServiceVersion
     */
    virtual void GetLampServiceVersionReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& lampServiceVersion) { }

    /**
     * Indicates that a reply has been received for the ClearLampFault method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCode
     */
    virtual void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCode& faultCode) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateOnOffField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateHueField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateSaturationField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateBrightnessField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateColorTempField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateToPreset method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateToPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }
    /**
     * GetLampSupportedLanguages Reply callback
     * @param responseCode
     * @param lampID             - The requested lamp id
     * @param supportedLanguages - the required answer
     */
    virtual void GetLampSupportedLanguagesReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFStringList& supportedLanguages) { };
};

/**
 * LampManager is used by the application to make the controller do useful things related to Lamps
 */
class LampManager : public Manager {

    friend class ControllerClient;

  public:
    /**
     * LampManager constructor
     * @param controller reference
     * @param callback reference of type LampManagerCallback to get the answers asynchronously
     */
    LampManager(ControllerClient& controller, LampManagerCallback& callback);

    /**
     * Get the IDs of all visible Lamps \n
     * Response in  LampManagerCallback::GetAllLampIDsReplyCB
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetAllLampIDs(void);
    /**
     * Get the lamp manufacturer details \n
     * This is the About Framework field “Manufacturer” \n
     * Calls Interface org.allseen.LSF.ControllerService.Lamp method GetLampManufacturer \n
     * The answer will be received at 'GetLampManufacturerReplyCB'
     * @param lampID
     * @param language - the language you want to get the answer. English by default.
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampManufacturer(const LSFString& lampID, const LSFString& language = LSFString("en"));

    /**
     * Get the name of the specified Lamp \n
     * Calls Interface org.allseen.LSF.ControllerService.Lamp method GetLampName \n
     * Response in LampManagerCallback::GetLampNameReplyCB
     * @param lampID
     * @param language - the language to get the answer. Default is english.
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampName(const LSFString& lampID, const LSFString& language = LSFString("en"));

    /**
     * Set an Lamp's Name \n
     * Calls Interface org.allseen.LSF.ControllerService.Lamp method SetLampName \n
     * Response in LampManagerCallback::SetLampNameReplyCB
     *
     * @param lampID    The Lamp ID
     * @param lampName  The Lamp Name
     * @param language  The language which the name is given
     * @return ControllerClientStatus
     */
    ControllerClientStatus SetLampName(const LSFString& lampID, const LSFString& lampName, const LSFString& language = LSFString("en"));

    /**
     * Get the details of a Lamp \n
     * Lamp Details are the LSF-specific data that the Lamp exposes via a Lamp Service interface. \n
     * Such that information about the Lamp can be introspected via a Lighting Controller Service.  \n
     * Lamp Details are read-only and set at the time of manufacturing. \n
     * Calls Interface org.allseen.LSF.ControllerService.Lamp method GetLampDetails. \n
     * Return in LampManagerCallback::GetLampDetailsReplyCB
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampDetails(const LSFString& lampID);

    /**
     * Get the parameters of a given Lamp \n
     * Lamp parameters are read-only volatile parameters that are read from the Lamp hardware. \n
     * This consists of parameters like Lamp Output and Power Draw. \n
     * Following is a programmatic representation of Lamp Parameters: \n
     * Lamp Parameters = {LampOutput, PowerDraw} \n
     * Lamp may implement only a subset of the Lamp Parameters depending on their type. \n \n
     *
     * Calls Interface org.allseen.LSF.ControllerService.Lamp  method GetLampParameters \n
     * Response in LampManagerCallback::GetLampParametersReplyCB
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampParameters(const LSFString& lampID);

    /**
     * Get a given parameter field from the Lamp - Energy_Usage_Milliwatts \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampParametersEnergyUsageMilliwattsFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampParametersEnergyUsageMilliwattsField(const LSFString& lampID) {
        return GetLampParametersField(lampID, LSFString("Energy_Usage_Milliwatts"));
    }

    /**
     * Get a given parameter field from the Lamp - Brightness_Lumens \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampParametersLumensFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampParametersLumensField(const LSFString& lampID) {
        return GetLampParametersField(lampID, LSFString("Brightness_Lumens"));
    }

    /**
     * Get the Lamp's full state \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method GetLampState. \n
     * Saved State is a Lamp State associated with a Lighting Controller Service assigned SavedStateID and a user configurable SavedStateName. \n
     * Saved States are persisted through power cycles in the Lighting Controller Service. \n
     * Following is a programmatic representation of Saved State: \n
     * Saved State = {SavedStateID, SavedStateName, LampState} \n
     * SavedStateID is a Lighting Controller Service generated unique identifier for the Saved State while SavedStateName is a user configurable name for the Saved State \n \n
     *
     * Response in LampManagerCallback::GetLampStateReplyCB
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampState(const LSFString& lampID);

    /**
     * Get the Lamp's state param - OnOff field \n
     * align interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampStateOnOffFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampStateOnOffField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("OnOff"));
    }

    /**
     * Get the Lamp's state param - Hue field \n
     * align interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampStateHueFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampStateHueField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Hue"));
    }

    /**
     * Get the Lamp's state param - Saturation field \n
     * align interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampStateSaturationFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampStateSaturationField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Saturation"));
    }

    /**
     * Get the Lamp's state param - Brightness field \n
     * align interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampStateBrightnessFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampStateBrightnessField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Brightness"));
    }

    /**
     * Get the Lamp's state param - ColorTemp field \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method GetLampStateField \n
     * Response in LampManagerCallback::GetLampStateColorTempFieldReplyCB
     * @param lampID
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampStateColorTempField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("ColorTemp"));
    }

    /**
     * Reset the Lamp's state to the default state \n
     * Response in LampManagerCallback::ResetLampStateReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampState \n
     * Default state values can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampState(const LSFString& lampID);

    /**
     * Reset the Lamp's OnOff field to the default state \n
     * Response in LampManagerCallback::ResetLampStateOnOffFieldReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampStateField \n
     * Default field value can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampStateOnOffField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("OnOff"));
    }

    /**
     * Reset the Lamp's hue field to the default state \n
     * Response in LampManagerCallback::ResetLampStateHueFieldReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampStateField \n
     * Default field value can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampStateHueField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Hue"));
    }

    /**
     * Reset the Lamp's saturation field to the default state \n
     * Response in LampManagerCallback::ResetLampStateSaturationFieldReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampStateField \n
     * Default field value can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampStateSaturationField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Saturation"));
    }

    /**
     * Reset the Lamp's brightness filed to the default state \n
     * Response in LampManagerCallback::ResetLampStateBrightnessFieldReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampStateField \n
     * Default field value can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampStateBrightnessField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Brightness"));
    }

    /**
     * Reset the Lamp's color field to the default state \n
     * Response in LampManagerCallback::ResetLampStateColorTempFieldReplyCB \n
     * Calling interface org.allseen.LSF.ControllerService.Lamp  method ResetLampStateField \n
     * Default field value can be found at OEM_CS_Config.cc
     *
     * @param lampID    The Lamp id
     * @return ControllerClientStatus
     */
    ControllerClientStatus ResetLampStateColorTempField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("ColorTemp"));
    }

    /**
     * Transition the Lamp to a given state \n
     * Response in LampManagerCallback::TransitionLampStateReplyCB \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to function 'TransitionLampState' \n
     * An Effect that transitions a Lamp from its current state to a target state over N milliseconds. \n
     * For a Hue, Saturation, Color Temperature or Brightness the transition, the transition is linear in the HSV color model. \n
     * Response in LampManagerCallback::TransitionLampStateReplyCB
     * @param lampID
     * @param lampState - target state
     * @param transitionPeriod - time in milliseconds
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampState(const LSFString& lampID, const LampState& lampState, const uint32_t& transitionPeriod = 0);

    /**
     * Transition the Lamp to a given state \n
     * Response in LampManagerCallback::PulseLampWithStateReplyCB \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to function 'PulseLampWithState' \n
     * The Pulse Effect shall Brighten then Dim a Lamp set to a predefined hue, saturation, or color temperature at a defined duty cycle  (ratio between pulse duration in milliseconds and period).
     * @param lampID
     * @param toLampState - target lamp state
     * @param period - The time duration of one pulse in milliseconds. Time to transform from 'fromLampState' to 'toLampState'.
     * @param duration - The time duration that the 'fromLampState' will be lingered. Always duration <= period.
     * @param numPulses - number of pulses
     * @param fromLampState - source lamp state
     * @return ControllerClientStatus
     */
    ControllerClientStatus PulseLampWithState(const LSFString& lampID, const LampState& toLampState, const uint32_t& period, const uint32_t& duration, const uint32_t& numPulses, const LampState& fromLampState = LampState());

    /**
     * Transition the Lamp to a given state \n
     * Response in LampManagerCallback::TransitionLSFStringReplyCB \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to function 'PulseLampWithPreset'
     * @param lampID
     * @param toPresetID - the required pre-set identifier
     * @param period - The time duration of one pulse in milliseconds. Time to transform from 'fromPresetID' to 'toPresetID'.
     * @param duration - The time duration that the 'fromPresetID' will be lingered. Always duration <= period.
     * @param numPulses - number of pulses
     * @param fromPresetID - the target pre-set identifier
     */
    ControllerClientStatus PulseLampWithPreset(const LSFString& lampID, const LSFString& toPresetID, const uint32_t& period, const uint32_t& duration, const uint32_t& numPulses, const LSFString& fromPresetID = CurrentStateIdentifier);

    /**
     * Transition lamp state on/off field \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateField' \n
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     * @param lampID
     * @param onOff - boolean on or off
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampStateOnOffField(const LSFString& lampID, const bool& onOff) {
        LSFString name("OnOff");
        return TransitionLampStateBooleanField(lampID, name, onOff);
    }

    /**
     * Transition lamp state hue field \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateField' \n
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     * @param lampID
     * @param hue - boundaries from 0 to 360
     * @param transitionPeriod - The time duration that the transition will take place.
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampStateHueField(const LSFString& lampID, const uint32_t& hue, const uint32_t& transitionPeriod = 0) {
        LSFString name("Hue");
        return TransitionLampStateIntegerField(lampID, name, hue, transitionPeriod);
    }

    /**
     * Transition lamp state saturation field \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateField' \n
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     * @param lampID
     * @param saturation - boundaries from 0 to 100
     * @param transitionPeriod - The time duration that the transition will take place.
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampStateSaturationField(const LSFString& lampID, const uint32_t& saturation, const uint32_t& transitionPeriod = 0) {
        LSFString name("Saturation");
        return TransitionLampStateIntegerField(lampID, name, saturation, transitionPeriod);
    }

    /**
     * Transition lamp state brightness field \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateField' \n
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     * @param lampID
     * @param brightness - boundaries from 0 to 100
     * @param transitionPeriod - The time duration that the transition will take place.
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampStateBrightnessField(const LSFString& lampID, const uint32_t& brightness, const uint32_t& transitionPeriod = 0) {
        LSFString name("Brightness");
        return TransitionLampStateIntegerField(lampID, name, brightness, transitionPeriod);
    }

    /**
     * Transition lamp state color temperature field \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateField' \n
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     * @param lampID
     * @param colorTemp - boundaries from 2700 to 9000 (K)
     * @param transitionPeriod - The time duration that the transition will take place.
     * @return ControllerClientStatus
     */
    ControllerClientStatus TransitionLampStateColorTempField(const LSFString& lampID, const uint32_t& colorTemp, const uint32_t& transitionPeriod = 0) {
        LSFString name("ColorTemp");
        return TransitionLampStateIntegerField(lampID, name, colorTemp, transitionPeriod);
    }

    /**
     * Transition the Lamp to a given pre-set \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'TransitionLampStateToPreset' \n
     * Response in LampManagerCallback::TransitionLampStateToPresetReplyCB
     *
     * @param lampID - The id of the Lamp
     * @param presetID - The id of the target pre-set
     * @param transitionPeriod - time duration of the transition in millisecond
     */
    ControllerClientStatus TransitionLampStateToPreset(const LSFString& lampID, const LSFString& presetID, const uint32_t& transitionPeriod = 0);

    /**
     * Get a list of the Lamp's fault codes \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'GetLampFaults' \n
     * Response in LampManagerCallback::GetLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampFaults(const LSFString& lampID);

    /**
     * Get lamp service version \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'GetLampServiceVersion' \n
     * Response in LampManagerCallback::GetLampServiceVersionReplyCB
     *
     * @param lampID    The id of the Lamp
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampServiceVersion(const LSFString& lampID);

    /**
     * Reset the Lamp's faults \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'ClearLampFault' \n
     * Response in LampManagerCallback::ClearLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     * @param faultCode Lamp fault code
     * @return ControllerClientStatus
     */
    ControllerClientStatus ClearLampFault(const LSFString& lampID, const LampFaultCode& faultCode);
    /**
     * Get Lamp Supported Languages \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'GetLampSupportedLanguages' \n
     * return by LampGroupManagerCallback::GetLampSupportedLanguagesReplyCB
     * @param lampID
     * @return ControllerClientStatus
     *
     */
    ControllerClientStatus GetLampSupportedLanguages(const LSFString& lampID);

    /**
     * Get lamp data - get lamp details, get lamp name and get lamp state \n
     * Calling to interface 'org.allseen.LSF.ControllerService.Lamp' to method 'GetLampDataSet'
     * @param lampID
     * @param language - to get the lamp name
     * @return ControllerClientStatus
     */
    ControllerClientStatus GetLampDataSet(const LSFString& lampID, const LSFString& language = LSFString("en"));

  private:

    ControllerClientStatus GetLampStateField(const LSFString& lampID, const LSFString& stateFieldName);
    ControllerClientStatus ResetLampStateField(const LSFString& lampID, const LSFString& stateFieldName);
    ControllerClientStatus TransitionLampStateIntegerField(const LSFString& lampID, const LSFString& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod = 0);
    ControllerClientStatus TransitionLampStateBooleanField(const LSFString& lampID, const LSFString& stateFieldName, const bool& value);
    ControllerClientStatus GetLampParametersField(const LSFString& lampID, const LSFString& stateFieldName);

    void LampNameChanged(LSFString& lampId, LSFString& lampName) {
        callback.LampNameChangedCB(lampId, lampName);
    }

    void LampStateChanged(LSFString& id, LampState& state) {
        callback.LampStateChangedCB(id, state);
    }

    void LampsFound(LSFStringList& idList) {
        callback.LampsFoundCB(idList);
    }

    void LampsLost(LSFStringList& idList) {
        callback.LampsLostCB(idList);
    }

    // method reply handlers
    void GetAllLampIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
        callback.GetAllLampIDsReplyCB(responseCode, idList);
    }

    void GetLampManufacturerReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& manufacturer) {
        callback.GetLampManufacturerReplyCB(responseCode, lsfId, language, manufacturer);
    }
    void GetLampNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name) {
        callback.GetLampNameReplyCB(responseCode, lsfId, language, name);
    }
    void GetLampSupportedLanguagesReply(ajn::Message& message);

    void SetLampNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
        callback.SetLampNameReplyCB(responseCode, lsfId, language);
    }

    void GetLampStateReply(ajn::Message& message);
    void GetLampStateFieldReply(ajn::Message& message);

    void ResetLampStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.ResetLampStateReplyCB(responseCode, lsfId);
    }

    void ResetLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName);

    void TransitionLampStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.TransitionLampStateReplyCB(responseCode, lsfId);
    }

    void PulseLampWithStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.PulseLampWithStateReplyCB(responseCode, lsfId);
    }

    void PulseLampWithPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.PulseLampWithPresetReplyCB(responseCode, lsfId);
    }

    void TransitionLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName);

    void TransitionLampStateToPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.TransitionLampStateToPresetReplyCB(responseCode, lsfId);
    }

    void GetLampFaultsReply(ajn::Message& message);
    void GetLampServiceVersionReply(ajn::Message& message);
    void ClearLampFaultReply(ajn::Message& message);
    void GetLampDetailsReply(ajn::Message& message);
    void GetLampParametersReply(ajn::Message& message);
    void GetLampParametersFieldReply(ajn::Message& message);
    LampManagerCallback&    callback;
};

}

#endif
