#ifndef _CONTROLLER_SERVICE_MANAGER_H_
#define _CONTROLLER_SERVICE_MANAGER_H_
/**
 * \ingroup ControllerClient
 */

/**
 * \file  lighting_controller_client/inc/ControllerServiceManager.h
 * This file provides definitions for the Controller Service Manager
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

#include <Manager.h>
#include <ControllerClientDefs.h>

namespace lsf {

/**
 * Abstract base class implemented by User Application Developers.
 * The callbacks defined in this class allow the User Application
 * to be informed when Controller Service specific AllJoyn method
 * replies or signals are received from the Lighting Controller
 * Service.
 */
class ControllerServiceManagerCallback {
  public:
    /**
     * Destructor
     */
    virtual ~ControllerServiceManagerCallback() { }

    /**
     * Indicates that a reply has been received for the GetControllerServiceVersion method call
     *
     * @param version	Lighting Controller Service version
     */
    virtual void GetControllerServiceVersionReplyCB(const uint32_t& version) { }

    /**
     * Indicates that a reply has been received for the LightingResetControllerService method call
     *
     * @param responseCode	Response Code
     */
    virtual void LightingResetControllerServiceReplyCB(const LSFResponseCode& responseCode) { }

    /**
     * Indicates that a ControllerServiceLightingReset signal has been received
     */
    virtual void ControllerServiceLightingResetCB(void) { }

    /**
     * Indicates that a ControllerServiceNameChanged signal has been received
     */
    virtual void ControllerServiceNameChangedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) { }
};


class ControllerServiceManager : public Manager {

    friend class ControllerClient;

  public:

    /**
     * Constructor
     */
    ControllerServiceManager(ControllerClient& controller, ControllerServiceManagerCallback& callback);

    /**
     * Get the version of the Controller Service to which the Controller Client is currently connected to. \n
     * The response will come in ControllerServiceManagerCallback::GetControllerServiceVersionReplyCB.
     *
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     */
    ControllerClientStatus GetControllerServiceVersion(void);

    /**
     * Make the ControllerService perform a lighting reset. \n
     * The reply will come in the form of ControllerServiceManagerCallback::LightingResetControllerServiceReplyCB.
     *
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     */
    ControllerClientStatus LightingResetControllerService(void);

  private:

    /**
     * Handler for the signal ControllerServiceLightingReset
     */
    void ControllerServiceLightingReset(void) {
        callback.ControllerServiceLightingResetCB();
    }

    /**
     * Method Reply Handler for the signal GetControllerServiceVersion
     */
    void GetControllerServiceVersionReply(uint32_t& version) {
        callback.GetControllerServiceVersionReplyCB(version);
    }

    /**
     * Method Reply Handler for the signal LightingResetControllerService
     */
    void LightingResetControllerServiceReply(uint32_t& responseCode) {
        callback.LightingResetControllerServiceReplyCB((LSFResponseCode &)responseCode);
    }

    /**
     * Callback used to send Controller Service specific signals and method replies
     * to the User Application
     */
    ControllerServiceManagerCallback& callback;
};

} // namespace lsf

#endif
