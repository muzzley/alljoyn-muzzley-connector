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

#include <ControllerClientDefs.h>
#include <LSFTypes.h>

namespace lsf {

const char* ControllerClientErrorText(ErrorCode errorCode)
{
    switch (errorCode) {
        LSF_CASE(ERROR_NONE);
        LSF_CASE(ERROR_REGISTERING_SIGNAL_HANDLERS);
        LSF_CASE(ERROR_NO_ACTIVE_CONTROLLER_SERVICE_FOUND);
        LSF_CASE(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
        LSF_CASE(ERROR_IRRECOVERABLE);
        LSF_CASE(ERROR_DISCONNECTED_FROM_BUS);
        LSF_CASE(ERROR_CONTROLLER_CLIENT_EXITING);
        LSF_CASE(ERROR_MESSAGE_WITH_INVALID_ARGS);
        LSF_CASE(ERROR_LAST_VALUE);

    default:
        return "<unknown>";
    }
}

const char* ControllerClientStatusText(ControllerClientStatus status)
{
    switch (status) {
        LSF_CASE(CONTROLLER_CLIENT_OK);
        LSF_CASE(CONTROLLER_CLIENT_ERR_NOT_CONNECTED);
        LSF_CASE(CONTROLLER_CLIENT_ERR_FAILURE);
        LSF_CASE(CONTROLLER_CLIENT_ERR_RETRY);
        LSF_CASE(CONTROLLER_CLIENT_LAST_VALUE);

    default:
        return "<unknown>";
    }
}

}
