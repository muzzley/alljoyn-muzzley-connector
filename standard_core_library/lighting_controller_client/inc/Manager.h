#ifndef _MANAGER_H_
#define _MANAGER_H_
/**
 * \ingroup ControllerClient
 */
/**
 * \file  lighting_controller_client/inc/Manager.h
 * This file provides definitions for manager type of classes
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

#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/MessageReceiver.h>

namespace lsf {

class ControllerClient;
/**
 * Manager base class for managers
 */
class Manager : public ajn::MessageReceiver {

  protected:
    /**
     * manager constructor
     */
    Manager(ControllerClient& controllerClient);
    /**
     * controller client reference
     */
    ControllerClient& controllerClient;

  private:

    Manager();
    Manager(const Manager& other);
    Manager& operator=(const Manager& other);
};

}

#endif
