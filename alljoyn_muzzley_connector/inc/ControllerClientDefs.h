#ifndef _CONTROLLER_CLIENT_DEFS_H_
#define _CONTROLLER_CLIENT_DEFS_H_
/**
 * \ingroup ControllerClient
 */

/**
 *  \file  lighting_controller_client/inc/ControllerClientDefs.h
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

#include <LSFTypes.h>
#include <LSFResponseCodes.h>
#include <Mutex.h>

namespace lsf {

/**
 * Error Codes representing any internal errors in the Lighting
 * Controller Client. The Controller Client will invoke the
 * ControllerClientErrorCB with this error code to inform the
 * User Application about an internal error
 * TODO: Any Lighting Controller specific internal error should be
 * defined here
 */
typedef enum _ErrorCode {
    /**< No Error */
    ERROR_NONE = 0,
    /**< Error encountered when trying to register signal handlers */
    ERROR_REGISTERING_SIGNAL_HANDLERS = 1,
    /**< No active Controller Service found */
    ERROR_NO_ACTIVE_CONTROLLER_SERVICE_FOUND = 2,
    /**< AllJoyn Method Call Timeout */
    ERROR_ALLJOYN_METHOD_CALL_TIMEOUT = 3,
    /**< Irrecoverable error */
    ERROR_IRRECOVERABLE = 4,
    /**< Disconnected from the AllJoyn Bus */
    ERROR_DISCONNECTED_FROM_BUS = 5,
    /**< Controller Client is exiting */
    ERROR_CONTROLLER_CLIENT_EXITING = 6,
    /**< Received an AllJoyn message with invalid arguments */
    ERROR_MESSAGE_WITH_INVALID_ARGS = 7,
    /**< Last value */
    ERROR_LAST_VALUE = 8
} ErrorCode;
/**
 * List of enum error codes
 */
typedef std::list<ErrorCode> ErrorCodeList;
/**
 * convert error code enum type to string type
 */
const char* ControllerClientErrorText(ErrorCode errorCode);

/**
 * Status returned by an invoked Controller Client
 * API function call
 */
typedef enum _ControllerClientStatus {
    CONTROLLER_CLIENT_OK = 0,                 /**< Success */
    CONTROLLER_CLIENT_ERR_NOT_CONNECTED = 1,  /**< The Controller Client is not connected to any Controller Service */
    CONTROLLER_CLIENT_ERR_FAILURE = 2,        /**< The requested operation failed. Look at the error logs to understand the underlying issue */
    CONTROLLER_CLIENT_ERR_RETRY = 3,          /**< The requested operation should be re-tried */
    CONTROLLER_CLIENT_LAST_VALUE = 4          /**< Last value */
} ControllerClientStatus;

/**
 * Convert ControllerClientStatus enum type to string type
 */
const char* ControllerClientStatusText(ControllerClientStatus status);

} // namespace lsf

#endif
