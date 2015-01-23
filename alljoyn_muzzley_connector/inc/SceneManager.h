#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
/**
 * \ingroup ControllerClient
 */
/**
 * \file lighting_controller_client/inc/SceneManager.h
 * This file provides definitions for scene manager
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
#include <LSFResponseCodes.h>
#include <ControllerClientDefs.h>
#include <list>

namespace lsf {

class ControllerClient;

/**
 * scene callback class
 */
class SceneManagerCallback {
  public:
    virtual ~SceneManagerCallback() { }

    /**
     * Response to SceneManager::GetAllSceneIds.
     *
     * @param responseCode    The response code
     * @param sceneIDs   The list of scene ID's
     */
    virtual void GetAllSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& sceneIDs) { }

    /**
     * Response to SceneManager::GetSceneName.
     *
     * @param responseCode    The response code: \n
     *   LSF_OK - operation succeeded \n
     *   LSF_ERR_NOT_FOUND  - the scene not found \n
     *   LSF_ERR_INVALID_ARGS - Language not supported \n
     * @param sceneID    The id of the scene
     * @param language   The language of the scene name
     * @param sceneName  The name of the scene
     */
    virtual void GetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language, const LSFString& sceneName) { }

    /**
     * Response to SceneManager::SetSceneName.
     *
     * @param responseCode    The response code: \n
     *   LSF_OK - operation succeeded \n
     *   LSF_ERR_INVALID_ARGS - Language not supported, length exceeds LSF_MAX_NAME_LENGTH \n
     *   LSF_ERR_EMPTY_NAME - scene name is empty \n
     *   LSF_ERR_RESOURCES - blob length is longer than MAX_FILE_LEN \n
     * @param sceneID    The id of the scene whose name was changed
     * @param language   language of the scene
     */
    virtual void SetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language) { }

    /**
     * This signal is fired any time a scene's name is changed.
     *
     * @param sceneIDs    The id of the scene whose name changed
     */
    virtual void ScenesNameChangedCB(const LSFStringList& sceneIDs) { }

    /**
     * Response to SceneManager::SetSceneName.
     *
     * @param responseCode    The response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_INVALID_ARGS - Language not supported, scene name is empty, Invalid Scene components specified, ame length exceeds \n
     *  LSF_ERR_RESOURCES - Could not allocate memory \n
     *  LSF_ERR_NO_SLOT - No slot for new Scene \n
     * @param sceneID    The id of the new Scene
     */
    virtual void CreateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) { }

    /**
     *  This signal is fired any time a scene is been created.
     *
     * @param sceneIDs    The id of the new scene
     */
    virtual void ScenesCreatedCB(const LSFStringList& sceneIDs) { }

    /**
     * Response to SceneManager::UpdateScene.
     *
     * @param responseCode    The response code
     * @param sceneID    The id of the scene that was updated
     */
    virtual void UpdateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) { }

    /**
     * This signal is fired any time a scene has been updated.
     *
     * @param sceneIDs    The id of the updated scene
     */
    virtual void ScenesUpdatedCB(const LSFStringList& sceneIDs) { }

    /**
     * Response to SceneManager::DeleteScene.
     *
     * @param responseCode    The response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_NOT_FOUND - can't find scene id \n
     * @param sceneID    The id of the deleted scene
     */
    virtual void DeleteSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) { }

    /**
     * This signal is fired any time a scene has been deleted.
     *
     * @param sceneIDs    The id of the deleted scene
     */
    virtual void ScenesDeletedCB(const LSFStringList& sceneIDs) { }

    /**
     * Response to SceneManager::GetScene.
     *
     * @param responseCode    The response code: \n
     *  return LSF_OK \n
     *  return LSF_ERR_NOT_FOUND - scene not found \n
     * @param sceneID    The id of the scene
     * @param data  The scene data
     */
    virtual void GetSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const Scene& data) { }

    /**
     * Response to SceneManager::ApplyScene.
     *
     * @param responseCode    The response code \n
     *  LSF_OK - on success \n
     *  LSF_ERR_NOT_FOUND - scene id not found in current list of scenes
     * @param sceneID    The id of the scene
     */
    virtual void ApplySceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) { }

    /**
     * This signal is fired any time a scene has been applied.
     *
     * @param sceneIDs    The id of the scene
     */
    virtual void ScenesAppliedCB(const LSFStringList& sceneIDs) { }
};


/**
 * scene management class
 */
class SceneManager : public Manager {

    friend class ControllerClient;

  public:
    /**
     * SceneManager CTOR.
     * @param controller - a reference to ControllerClient instance
     * @param callback - a reference to SceneManagerCallback instance, to get the callback messages
     */
    SceneManager(ControllerClient& controller, SceneManagerCallback& callback);

    /**
     * Get the IDs of all available scenes. \n
     * Response comes in SceneManagerCallback::GetAllSceneIDsReplyCB
     */
    ControllerClientStatus GetAllSceneIDs(void);

    /**
     * Get the name of the given Scene. \n
     * Response in SceneManagerCallback::GetSceneNameReplyCB
     *
     * @param sceneID    The id of the scene
     * @param language   The requested language
     */
    ControllerClientStatus GetSceneName(const LSFString& sceneID, const LSFString& language = LSFString("en"));

    /**
     * Set the name of a Scene. \n
     * Response in SceneManagerCallback::SetSceneNameReplyCB
     *
     * @param sceneID    The id of the scene to modify
     * @param sceneName  The new scene name
     * @param language   The requested language
     */
    ControllerClientStatus SetSceneName(const LSFString& sceneID, const LSFString& sceneName, const LSFString& language = LSFString("en"));

    /**
     *  Create a new Scene. \n
     *  Response in SceneManagerCallback::CreateSceneReplyCB
     *
     * @param scene The scene data
     * @param sceneName  The scene name
     * @param language   The scene language
     */
    ControllerClientStatus CreateScene(const Scene& scene, const LSFString& sceneName, const LSFString& language = LSFString("en"));

    /**
     * Modify an existing scene. \n
     * Response in SceneManagerCallback::UpdateSceneReplyCB \n
     *
     * @param sceneID    The id of the scene to modify
     * @param scene      The scene to modify with
     */
    ControllerClientStatus UpdateScene(const LSFString& sceneID, const Scene& scene);

    /**
     * Delete an existing scene. \n
     * Response in SceneManagerCallback::DeleteSceneReplyCB
     *
     * @param sceneID    The id of the scene to delete
     */
    ControllerClientStatus DeleteScene(const LSFString& sceneID);

    /**
     * Get the information about the specified scene. \n
     * Response in SceneManagerCallback::GetSceneReplyCB
     *
     * @param sceneID    The id of the scene to find
     */
    ControllerClientStatus GetScene(const LSFString& sceneID);

    /**
     * Apply a scene. \n
     * Activate an already created scene. Make it happen. \n
     * Response in SceneManagerCallback::ApplySceneReplyCB
     *
     * @param sceneID    The ID of the scene to apply
     */
    ControllerClientStatus ApplyScene(const LSFString& sceneID);

    /**
     * Get the Scene Info and Name. \n
     * Combination of GetScene and GetScneneName. Responses are accordingly. \n
     *
     * @param sceneID    The ID of the master scene
     * @param language   The requested language
     */
    ControllerClientStatus GetSceneDataSet(const LSFString& sceneID, const LSFString& language = LSFString("en"));

  private:

    // Signal handlers:
    void ScenesNameChanged(LSFStringList& idList) {
        callback.ScenesNameChangedCB(idList);
    }

    void ScenesCreated(LSFStringList& idList) {
        callback.ScenesCreatedCB(idList);
    }

    void ScenesUpdated(LSFStringList& idList) {
        callback.ScenesUpdatedCB(idList);
    }

    void ScenesDeleted(LSFStringList& idList) {
        callback.ScenesDeletedCB(idList);
    }

    void ScenesApplied(LSFStringList& idList) {
        callback.ScenesAppliedCB(idList);
    }

    // asynch method response handlers
    void GetAllSceneIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
        callback.GetAllSceneIDsReplyCB(responseCode, idList);
    }

    void GetSceneReply(ajn::Message& message);

    void ApplySceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.ApplySceneReplyCB(responseCode, lsfId);
    }

    void DeleteSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.DeleteSceneReplyCB(responseCode, lsfId);
    }

    void SetSceneNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
        callback.SetSceneNameReplyCB(responseCode, lsfId, language);
    }

    void GetSceneNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& lsfName) {
        callback.GetSceneNameReplyCB(responseCode, lsfId, language, lsfName);
    }

    void CreateSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.CreateSceneReplyCB(responseCode, lsfId);
    }

    void UpdateSceneReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.UpdateSceneReplyCB(responseCode, lsfId);
    }

    SceneManagerCallback& callback;
};


}

#endif
