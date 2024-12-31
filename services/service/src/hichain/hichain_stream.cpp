/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "AudioStreamCollector"
#endif

#include "audio_stream_collector.h"

#include "audio_client_tracker_callback_proxy.h"
#include "audio_spatialization_service.h"
#include "audio_utils.h"

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr uint32_t THP_EXTRA_SA_UID = 5000;

const map<pair<ContentType, StreamUsage>, AudioStreamType> AudioStreamCollector::streamTypeMap_ =
    AudioStreamCollector::CreateStreamMap();
const std::unordered_map<std::string, uint8_t> EFFECT_CHAIN_TYPE_MAP {
    {"UNKNOWN", 0},
    {"NONE", 1},
    {"SCENE_OTHERS", 2},
    {"SCENE_MUSIC", 3},
    {"SCENE_MOVIE", 4},
    {"SCENE_GAME", 5},
    {"SCENE_SPEECH", 6},
    {"SCENE_RING", 7},
    {"SCENE_VOIP_DOWN", 8}
};

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioStreamCollector::CreateStreamMap()
{
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    // Only use stream usage to choose stream type
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VIDEO_COMMUNICATION)] = STREAM_VOICE_COMMUNICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_RINGTONE)] = STREAM_VOICE_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_CALL_ASSISTANT)] = STREAM_VOICE_CALL_ASSISTANT;

    return streamMap;
}

AudioStreamCollector::AudioStreamCollector() : audioSystemMgr_
    (AudioSystemManager::GetInstance())
{
    audioPolicyServerHandler_ = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    audioConcurrencyService_ = std::make_shared<AudioConcurrencyService>();
    audioPolicyServerHandler_->AddConcurrencyEventDispatcher(audioConcurrencyService_);
    audioConcurrencyService_->Init();
    audioConcurrencyService_->SetCallbackHandler(audioPolicyServerHandler_);
    AUDIO_INFO_LOG("AudioStreamCollector()");
}

AudioStreamCollector::~AudioStreamCollector()
{
    AUDIO_INFO_LOG("~AudioStreamCollector()");
}

int32_t AudioStreamCollector::AddRendererStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("Add playback client uid %{public}d sessionId %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.sessionId);

    rendererStatequeue_.insert({{streamChangeInfo.audioRendererChangeInfo.clientUID,
        streamChangeInfo.audioRendererChangeInfo.sessionId},
        streamChangeInfo.audioRendererChangeInfo.rendererState});

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    if (!rendererChangeInfo) {
        AUDIO_ERR_LOG("AddRendererStream Memory Allocation Failed");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
    rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
    rendererChangeInfo->sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    rendererChangeInfo->callerPid = streamChangeInfo.audioRendererChangeInfo.callerPid;
    rendererChangeInfo->tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    rendererChangeInfo->rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    rendererChangeInfo->rendererInfo = streamChangeInfo.audioRendererChangeInfo.rendererInfo;
    rendererChangeInfo->outputDeviceInfo = streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo;
    rendererChangeInfo->channelCount = streamChangeInfo.audioRendererChangeInfo.channelCount;
    audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "audioPolicyServerHandler_ is nullptr, callback error");
    audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);
    return SUCCESS;
}

void AudioStreamCollector::GetRendererStreamInfo(AudioStreamChangeInfo &streamChangeInfo,
    AudioRendererChangeInfo &rendererInfo)
{
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID &&
            (*it)->sessionId == streamChangeInfo.audioRendererChangeInfo.sessionId) {
            rendererInfo.outputDeviceInfo = (*it)->outputDeviceInfo;
            return;
        }
    }
}

void AudioStreamCollector::GetCapturerStreamInfo(AudioStreamChangeInfo &streamChangeInfo,
    AudioCapturerChangeInfo &capturerInfo)
{
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == streamChangeInfo.audioCapturerChangeInfo.clientUID &&
            (*it)->sessionId == streamChangeInfo.audioCapturerChangeInfo.sessionId) {
            capturerInfo.inputDeviceInfo = (*it)->inputDeviceInfo;
            return;
        }
    }
}

int32_t AudioStreamCollector::GetPipeType(const int32_t sessionId, AudioPipeType &pipeType)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it == audioRendererChangeInfos_.end()) {
        AUDIO_WARNING_LOG("invalid session id: %{public}d", sessionId);
        return ERROR;
    }

    pipeType = (*it)->rendererInfo.pipeType;
    return SUCCESS;
}

bool AudioStreamCollector::ExistStreamForPipe(AudioPipeType pipeType)
{
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&pipeType](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->rendererInfo.pipeType == pipeType;
        });
    if (it == audioRendererChangeInfos_.end()) {
        return false;
    }
    return true;
}

int32_t AudioStreamCollector::GetRendererDeviceInfo(const int32_t sessionId, AudioDeviceDescriptor &outputDeviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it == audioRendererChangeInfos_.end()) {
        AUDIO_WARNING_LOG("invalid session id: %{public}d", sessionId);
        return ERROR;
    }
    outputDeviceInfo = (*it)->outputDeviceInfo;
    return SUCCESS;
}

int32_t AudioStreamCollector::AddCapturerStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("Add recording client uid %{public}d sessionId %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.sessionId);

    capturerStatequeue_.insert({{streamChangeInfo.audioCapturerChangeInfo.clientUID,
        streamChangeInfo.audioCapturerChangeInfo.sessionId},
        streamChangeInfo.audioCapturerChangeInfo.capturerState});

    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    if (!capturerChangeInfo) {
        AUDIO_ERR_LOG("AddCapturerStream Memory Allocation Failed");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    capturerChangeInfo->createrUID = streamChangeInfo.audioCapturerChangeInfo.createrUID;
    capturerChangeInfo->clientUID = streamChangeInfo.audioCapturerChangeInfo.clientUID;
    capturerChangeInfo->sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    capturerChangeInfo->callerPid = streamChangeInfo.audioCapturerChangeInfo.callerPid;
    capturerChangeInfo->muted = streamChangeInfo.audioCapturerChangeInfo.muted;
    capturerChangeInfo->appTokenId = streamChangeInfo.audioCapturerChangeInfo.appTokenId;

    capturerChangeInfo->capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    capturerChangeInfo->capturerInfo = streamChangeInfo.audioCapturerChangeInfo.capturerInfo;
    capturerChangeInfo->inputDeviceInfo = streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo;
    audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));

    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERR_MEMORY_ALLOC_FAILED,
        "audioPolicyServerHandler_ is nullptr, callback error");
    SendCapturerInfoEvent(audioCapturerChangeInfos_);
    return SUCCESS;
}

void AudioStreamCollector::SendCapturerInfoEvent(const std::vector<std::shared_ptr<AudioCapturerChangeInfo>>
    &audioCapturerChangeInfos)
{
    bool earseFlag = false;
    for (const auto &capChangeinfoUPtr : audioCapturerChangeInfos) {
        if (IsTransparentCapture(capChangeinfoUPtr->clientUID)) {
            earseFlag = true;
            break;
        }
    }
    if (earseFlag == false) {
        if (!audioCapturerChangeInfos.empty()) {
            audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfos);
        }
        return;
    }

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfoSent;
    for (const auto &capChangeinfoUPtr : audioCapturerChangeInfos) {
        if (IsTransparentCapture(capChangeinfoUPtr->clientUID)) {
            AUDIO_INFO_LOG("bypass uid:%{public}d", capChangeinfoUPtr->clientUID);
        } else {
            audioCapturerChangeInfoSent.push_back(make_shared<AudioCapturerChangeInfo>(*capChangeinfoUPtr));
        }
    }
    if (audioCapturerChangeInfoSent.empty()) {
        return;
    }
    audioPolicyServerHandler_->SendCapturerInfoEvent(audioCapturerChangeInfoSent);
}

bool AudioStreamCollector::IsTransparentCapture(const uint32_t clientUid)
{
    if (clientUid == THP_EXTRA_SA_UID) {
        return true;
    }
    return false;
}

int32_t AudioStreamCollector::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object)
{
    AUDIO_DEBUG_LOG("RegisterTracker mode %{public}d", mode);

    int32_t clientId;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    if (mode == AUDIO_MODE_PLAYBACK) {
        AddRendererStream(streamChangeInfo);
        clientId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    } else {
        // mode = AUDIO_MODE_RECORD
        AddCapturerStream(streamChangeInfo);
        clientId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    }

    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr,
        ERR_INVALID_PARAM, "AudioStreamCollector: client tracker obj cast failed");
    std::shared_ptr<AudioClientTracker> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ERR_INVALID_PARAM, "AudioStreamCollector: failed to create tracker cb obj");
    clientTracker_[clientId] = callback;
    WriterStreamChangeSysEvent(mode, streamChangeInfo);
    return SUCCESS;
}

void AudioStreamCollector::SetRendererStreamParam(AudioStreamChangeInfo &streamChangeInfo,
    shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo)
{
    rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
    rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
    rendererChangeInfo->sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    rendererChangeInfo->callerPid = streamChangeInfo.audioRendererChangeInfo.callerPid;
    rendererChangeInfo->clientPid = streamChangeInfo.audioRendererChangeInfo.clientPid;
    rendererChangeInfo->tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    rendererChangeInfo->rendererState = streamChangeInfo.audioRendererChangeInfo.rendererState;
    rendererChangeInfo->rendererInfo = streamChangeInfo.audioRendererChangeInfo.rendererInfo;
    rendererChangeInfo->outputDeviceInfo = streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo;
    rendererChangeInfo->prerunningState = streamChangeInfo.audioRendererChangeInfo.prerunningState;
}

void AudioStreamCollector::SetCapturerStreamParam(AudioStreamChangeInfo &streamChangeInfo,
    shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo)
{
    capturerChangeInfo->createrUID = streamChangeInfo.audioCapturerChangeInfo.createrUID;
    capturerChangeInfo->clientUID = streamChangeInfo.audioCapturerChangeInfo.clientUID;
    capturerChangeInfo->sessionId = streamChangeInfo.audioCapturerChangeInfo.sessionId;
    capturerChangeInfo->callerPid = streamChangeInfo.audioCapturerChangeInfo.callerPid;
    capturerChangeInfo->clientPid = streamChangeInfo.audioCapturerChangeInfo.clientPid;
    capturerChangeInfo->muted = streamChangeInfo.audioCapturerChangeInfo.muted;
    capturerChangeInfo->capturerState = streamChangeInfo.audioCapturerChangeInfo.capturerState;
    capturerChangeInfo->capturerInfo = streamChangeInfo.audioCapturerChangeInfo.capturerInfo;
    capturerChangeInfo->inputDeviceInfo = streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo;
}

void AudioStreamCollector::ResetRendererStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc)
{
    AUDIO_INFO_LOG("ResetRendererStreamDeviceInfo, deviceType:[%{public}d]", updatedDesc.deviceType_);
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->outputDeviceInfo.deviceType_ == updatedDesc.deviceType_ &&
            (*it)->outputDeviceInfo.macAddress_ == updatedDesc.macAddress_ &&
            (*it)->outputDeviceInfo.networkId_ == updatedDesc.networkId_ &&
            (*it)->rendererState != RENDERER_RUNNING) {
            (*it)->outputDeviceInfo.deviceType_ = DEVICE_TYPE_NONE;
            (*it)->outputDeviceInfo.macAddress_ = "";
            (*it)->outputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
        }
    }
}

void AudioStreamCollector::ResetCapturerStreamDeviceInfo(const AudioDeviceDescriptor& updatedDesc)
{
    AUDIO_INFO_LOG("ResetCapturerStreamDeviceInfo, deviceType:[%{public}d]", updatedDesc.deviceType_);
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->inputDeviceInfo.deviceType_ == updatedDesc.deviceType_ &&
            (*it)->inputDeviceInfo.macAddress_ == updatedDesc.macAddress_ &&
            (*it)->inputDeviceInfo.networkId_ == updatedDesc.networkId_ &&
            (*it)->capturerState != CAPTURER_RUNNING) {
            (*it)->inputDeviceInfo.deviceType_ = DEVICE_TYPE_NONE;
            (*it)->inputDeviceInfo.macAddress_ = "";
            (*it)->inputDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
        }
    }
}

bool AudioStreamCollector::CheckRendererStateInfoChanged(AudioStreamChangeInfo &streamChangeInfo)
{
    if (rendererStatequeue_.find(make_pair(streamChangeInfo.audioRendererChangeInfo.clientUID,
        streamChangeInfo.audioRendererChangeInfo.sessionId)) != rendererStatequeue_.end()) {
        if (streamChangeInfo.audioRendererChangeInfo.rendererState ==
            rendererStatequeue_[make_pair(streamChangeInfo.audioRendererChangeInfo.clientUID,
                streamChangeInfo.audioRendererChangeInfo.sessionId)]) {
            // Renderer state not changed
            return false;
        }
    } else {
        AUDIO_INFO_LOG("client %{public}d not found ", streamChangeInfo.audioRendererChangeInfo.clientUID);
    }
    return true;
}

bool AudioStreamCollector::CheckRendererInfoChanged(AudioStreamChangeInfo &streamChangeInfo)
{
    int32_t sessionId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it == audioRendererChangeInfos_.end()) {
        return true;
    }

    bool changed = false;
    bool isOffloadAllowed = (*it)->rendererInfo.isOffloadAllowed;
    if (isOffloadAllowed != streamChangeInfo.audioRendererChangeInfo.rendererInfo.isOffloadAllowed) {
        changed = true;
    }
    AudioPipeType pipeType = (*it)->rendererInfo.pipeType;
    if (pipeType != streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType) {
        changed = true;
    }
    return changed;
}

int32_t AudioStreamCollector::UpdateRendererStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("UpdateRendererStream client %{public}d state %{public}d session %{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.rendererState,
        streamChangeInfo.audioRendererChangeInfo.sessionId);
    bool stateChanged = CheckRendererStateInfoChanged(streamChangeInfo);
    bool infoChanged = CheckRendererInfoChanged(streamChangeInfo);
    CHECK_AND_RETURN_RET(stateChanged || infoChanged, SUCCESS);

    // Update the renderer info in audioRendererChangeInfos_
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfo = **it;
        if (audioRendererChangeInfo.clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID &&
            audioRendererChangeInfo.sessionId == streamChangeInfo.audioRendererChangeInfo.sessionId) {
            rendererStatequeue_[make_pair(audioRendererChangeInfo.clientUID, audioRendererChangeInfo.sessionId)] =
                streamChangeInfo.audioRendererChangeInfo.rendererState;
            streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType = (*it)->rendererInfo.pipeType;
            AUDIO_DEBUG_LOG("update client %{public}d session %{public}d", audioRendererChangeInfo.clientUID,
                audioRendererChangeInfo.sessionId);
            shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
            CHECK_AND_RETURN_RET_LOG(rendererChangeInfo != nullptr, ERR_MEMORY_ALLOC_FAILED,
                "Memory Allocation Failed");
            SetRendererStreamParam(streamChangeInfo, rendererChangeInfo);
            rendererChangeInfo->channelCount = (*it)->channelCount;
            rendererChangeInfo->backMute = (*it)->backMute;
            if (rendererChangeInfo->outputDeviceInfo.deviceType_ == DEVICE_TYPE_INVALID) {
                streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo = (*it)->outputDeviceInfo;
                rendererChangeInfo->outputDeviceInfo = (*it)->outputDeviceInfo;
            }
            *it = move(rendererChangeInfo);

            if (audioPolicyServerHandler_ != nullptr && stateChanged) {
                audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
            }
            AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);

            if (streamChangeInfo.audioRendererChangeInfo.rendererState == RENDERER_RELEASED) {
                audioRendererChangeInfos_.erase(it);
                rendererStatequeue_.erase(make_pair(audioRendererChangeInfo.clientUID,
                    audioRendererChangeInfo.sessionId));
                clientTracker_.erase(audioRendererChangeInfo.sessionId);
            }
            return SUCCESS;
        }
    }

    AUDIO_INFO_LOG("UpdateRendererStream: Not found clientUid:%{public}d sessionId:%{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.clientUID);
    return SUCCESS;
}


int32_t AudioStreamCollector::UpdateRendererStreamInternal(AudioStreamChangeInfo &streamChangeInfo)
{
    // Update the renderer internal info in audioRendererChangeInfos_
    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfo = **it;
        if ((*it)->clientUID == streamChangeInfo.audioRendererChangeInfo.clientUID &&
            (*it)->sessionId == streamChangeInfo.audioRendererChangeInfo.sessionId) {
            AUDIO_DEBUG_LOG("update client %{public}d session %{public}d", (*it)->clientUID, (*it)->sessionId);
            (*it)->prerunningState = streamChangeInfo.audioRendererChangeInfo.prerunningState;
            return SUCCESS;
        }
    }

    AUDIO_ERR_LOG("Not found clientUid:%{public}d sessionId:%{public}d",
        streamChangeInfo.audioRendererChangeInfo.clientUID, streamChangeInfo.audioRendererChangeInfo.sessionId);
    return ERROR;
}

int32_t AudioStreamCollector::UpdateCapturerStream(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("UpdateCapturerStream client %{public}d state %{public}d session %{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID, streamChangeInfo.audioCapturerChangeInfo.capturerState,
        streamChangeInfo.audioCapturerChangeInfo.sessionId);

    if (capturerStatequeue_.find(make_pair(streamChangeInfo.audioCapturerChangeInfo.clientUID,
        streamChangeInfo.audioCapturerChangeInfo.sessionId)) != capturerStatequeue_.end()) {
        if (streamChangeInfo.audioCapturerChangeInfo.capturerState ==
            capturerStatequeue_[make_pair(streamChangeInfo.audioCapturerChangeInfo.clientUID,
                streamChangeInfo.audioCapturerChangeInfo.sessionId)]) {
            // Capturer state not changed
            return SUCCESS;
        }
    }

    // Update the capturer info in audioCapturerChangeInfos_
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfo = **it;
        if (audioCapturerChangeInfo.clientUID == streamChangeInfo.audioCapturerChangeInfo.clientUID &&
            audioCapturerChangeInfo.sessionId == streamChangeInfo.audioCapturerChangeInfo.sessionId) {
            capturerStatequeue_[make_pair(audioCapturerChangeInfo.clientUID, audioCapturerChangeInfo.sessionId)] =
                streamChangeInfo.audioCapturerChangeInfo.capturerState;

            AUDIO_DEBUG_LOG("Session is updated for client %{public}d session %{public}d",
                streamChangeInfo.audioCapturerChangeInfo.clientUID,
                streamChangeInfo.audioCapturerChangeInfo.sessionId);

            shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
            CHECK_AND_RETURN_RET_LOG(capturerChangeInfo != nullptr,
                ERR_MEMORY_ALLOC_FAILED, "CapturerChangeInfo Memory Allocation Failed");
            SetCapturerStreamParam(streamChangeInfo, capturerChangeInfo);
            if (capturerChangeInfo->inputDeviceInfo.deviceType_ == DEVICE_TYPE_INVALID) {
                streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo = (*it)->inputDeviceInfo;
                capturerChangeInfo->inputDeviceInfo = (*it)->inputDeviceInfo;
            }
            capturerChangeInfo->appTokenId = (*it)->appTokenId;
            *it = move(capturerChangeInfo);
            if (audioPolicyServerHandler_ != nullptr) {
                SendCapturerInfoEvent(audioCapturerChangeInfos_);
            }
            if (streamChangeInfo.audioCapturerChangeInfo.capturerState ==  CAPTURER_RELEASED) {
                audioCapturerChangeInfos_.erase(it);
                capturerStatequeue_.erase(make_pair(audioCapturerChangeInfo.clientUID,
                    audioCapturerChangeInfo.sessionId));
                clientTracker_.erase(audioCapturerChangeInfo.sessionId);
            }
            return SUCCESS;
        }
    }
    AUDIO_DEBUG_LOG("UpdateCapturerStream: clientUI not in audioCapturerChangeInfos_::%{public}d",
        streamChangeInfo.audioCapturerChangeInfo.clientUID);
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateRendererDeviceInfo(AudioDeviceDescriptor &outputDeviceInfo)
{
    bool deviceInfoUpdated = false;

    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if (!(*it)->outputDeviceInfo.IsSameDeviceInfo(outputDeviceInfo)) {
            AUDIO_DEBUG_LOG("UpdateRendererDeviceInfo: old device: %{public}d new device: %{public}d",
                (*it)->outputDeviceInfo.deviceType_, outputDeviceInfo.deviceType_);
            (*it)->outputDeviceInfo = outputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    }
    if (deviceInfoUpdated) {
        AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateCapturerDeviceInfo(AudioDeviceDescriptor &inputDeviceInfo)
{
    bool deviceInfoUpdated = false;

    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if (!(*it)->inputDeviceInfo.IsSameDeviceInfo(inputDeviceInfo)) {
            AUDIO_DEBUG_LOG("UpdateCapturerDeviceInfo: old device: %{public}d new device: %{public}d",
                (*it)->inputDeviceInfo.deviceType_, inputDeviceInfo.deviceType_);
            (*it)->inputDeviceInfo = inputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateRendererDeviceInfo(int32_t clientUID, int32_t sessionId,
    AudioDeviceDescriptor &outputDeviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool deviceInfoUpdated = false;

    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->clientUID == clientUID && (*it)->sessionId == sessionId &&
            !(*it)->outputDeviceInfo.IsSameDeviceInfo(outputDeviceInfo)) {
            AUDIO_DEBUG_LOG("uid %{public}d sessionId %{public}d update device: old %{public}d, new %{public}d",
                clientUID, sessionId, (*it)->outputDeviceInfo.deviceType_, outputDeviceInfo.deviceType_);
            (*it)->outputDeviceInfo = outputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    }
    if (deviceInfoUpdated) {
        AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);
    }
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateRendererPipeInfo(const int32_t sessionId, const AudioPipeType pipeType)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool pipeTypeUpdated = false;

    for (auto it = audioRendererChangeInfos_.begin(); it != audioRendererChangeInfos_.end(); it++) {
        if ((*it)->sessionId == sessionId && (*it)->rendererInfo.pipeType != pipeType) {
            AUDIO_INFO_LOG("sessionId %{public}d update pipeType: old %{public}d, new %{public}d",
                sessionId, (*it)->rendererInfo.pipeType, pipeType);
            (*it)->rendererInfo.pipeType = pipeType;
            pipeTypeUpdated = true;
        }
    }

    if (pipeTypeUpdated && audioPolicyServerHandler_ != nullptr) {
        audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
    }
    if (pipeTypeUpdated) {
        AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);
    }
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateCapturerDeviceInfo(int32_t clientUID, int32_t sessionId,
    AudioDeviceDescriptor &inputDeviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool deviceInfoUpdated = false;

    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == clientUID && (*it)->sessionId == sessionId &&
            !(*it)->inputDeviceInfo.IsSameDeviceInfo(inputDeviceInfo)) {
            AUDIO_DEBUG_LOG("uid %{public}d sessionId %{public}d update device: old %{public}d, new %{public}d",
                (*it)->clientUID, (*it)->sessionId, (*it)->inputDeviceInfo.deviceType_, inputDeviceInfo.deviceType_);
            (*it)->inputDeviceInfo = inputDeviceInfo;
            deviceInfoUpdated = true;
        }
    }

    if (deviceInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTracker(const AudioMode &mode, AudioDeviceDescriptor &deviceInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    if (mode == AUDIO_MODE_PLAYBACK) {
        UpdateRendererDeviceInfo(deviceInfo);
    } else {
        UpdateCapturerDeviceInfo(deviceInfo);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // update the stream change info
    if (mode == AUDIO_MODE_PLAYBACK) {
        UpdateRendererStream(streamChangeInfo);
    } else {
    // mode = AUDIO_MODE_RECORD
        UpdateCapturerStream(streamChangeInfo);
    }
    WriterStreamChangeSysEvent(mode, streamChangeInfo);
    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateTrackerInternal(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // update the stream change internal info
    if (mode == AUDIO_MODE_PLAYBACK) {
        return UpdateRendererStreamInternal(streamChangeInfo);
    }
    return SUCCESS;
}

AudioStreamType AudioStreamCollector::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(std::make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    }

    if (streamType == STREAM_MEDIA) {
        streamType = STREAM_MUSIC;
    }

    return streamType;
}

AudioStreamType AudioStreamCollector::GetStreamType(int32_t sessionId)
{
    AudioStreamType streamType = STREAM_MUSIC;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->sessionId == sessionId) {
            streamType = GetStreamType(changeInfo->rendererInfo.contentType, changeInfo->rendererInfo.streamUsage);
        }
    }
    return streamType;
}

std::set<int32_t> AudioStreamCollector::GetSessionIdByStreamUsage(StreamUsage streamUsage)
{
    std::set<int32_t> sessionIdSet;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->rendererInfo.streamUsage == streamUsage &&
            changeInfo->outputDeviceInfo.deviceType_ == DEVICE_TYPE_SPEAKER &&
            changeInfo->outputDeviceInfo.networkId_ != LOCAL_NETWORK_ID) {
            sessionIdSet.insert(changeInfo->sessionId);
        }
    }
    return sessionIdSet;
}

std::set<int32_t> AudioStreamCollector::GetSessionIdBySourceType(SourceType sourceType)
{
    std::set<int32_t> sessionIdSet;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioCapturerChangeInfos_) {
        if (changeInfo->capturerInfo.sourceType == sourceType &&
            changeInfo->inputDeviceInfo.deviceType_ == DEVICE_TYPE_MIC &&
            changeInfo->inputDeviceInfo.networkId_ != LOCAL_NETWORK_ID) {
            sessionIdSet.insert(changeInfo->sessionId);
        }
    }
    return sessionIdSet;
}

std::set<int32_t> AudioStreamCollector::GetSessionIdByDeviceType(DeviceType deviceType)
{
    std::set<int32_t> sessionIdSet;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->outputDeviceInfo.deviceType_ == deviceType) {
            sessionIdSet.insert(changeInfo->sessionId);
        }
    }
    return sessionIdSet;
}

int32_t AudioStreamCollector::GetSessionIdPauseByRemote(InterruptHint hintType)
{
    int32_t sessionIdVec = -1;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->outputDeviceInfo.deviceType_ == DEVICE_TYPE_REMOTE_CAST &&
            changeInfo->rendererState == RendererState::RENDERER_RUNNING) {
            return changeInfo->sessionId;
        }
    }
    return sessionIdVec;
}

bool AudioStreamCollector::IsOffloadAllowed(const int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it == audioRendererChangeInfos_.end()) {
        AUDIO_WARNING_LOG("invalid session id: %{public}d", sessionId);
        return false;
    }
    return (*it)->rendererInfo.isOffloadAllowed;
}

int32_t AudioStreamCollector::GetChannelCount(int32_t sessionId)
{
    int32_t channelCount = 0;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it != audioRendererChangeInfos_.end()) {
        channelCount = (*it)->channelCount;
    }
    return channelCount;
}

int32_t AudioStreamCollector::GetCurrentRendererChangeInfos(
    std::vector<shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        rendererChangeInfos.push_back(make_shared<AudioRendererChangeInfo>(*changeInfo));
    }
    AUDIO_DEBUG_LOG("GetCurrentRendererChangeInfos returned");

    return SUCCESS;
}

int32_t AudioStreamCollector::GetCurrentCapturerChangeInfos(
    std::vector<shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos)
{
    AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos");
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioCapturerChangeInfos_) {
        if (!IsTransparentCapture(changeInfo->clientUID)) {
            capturerChangeInfos.push_back(make_shared<AudioCapturerChangeInfo>(*changeInfo));
        } else {
            AUDIO_INFO_LOG("GetCurrentCapturerChangeInfos remove uid:%{public}d", changeInfo->clientUID);
        }
        AUDIO_DEBUG_LOG("GetCurrentCapturerChangeInfos returned");
    }

    return SUCCESS;
}

void AudioStreamCollector::RegisteredRendererTrackerClientDied(const int32_t uid)
{
    int32_t sessionID = -1;
    auto audioRendererBegin = audioRendererChangeInfos_.begin();
    while (audioRendererBegin != audioRendererChangeInfos_.end()) {
        const auto &audioRendererChangeInfo = *audioRendererBegin;
        if (audioRendererChangeInfo == nullptr ||
            (audioRendererChangeInfo->clientUID != uid && audioRendererChangeInfo->createrUID != uid)) {
            audioRendererBegin++;
            continue;
        }
        sessionID = audioRendererChangeInfo->sessionId;
        audioRendererChangeInfo->rendererState = RENDERER_RELEASED;
        WriteRenderStreamReleaseSysEvent(audioRendererChangeInfo);
        if (audioPolicyServerHandler_ != nullptr) {
            audioPolicyServerHandler_->SendRendererInfoEvent(audioRendererChangeInfos_);
        }
        AudioSpatializationService::GetAudioSpatializationService().UpdateRendererInfo(audioRendererChangeInfos_);
        rendererStatequeue_.erase(make_pair(audioRendererChangeInfo->clientUID,
            audioRendererChangeInfo->sessionId));

        auto temp = audioRendererBegin;
        audioRendererBegin = audioRendererChangeInfos_.erase(temp);
        if ((sessionID != -1) && clientTracker_.erase(sessionID)) {
            AUDIO_INFO_LOG("TrackerClientDied:client %{public}d cleared", sessionID);
        }
    }
}

void AudioStreamCollector::RegisteredCapturerTrackerClientDied(const int32_t uid)
{
    int32_t sessionID = -1;
    auto audioCapturerBegin = audioCapturerChangeInfos_.begin();
    while (audioCapturerBegin != audioCapturerChangeInfos_.end()) {
        const auto &audioCapturerChangeInfo = *audioCapturerBegin;
        if (audioCapturerChangeInfo == nullptr ||
            (audioCapturerChangeInfo->clientUID != uid && audioCapturerChangeInfo->createrUID != uid)) {
            audioCapturerBegin++;
            continue;
        }
        sessionID = audioCapturerChangeInfo->sessionId;
        audioCapturerChangeInfo->capturerState = CAPTURER_RELEASED;
        WriteCaptureStreamReleaseSysEvent(audioCapturerChangeInfo);
        if (audioPolicyServerHandler_ != nullptr) {
            SendCapturerInfoEvent(audioCapturerChangeInfos_);
        }
        capturerStatequeue_.erase(make_pair(audioCapturerChangeInfo->clientUID,
            audioCapturerChangeInfo->sessionId));
        auto temp = audioCapturerBegin;
        audioCapturerBegin = audioCapturerChangeInfos_.erase(temp);
        if ((sessionID != -1) && clientTracker_.erase(sessionID)) {
            AUDIO_INFO_LOG("TrackerClientDied:client %{public}d cleared", sessionID);
        }
    }
}

void AudioStreamCollector::RegisteredTrackerClientDied(int32_t uid)
{
    AUDIO_INFO_LOG("TrackerClientDied:client:%{public}d Died", uid);

    // Send the release state event notification for all streams of died client to registered app
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    RegisteredRendererTrackerClientDied(uid);
    RegisteredCapturerTrackerClientDied(uid);
}

bool AudioStreamCollector::GetAndCompareStreamType(StreamUsage targetUsage, AudioRendererInfo rendererInfo)
{
    AudioStreamType requiredType = GetStreamType(CONTENT_TYPE_UNKNOWN, targetUsage);
    AUDIO_INFO_LOG("GetAndCompareStreamType:requiredType:%{public}d ", requiredType);
    AudioStreamType defaultStreamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(rendererInfo.contentType, rendererInfo.streamUsage));
    if (pos != streamTypeMap_.end()) {
        defaultStreamType = pos->second;
    }
    return defaultStreamType == requiredType;
}

int32_t AudioStreamCollector::GetUid(int32_t sessionId)
{
    int32_t defaultUid = -1;
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    const auto &it = std::find_if(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [&sessionId](const std::shared_ptr<AudioRendererChangeInfo> &changeInfo) {
            return changeInfo->sessionId == sessionId;
        });
    if (it != audioRendererChangeInfos_.end()) {
        defaultUid = (*it)->createrUID;
    }
    return defaultUid;
}

int32_t AudioStreamCollector::ResumeStreamState()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        std::shared_ptr<AudioClientTracker> callback = clientTracker_[changeInfo->sessionId];
        if (callback == nullptr) {
            AUDIO_ERR_LOG("AVSession is not alive,UpdateStreamState callback failed sId:%{public}d",
                changeInfo->sessionId);
            continue;
        }
        StreamSetStateEventInternal setStateEvent = {};
        setStateEvent.streamSetState = StreamSetState::STREAM_UNMUTE;
        setStateEvent.streamUsage = changeInfo->rendererInfo.streamUsage;
        callback->UnmuteStreamImpl(setStateEvent);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::UpdateStreamState(int32_t clientUid,
    StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->clientUID == clientUid &&
            streamSetStateEventInternal.streamUsage == changeInfo->rendererInfo.streamUsage) {
            AUDIO_INFO_LOG("UpdateStreamState Found matching uid=%{public}d and usage=%{public}d",
                clientUid, streamSetStateEventInternal.streamUsage);
            if (std::count(BACKGROUND_MUTE_STREAM_USAGE.begin(), BACKGROUND_MUTE_STREAM_USAGE.end(),
                streamSetStateEventInternal.streamUsage) == 0) {
                continue;
            }
            std::shared_ptr<AudioClientTracker> callback = clientTracker_[changeInfo->sessionId];
            if (callback == nullptr) {
                AUDIO_ERR_LOG("UpdateStreamState callback failed sId:%{public}d",
                    changeInfo->sessionId);
                continue;
            }
            if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_PAUSE) {
                callback->PausedStreamImpl(streamSetStateEventInternal);
            } else if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_RESUME) {
                callback->ResumeStreamImpl(streamSetStateEventInternal);
            } else if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_MUTE &&
                !changeInfo->backMute) {
                callback->MuteStreamImpl(streamSetStateEventInternal);
                changeInfo->backMute = true;
            } else if (streamSetStateEventInternal.streamSetState == StreamSetState::STREAM_UNMUTE &&
                changeInfo->backMute) {
                callback->UnmuteStreamImpl(streamSetStateEventInternal);
                changeInfo->backMute = false;
            }
        }
    }

    return SUCCESS;
}

bool AudioStreamCollector::IsStreamActive(AudioStreamType volumeType)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool result = false;
    for (auto &changeInfo: audioRendererChangeInfos_) {
        if (changeInfo->rendererState != RENDERER_RUNNING) {
            continue;
        }
        AudioVolumeType rendererVolumeType = GetVolumeTypeFromContentUsage((changeInfo->rendererInfo).contentType,
            (changeInfo->rendererInfo).streamUsage);
        if (rendererVolumeType == volumeType) {
            // An active stream has been found, return true directly.
            return true;
        }
    }
    return result;
}

int32_t AudioStreamCollector::GetRunningStream(AudioStreamType certainType, int32_t certainChannelCount)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    int32_t runningStream = -1;
    if ((certainType == STREAM_DEFAULT) && (certainChannelCount == 0)) {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if (changeInfo->rendererState == RENDERER_RUNNING) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    } else if (certainChannelCount == 0) {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if ((changeInfo->rendererState == RENDERER_RUNNING) &&
                    (certainType == GetStreamType(changeInfo->rendererInfo.contentType,
                    changeInfo->rendererInfo.streamUsage))) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    } else {
        for (auto &changeInfo : audioRendererChangeInfos_) {
            if ((changeInfo->rendererState == RENDERER_RUNNING) &&
                    (certainType == GetStreamType(changeInfo->rendererInfo.contentType,
                    changeInfo->rendererInfo.streamUsage)) && (certainChannelCount == changeInfo->channelCount)) {
                runningStream = changeInfo->sessionId;
                break;
            }
        }
    }
    return runningStream;
}

AudioStreamType AudioStreamCollector::GetVolumeTypeFromContentUsage(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    }
    return VolumeUtils::GetVolumeTypeFromStreamType(streamType);
}

AudioStreamType AudioStreamCollector::GetStreamTypeFromSourceType(SourceType sourceType)
{
    switch (sourceType) {
        case SOURCE_TYPE_MIC:
        case SOURCE_TYPE_UNPROCESSED:
            return STREAM_MUSIC;
        case SOURCE_TYPE_VOICE_COMMUNICATION:
        case SOURCE_TYPE_VOICE_CALL:
            return STREAM_VOICE_CALL;
        case SOURCE_TYPE_ULTRASONIC:
            return STREAM_ULTRASONIC;
        case SOURCE_TYPE_WAKEUP:
            return STREAM_WAKEUP;
        case SOURCE_TYPE_CAMCORDER:
            return STREAM_CAMCORDER;
        case SOURCE_TYPE_VOICE_RECOGNITION:
        case SOURCE_TYPE_PLAYBACK_CAPTURE:
        case SOURCE_TYPE_REMOTE_CAST:
        case SOURCE_TYPE_VIRTUAL_CAPTURE:
        case SOURCE_TYPE_VOICE_MESSAGE:
        default:
            return (AudioStreamType)sourceType;
    }
}

int32_t AudioStreamCollector::SetLowPowerVolume(int32_t streamId, float volume)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
        ERR_INVALID_PARAM, "SetLowPowerVolume streamId invalid.");
    std::shared_ptr<AudioClientTracker> callback = clientTracker_[streamId];
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ERR_INVALID_PARAM, "SetLowPowerVolume callback failed");
    callback->SetLowPowerVolumeImpl(volume);
    return SUCCESS;
}

float AudioStreamCollector::GetLowPowerVolume(int32_t streamId)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    float ret = 1.0; // invalue volume
    CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
        ret, "GetLowPowerVolume streamId invalid.");
    float volume;
    std::shared_ptr<AudioClientTracker> callback = clientTracker_[streamId];
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        ret, "GetLowPowerVolume callback failed");
    callback->GetLowPowerVolumeImpl(volume);
    return volume;
}

int32_t AudioStreamCollector::SetOffloadMode(int32_t streamId, int32_t state, bool isAppBack)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ERR_INVALID_PARAM, "streamId (%{public}d) invalid.", streamId);
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback failed");
    }
    callback->SetOffloadModeImpl(state, isAppBack);
    return SUCCESS;
}

int32_t AudioStreamCollector::UnsetOffloadMode(int32_t streamId)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ERR_INVALID_PARAM, "streamId (%{public}d) invalid.", streamId);
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback failed");
    }
    callback->UnsetOffloadModeImpl();
    return SUCCESS;
}

float AudioStreamCollector::GetSingleStreamVolume(int32_t streamId)
{
    std::shared_ptr<AudioClientTracker> callback;
    {
        std::lock_guard<std::mutex> lock(streamsInfoMutex_);
        float ret = 1.0; // invalue volume
        CHECK_AND_RETURN_RET_LOG(!(clientTracker_.count(streamId) == 0),
            ret, "GetSingleStreamVolume streamId invalid.");
        callback = clientTracker_[streamId];
        CHECK_AND_RETURN_RET_LOG(callback != nullptr,
            ret, "GetSingleStreamVolume callback failed");
    }
    float volume;
    callback->GetSingleStreamVolumeImpl(volume);
    return volume;
}

int32_t AudioStreamCollector::UpdateCapturerInfoMuteStatus(int32_t uid, bool muteStatus)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    bool capturerInfoUpdated = false;
    for (auto it = audioCapturerChangeInfos_.begin(); it != audioCapturerChangeInfos_.end(); it++) {
        if ((*it)->clientUID == uid || uid == 0) {
            (*it)->muted = muteStatus;
            capturerInfoUpdated = true;
            std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
                Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::CAPTURE_MUTE_STATUS_CHANGE,
                Media::MediaMonitor::EventType::BEHAVIOR_EVENT);
            bean->Add("ISOUTPUT", 0);
            bean->Add("STREAMID", (*it)->sessionId);
            bean->Add("STREAM_TYPE", (*it)->capturerInfo.sourceType);
            bean->Add("DEVICETYPE", (*it)->inputDeviceInfo.deviceType_);
            bean->Add("MUTED", (*it)->muted);
            Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        }
    }

    if (capturerInfoUpdated && audioPolicyServerHandler_ != nullptr) {
        SendCapturerInfoEvent(audioCapturerChangeInfos_);
    }

    return SUCCESS;
}

int32_t AudioStreamCollector::SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object)
{
    return audioConcurrencyService_->SetAudioConcurrencyCallback(sessionID, object);
}

int32_t AudioStreamCollector::UnsetAudioConcurrencyCallback(const uint32_t sessionID)
{
    return audioConcurrencyService_->UnsetAudioConcurrencyCallback(sessionID);
}

int32_t AudioStreamCollector::ActivateAudioConcurrency(const AudioPipeType &pipeType)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    return audioConcurrencyService_->ActivateAudioConcurrency(pipeType,
        audioRendererChangeInfos_, audioCapturerChangeInfos_);
}

void AudioStreamCollector::WriterStreamChangeSysEvent(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        WriterRenderStreamChangeSysEvent(streamChangeInfo);
    } else {
        WriterCaptureStreamChangeSysEvent(streamChangeInfo);
    }
}

void AudioStreamCollector::WriterRenderStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo)
{
    bool isOutput = true;
    AudioStreamType streamType = GetVolumeTypeFromContentUsage(
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.contentType,
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceType_, OUTPUT_DEVICE);

    uint8_t effectChainType = EFFECT_CHAIN_TYPE_MAP.count(
        streamChangeInfo.audioRendererChangeInfo.rendererInfo.sceneType) ?
        EFFECT_CHAIN_TYPE_MAP.at(streamChangeInfo.audioRendererChangeInfo.rendererInfo.sceneType) :
        EFFECT_CHAIN_TYPE_MAP.at("UNKNOWN");

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", isOutput ? 1 : 0);
    bean->Add("STREAMID", streamChangeInfo.audioRendererChangeInfo.sessionId);
    bean->Add("UID", streamChangeInfo.audioRendererChangeInfo.clientUID);
    bean->Add("PID", streamChangeInfo.audioRendererChangeInfo.clientPid);
    bean->Add("TRANSACTIONID", transactionId);
    bean->Add("STREAMTYPE", streamType);
    bean->Add("STATE", streamChangeInfo.audioRendererChangeInfo.rendererState);
    bean->Add("DEVICETYPE", streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceType_);
    bean->Add("BT_TYPE", streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.deviceCategory_);
    bean->Add("PIPE_TYPE", streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType);
    bean->Add("STREAM_TYPE", streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage);
    bean->Add("SAMPLE_RATE", streamChangeInfo.audioRendererChangeInfo.rendererInfo.samplingRate);
    bean->Add("NETWORKID", ConvertNetworkId(streamChangeInfo.audioRendererChangeInfo.outputDeviceInfo.networkId_));
    bean->Add("ENCODING_TYPE", streamChangeInfo.audioRendererChangeInfo.rendererInfo.encodingType);
    bean->Add("CHANNEL_LAYOUT", streamChangeInfo.audioRendererChangeInfo.rendererInfo.channelLayout);
    bean->Add("EFFECT_CHAIN", effectChainType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioStreamCollector::WriterCaptureStreamChangeSysEvent(AudioStreamChangeInfo &streamChangeInfo)
{
    bool isOutput = false;
    AudioStreamType streamType = GetStreamTypeFromSourceType(
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceType_, INPUT_DEVICE);

    uint8_t effectChainType = EFFECT_CHAIN_TYPE_MAP.count(
        streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sceneType) ?
        EFFECT_CHAIN_TYPE_MAP.at(streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sceneType) :
        EFFECT_CHAIN_TYPE_MAP.at("UNKNOWN");

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", isOutput ? 1 : 0);
    bean->Add("STREAMID", streamChangeInfo.audioCapturerChangeInfo.sessionId);
    bean->Add("UID", streamChangeInfo.audioCapturerChangeInfo.clientUID);
    bean->Add("PID", streamChangeInfo.audioCapturerChangeInfo.clientPid);
    bean->Add("TRANSACTIONID", transactionId);
    bean->Add("STREAMTYPE", streamType);
    bean->Add("STATE", streamChangeInfo.audioCapturerChangeInfo.capturerState);
    bean->Add("DEVICETYPE", streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceType_);
    bean->Add("BT_TYPE", streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.deviceCategory_);
    bean->Add("PIPE_TYPE", streamChangeInfo.audioCapturerChangeInfo.capturerInfo.pipeType);
    bean->Add("STREAM_TYPE", streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType);
    bean->Add("SAMPLE_RATE", streamChangeInfo.audioCapturerChangeInfo.capturerInfo.samplingRate);
    bean->Add("MUTED", streamChangeInfo.audioCapturerChangeInfo.muted);
    bean->Add("NETWORKID", ConvertNetworkId(streamChangeInfo.audioCapturerChangeInfo.inputDeviceInfo.networkId_));
    bean->Add("ENCODING_TYPE", streamChangeInfo.audioCapturerChangeInfo.capturerInfo.encodingType);
    bean->Add("CHANNEL_LAYOUT", streamChangeInfo.audioCapturerChangeInfo.capturerInfo.channelLayout);
    bean->Add("EFFECT_CHAIN", effectChainType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}


void AudioStreamCollector::WriteRenderStreamReleaseSysEvent(
    const std::shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfo)
{
    AudioStreamType streamType = GetVolumeTypeFromContentUsage(audioRendererChangeInfo->rendererInfo.contentType,
        audioRendererChangeInfo->rendererInfo.streamUsage);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        audioRendererChangeInfo->outputDeviceInfo.deviceType_, OUTPUT_DEVICE);

    uint8_t effectChainType = EFFECT_CHAIN_TYPE_MAP.count(
        audioRendererChangeInfo->rendererInfo.sceneType) ?
        EFFECT_CHAIN_TYPE_MAP.at(audioRendererChangeInfo->rendererInfo.sceneType) :
        EFFECT_CHAIN_TYPE_MAP.at("UNKNOWN");

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", 1);
    bean->Add("STREAMID", audioRendererChangeInfo->sessionId);
    bean->Add("UID", audioRendererChangeInfo->clientUID);
    bean->Add("PID", audioRendererChangeInfo->clientPid);
    bean->Add("TRANSACTIONID", transactionId);
    bean->Add("STREAMTYPE", streamType);
    bean->Add("STATE", audioRendererChangeInfo->rendererState);
    bean->Add("DEVICETYPE", audioRendererChangeInfo->outputDeviceInfo.deviceType_);
    bean->Add("BT_TYPE", audioRendererChangeInfo->outputDeviceInfo.deviceCategory_);
    bean->Add("PIPE_TYPE", audioRendererChangeInfo->rendererInfo.pipeType);
    bean->Add("STREAM_TYPE", audioRendererChangeInfo->rendererInfo.streamUsage);
    bean->Add("SAMPLE_RATE", audioRendererChangeInfo->rendererInfo.samplingRate);
    bean->Add("NETWORKID", ConvertNetworkId(audioRendererChangeInfo->outputDeviceInfo.networkId_));
    bean->Add("ENCODING_TYPE", audioRendererChangeInfo->rendererInfo.encodingType);
    bean->Add("CHANNEL_LAYOUT", audioRendererChangeInfo->rendererInfo.channelLayout);
    bean->Add("EFFECT_CHAIN", effectChainType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioStreamCollector::WriteCaptureStreamReleaseSysEvent(
    const std::shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfo)
{
    AudioStreamType streamType = GetStreamTypeFromSourceType(audioCapturerChangeInfo->capturerInfo.sourceType);
    uint64_t transactionId = audioSystemMgr_->GetTransactionId(
        audioCapturerChangeInfo->inputDeviceInfo.deviceType_, INPUT_DEVICE);

    uint8_t effectChainType = EFFECT_CHAIN_TYPE_MAP.count(
        audioCapturerChangeInfo->capturerInfo.sceneType) ?
        EFFECT_CHAIN_TYPE_MAP.at(audioCapturerChangeInfo->capturerInfo.sceneType) :
        EFFECT_CHAIN_TYPE_MAP.at("UNKNOWN");

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_CHANGE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ISOUTPUT", 1);
    bean->Add("STREAMID", audioCapturerChangeInfo->sessionId);
    bean->Add("UID", audioCapturerChangeInfo->clientUID);
    bean->Add("PID", audioCapturerChangeInfo->clientPid);
    bean->Add("TRANSACTIONID", transactionId);
    bean->Add("STREAMTYPE", streamType);
    bean->Add("STATE", audioCapturerChangeInfo->capturerState);
    bean->Add("DEVICETYPE", audioCapturerChangeInfo->inputDeviceInfo.deviceType_);
    bean->Add("BT_TYPE", audioCapturerChangeInfo->inputDeviceInfo.deviceCategory_);
    bean->Add("PIPE_TYPE", audioCapturerChangeInfo->capturerInfo.pipeType);
    bean->Add("STREAM_TYPE", audioCapturerChangeInfo->capturerInfo.sourceType);
    bean->Add("SAMPLE_RATE", audioCapturerChangeInfo->capturerInfo.samplingRate);
    bean->Add("MUTED", audioCapturerChangeInfo->muted);
    bean->Add("NETWORKID", ConvertNetworkId(audioCapturerChangeInfo->inputDeviceInfo.networkId_));
    bean->Add("ENCODING_TYPE", audioCapturerChangeInfo->capturerInfo.encodingType);
    bean->Add("CHANNEL_LAYOUT", audioCapturerChangeInfo->capturerInfo.channelLayout);
    bean->Add("EFFECT_CHAIN", effectChainType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

bool AudioStreamCollector::IsCallStreamUsage(StreamUsage usage)
{
    if (usage == STREAM_USAGE_VOICE_COMMUNICATION || usage == STREAM_USAGE_VIDEO_COMMUNICATION ||
        usage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        return true;
    }
    return false;
}

StreamUsage AudioStreamCollector::GetRunningStreamUsageNoUltrasonic()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->rendererState == RENDERER_RUNNING &&
            changeInfo->rendererInfo.streamUsage != STREAM_USAGE_ULTRASONIC) {
            return changeInfo->rendererInfo.streamUsage;
        }
    }
    return STREAM_USAGE_INVALID;
}

SourceType AudioStreamCollector::GetRunningSourceTypeNoUltrasonic()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioCapturerChangeInfos_) {
        if (changeInfo->capturerState == CAPTURER_RUNNING &&
            changeInfo->capturerInfo.sourceType != SOURCE_TYPE_ULTRASONIC) {
            return changeInfo->capturerInfo.sourceType;
        }
    }
    return SOURCE_TYPE_INVALID;
}

StreamUsage AudioStreamCollector::GetLastestRunningCallStreamUsage()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        StreamUsage usage = changeInfo->rendererInfo.streamUsage;
        RendererState state = changeInfo->rendererState;
        if ((IsCallStreamUsage(usage) && state == RENDERER_RUNNING) ||
            (usage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION && state == RENDERER_PREPARED)) {
            return usage;
        }
    }
    return STREAM_USAGE_UNKNOWN;
}

std::vector<uint32_t> AudioStreamCollector::GetAllRendererSessionIDForUID(int32_t uid)
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    std::vector<uint32_t> sessionIDSet;
    for (const auto &changeInfo : audioRendererChangeInfos_) {
        if (changeInfo->clientUID == uid) {
            sessionIDSet.push_back(changeInfo->sessionId);
        }
    }
    return sessionIDSet;
}

bool AudioStreamCollector::ChangeVoipCapturerStreamToNormal()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    int count = std::count_if(audioCapturerChangeInfos_.begin(), audioCapturerChangeInfos_.end(),
        [](const auto &changeInfo) {
            const auto &sourceType = changeInfo->capturerInfo.sourceType;
            return sourceType == SOURCE_TYPE_VOICE_COMMUNICATION || sourceType == SOURCE_TYPE_MIC ||
                sourceType == SOURCE_TYPE_VOICE_MESSAGE || sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION;
        });

    AUDIO_INFO_LOG("Has capture stream count: %{public}d", count);
    // becasue self has been added
    return count > 1;
}

bool AudioStreamCollector::HasVoipRendererStream()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // judge stream original flage is AUDIO_FLAG_VOIP_FAST
    bool hasVoip = std::any_of(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [](const auto &changeInfo) {
            return changeInfo->rendererInfo.originalFlag == AUDIO_FLAG_VOIP_FAST;
        });

    AUDIO_INFO_LOG("Has Fast Voip stream : %{public}d", hasVoip);
    return hasVoip;
}

bool AudioStreamCollector::HasRunningRendererStream()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // judge stream state is running
    bool hasRunningRendererStream = std::any_of(audioRendererChangeInfos_.begin(), audioRendererChangeInfos_.end(),
        [](const auto &changeInfo) {
            return ((changeInfo->rendererState == RENDERER_RUNNING) || (changeInfo->rendererInfo.streamUsage ==
                STREAM_USAGE_VOICE_MODEM_COMMUNICATION && changeInfo->rendererState == RENDERER_PREPARED));
        });
    AUDIO_INFO_LOG("Has Running Renderer stream : %{public}d", hasRunningRendererStream);
    return hasRunningRendererStream;
}

bool AudioStreamCollector::HasRunningRecognitionCapturerStream()
{
    std::lock_guard<std::mutex> lock(streamsInfoMutex_);
    // judge stream state is running
    bool hasRunningRecognitionCapturerStream = std::any_of(audioCapturerChangeInfos_.begin(),
        audioCapturerChangeInfos_.end(),
        [](const auto &changeInfo) {
            return ((changeInfo->capturerState == CAPTURER_RUNNING) && (changeInfo->capturerInfo.sourceType ==
                SOURCE_TYPE_VOICE_RECOGNITION));
        });

    AUDIO_INFO_LOG("Has Running Recognition stream : %{public}d", hasRunningRecognitionCapturerStream);
    return hasRunningRecognitionCapturerStream;
}
} // namespace AudioStandard
} // namespace OHOS