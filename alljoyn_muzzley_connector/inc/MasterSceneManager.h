#ifndef _MASTER_SCENE_MANAGER_H_
#define _MASTER_SCENE_MANAGER_H_
/**
 * \ingroup ControllerClient
 */
/**
 * \file  lighting_controller_client/inc/MasterSceneManager.h
 * This file provides definitions for controller client
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

#include <list>

#include <Manager.h>
#include <ControllerClientDefs.h>

namespace lsf {

class ControllerClient;
/**
 * a callback class that its instance and members are given to the MasterScene to get the methods reply and signals that are coming from the controller service.
 */
class MasterSceneManagerCallback {
  public:
    virtual ~MasterSceneManagerCallback() { }

    /**
     * Response to MasterSceneManager::GetAllMasterSceneIDs
     *
     * @param responseCode        The response code
     * @param masterSceneList    The masterScene ID's
     */
    virtual void GetAllMasterSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& masterSceneList) { }

    /**
     * Response to MasterSceneManager::GetMasterSceneName
     *
     * @param responseCode    The response code
     * @param masterSceneID    The masterScene id
     * @param language
     * @param masterSceneName  The masterScene masterSceneName
     */
    virtual void GetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language, const LSFString& masterSceneName) { }

    /**
     * Response to MasterSceneManager::SetMasterSceneName
     *
     * @param responseCode    The response code
     * @param masterSceneID    The Lamp masterScene id
     * @param language
     */
    virtual void SetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language) { }

    /**
     * A masterScene has had its masterSceneName set
     *
     * @param masterSceneIDs    The masterScene id
     */
    virtual void MasterScenesNameChangedCB(const LSFStringList& masterSceneIDs) { }

    /**
     * Response to MasterSceneManager::CreateMasterScene
     *
     * @param responseCode    The response code
     * @param masterSceneID    The Lamp masterScene id
     * @param masterSceneID The Lamp masterScene
     */
    virtual void CreateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) { }

    /**
     *  A masterScene has been created
     *
     *  @param masterSceneIDs   The masterScene id
     */
    virtual void MasterScenesCreatedCB(const LSFStringList& masterSceneIDs) { }

    /**
     * Response to MasterSceneManager::GetMasterScene
     *
     * @param responseCode    The response code
     * @param masterSceneID    The Lamp masterScene id
     * @param masterScene The GroupID
     */
    virtual void GetMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const MasterScene& masterScene) { }

    /**
     * Response to MasterSceneManager::DeleteMasterScene
     *
     * @param responseCode    The response code
     * @param masterSceneID    The Lamp masterScene id
     */
    virtual void DeleteMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) { }

    /**
     *  A masterScene has been deleted
     *
     *  @param masterSceneIDs   The masterScene id
     */
    virtual void MasterScenesDeletedCB(const LSFStringList& masterSceneIDs) { }

    /**
     * Response to MasterSceneManager::UpdateMasterScene
     *
     * @param responseCode    The response code
     * @param masterSceneID    The Lamp masterScene id
     */
    virtual void UpdateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) { }

    /**
     * A Lamp masterScene has been updated
     *
     * @param masterSceneIDs    The id of the Lamp masterScene
     */
    virtual void MasterScenesUpdatedCB(const LSFStringList& masterSceneIDs) { }

    /**
     * Response to MasterSceneManager::ApplyMasterScene
     *
     * @param responseCode    The response code
     * @param masterSceneID    The id of the scene masterScene
     */
    virtual void ApplyMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) { }

    /**
     * A scene masterScene has been applied
     *
     * @param masterSceneIDs    The id of the scene masterScene
     */
    virtual void MasterScenesAppliedCB(const LSFStringList& masterSceneIDs) { }
};

/**
 * Master Scene Manager
 */
class MasterSceneManager : public Manager {

    friend class ControllerClient;

  public:
    /**
     * MasterSceneManager constructor
     */
    MasterSceneManager(ControllerClient& controller, MasterSceneManagerCallback& callback);

    /**
     * Get the IDs of all masterSceneList. \n
     * Response in MasterSceneManagerCallback::GetAllMasterSceneIDsReplyCB
     */
    ControllerClientStatus GetAllMasterSceneIDs(void);

    /**
     * Get the names of the masterScene. \n
     * Response in MasterSceneManagerCallback::GetMasterSceneNameCB
     *
     * @param masterSceneID    The masterScene id
     * @param language
     */
    ControllerClientStatus GetMasterSceneName(const LSFString& masterSceneID, const LSFString& language = LSFString("en"));

    /**
     * Set the masterSceneName of the specified masterScene. \n
     * Response in MasterSceneManagerCallback::SetMasterSceneNameReplyCB
     *
     * @param masterSceneID    The id of the masterScene
     * @param masterSceneName  The Master Scene Name
     * @param language
     */
    ControllerClientStatus SetMasterSceneName(const LSFString& masterSceneID, const LSFString& masterSceneName, const LSFString& language = LSFString("en"));

    /**
     * Create a new Scene masterScene. \n
     * Response in MasterSceneManagerCallback::CreateMasterSceneReplyCB
     *
     */
    ControllerClientStatus CreateMasterScene(const MasterScene& masterScene, const LSFString& masterSceneName, const LSFString& language = LSFString("en"));

    /**
     * Modify a masterScene. \n
     * Change master scene to another master scene. \n
     * Response in MasterSceneManagerCallback::UpdateMasterSceneReplyCB
     *
     * @param masterSceneID    The id of the masterScene to modify
     * @param masterScene
     */
    ControllerClientStatus UpdateMasterScene(const LSFString& masterSceneID, const MasterScene& masterScene);

    /**
     * Get the information about the masterScene. \n
     * Response in MasterSceneManagerCallback::GetMasterSceneReplyCB
     *
     * @param masterSceneID    Group id to get
     */
    ControllerClientStatus GetMasterScene(const LSFString& masterSceneID);

    /**
     * Delete a Lamp masterScene. \n
     * Response in MasterSceneManagerCallback::DeleteMasterSceneReplyCB
     *
     * @param masterSceneID    The id of the masterScene to delete
     */
    ControllerClientStatus DeleteMasterScene(const LSFString& masterSceneID);

    /**
     * Apply a scene masterScene. \n
     * Make the master scene happen. \n
     * Response in MasterSceneManagerCallback::ApplyMasterSceneReplyCB
     *
     * @param masterSceneID    The ID of the scene to apply
     */
    ControllerClientStatus ApplyMasterScene(const LSFString& masterSceneID);

    /**
     * Get the Master Scene Info and Name
     *
     * @param masterSceneID    The ID of the master scene
     * @param language
     */
    ControllerClientStatus GetMasterSceneDataSet(const LSFString& masterSceneID, const LSFString& language = LSFString("en"));

  private:

    // signal handlers
    void MasterScenesNameChanged(LSFStringList& idList) {
        callback.MasterScenesNameChangedCB(idList);
    }

    void MasterScenesCreated(LSFStringList& idList) {
        callback.MasterScenesCreatedCB(idList);
    }

    void MasterScenesDeleted(LSFStringList& idList) {
        callback.MasterScenesDeletedCB(idList);
    }

    void MasterScenesUpdated(LSFStringList& idList) {
        callback.MasterScenesUpdatedCB(idList);
    }

    void MasterScenesApplied(LSFStringList& idList) {
        callback.MasterScenesAppliedCB(idList);
    }

    // method response handlers
    void GetAllMasterSceneIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
        callback.GetAllMasterSceneIDsReplyCB(responseCode, idList);
    }

    void GetMasterSceneNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& lsfName) {
        callback.GetMasterSceneNameReplyCB(responseCode, lsfId, language, lsfName);
    }

    void ApplyMasterSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.ApplyMasterSceneReplyCB(responseCode, lsfId);
    }

    void SetMasterSceneNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
        callback.SetMasterSceneNameReplyCB(responseCode, lsfId, language);
    }

    void CreateMasterSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.CreateMasterSceneReplyCB(responseCode, lsfId);
    }

    void UpdateMasterSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.UpdateMasterSceneReplyCB(responseCode, lsfId);
    }

    void GetMasterSceneReply(ajn::Message& message);

    void DeleteMasterSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.DeleteMasterSceneReplyCB(responseCode, lsfId);
    }

    MasterSceneManagerCallback&   callback;
};


}

#endif
