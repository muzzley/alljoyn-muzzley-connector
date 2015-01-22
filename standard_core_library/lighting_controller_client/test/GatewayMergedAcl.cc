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

#include "alljoyn/gateway/GatewayMergedAcl.h"

using namespace ajn;
using namespace gw;
using namespace std;


QStatus GatewayMergedAcl::unmarshal(Message& msg)
{
    QStatus status = ER_OK;

    //exposed services
    MsgArg* exposedServiceArgs;
    size_t numExposedServiceArgs;
    status = msg->GetArg(0)->Get("a(obas)", &numExposedServiceArgs, &exposedServiceArgs);
    if (ER_OK != status) {
        return status;
    }
    status = unmarshalObjectDescriptions(exposedServiceArgs, numExposedServiceArgs, m_ExposedServices);
    if (ER_OK != status) {
        return status;
    }

    //remoted apps
    MsgArg* remotedAppArgs;
    size_t numRemotedAppArgs;
    status = msg->GetArg(1)->Get("a(saya(obas))", &numRemotedAppArgs, &remotedAppArgs);
    if (ER_OK != status) {
        return status;
    }
    for (size_t i = 0; i < numRemotedAppArgs; i++) {
        char* deviceIdArg;

        uint8_t* appIdArg;
        size_t appIdArgLen;

        MsgArg* objDescArgs;
        size_t numObjDescArgs;

        status = remotedAppArgs[i].Get("(saya(obas))", &deviceIdArg, &appIdArgLen, &appIdArg, &numObjDescArgs, &objDescArgs);
        if (status != ER_OK) {
            return status;
        }

        RemotedApp remotedApp;
        remotedApp.deviceId.assign(deviceIdArg);
        memcpy(remotedApp.appId, appIdArg, 16);
        status = unmarshalObjectDescriptions(objDescArgs, numObjDescArgs, remotedApp.objectDescs);
        if (status != ER_OK) {
            return status;
        }
        m_RemotedApps.push_back(remotedApp);
    }

    return status;
}

QStatus GatewayMergedAcl::unmarshalObjectDescriptions(MsgArg* objDescArgs, size_t numObjDescs, std::list<ObjectDescription>& dest)
{
    QStatus status = ER_OK;

    for (size_t i = 0; i < numObjDescs; i++) {
        ObjectDescription objDesc;

        char* objArg;
        bool isPrefix;
        MsgArg* ifcArgs;
        size_t numIfcArgs;
        status = (objDescArgs)[i].Get("(obas)", &objArg, &isPrefix, &numIfcArgs, &ifcArgs);
        if (ER_OK != status) {
            return status;
        }

        objDesc.objectPath.assign(objArg);
        objDesc.isPrefix = isPrefix;

        for (size_t j = 0; j < numIfcArgs; j++) {
            char* ifcArg;
            status = ifcArgs[j].Get("s", &ifcArg);
            if (ER_OK != status) {
                return status;
            }
            objDesc.interfaces.push_back(qcc::String(ifcArg));
        }

        dest.push_back(objDesc);
    }

    return status;
}
