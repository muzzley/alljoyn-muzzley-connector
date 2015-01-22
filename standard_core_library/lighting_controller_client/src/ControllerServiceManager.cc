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

#include <ControllerServiceManager.h>
#include <ControllerClient.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "CONTROLLER_SERVICE_MANAGER"

namespace lsf {

ControllerServiceManager::ControllerServiceManager(ControllerClient& controllerClient, ControllerServiceManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.controllerServiceManagerPtr = this;
}

ControllerClientStatus ControllerServiceManager::GetControllerServiceVersion(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithUint32Value(
               ControllerServiceInterfaceName,
               "GetControllerServiceVersion");
}

ControllerClientStatus ControllerServiceManager::LightingResetControllerService(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return controllerClient.MethodCallAsyncForReplyWithUint32Value(
               ControllerServiceInterfaceName,
               "LightingResetControllerService");
}

}
