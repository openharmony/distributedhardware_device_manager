/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "dm_auth_cert.h"

#include "dm_error_type.h"
#include "dm_log.h"

namespace OHOS {
namespace DistributedHardware {

constexpr const char* LIB_DM_AUTH_CERT = "libdm_auth_cert.z.so";

AuthCert::AuthCert()
{
    LOGD("AuthCert AuthCert");
}

AuthCert::~AuthCert()
{
    LOGD("AuthCert destructor");
    if (authCertSoHandle_ != nullptr) {
        LOGI("dm auth cert authCertSoHandle_ is not nullptr.");
        dlclose(authCertSoHandle_);
        authCertSoHandle_ = nullptr;
    }
}

bool AuthCert::IsDMAdapterAuthCertLoaded()
{
    LOGI("Start.");
    std::lock_guard<ffrt::mutex> lock(isAdapterAuthCertLoadedLock_);
    if (isAdapterAuthCertSoLoaded_ && (dmAuthCertExt_ != nullptr)) {
        return true;
    }
    authCertSoHandle_ = dlopen(LIB_DM_AUTH_CERT, RTLD_NOW | RTLD_NODELETE | RTLD_NOLOAD);
    char *error = dlerror();
    if (error != nullptr) {
        LOGE("dlopen failed, err: %{public}s", error);
    }
    if (authCertSoHandle_ == nullptr) {
        authCertSoHandle_ = dlopen(LIB_DM_AUTH_CERT, RTLD_NOW | RTLD_NODELETE);
        error = dlerror();
        if (error != nullptr) {
            LOGE("dlopen failed again! err: %{public}s", error);
        }
    }
    if (authCertSoHandle_ == nullptr) {
        LOGE("load dm check api white list so failed.");
        return false;
    }
    auto func = (CreateDMAuthCertFuncPtr)dlsym(authCertSoHandle_, "CreateDMAuthCertExtObject");
    if (func == nullptr || dlerror() != nullptr) {
        dlclose(authCertSoHandle_);
        authCertSoHandle_ = nullptr;
        LOGE("Create object function is not exist. err: %{public}s", (dlerror() == nullptr ? "null" : dlerror()));
        return false;
    }
    dmAuthCertExt_ = std::shared_ptr<IDMAuthCertExt>(func());
    isAdapterAuthCertSoLoaded_ = true;
    LOGI("Success.");
    return true;
}

int32_t AuthCert::GenerateCertificate(DmCertChain &dmCertChain)
{
    if (!IsDMAdapterAuthCertLoaded()) {
        LOGE("authCertSo load failed!");
        return ERR_DM_FAILED;
    }
    return dmAuthCertExt_->GenerateCertificate(dmCertChain);
}

int32_t AuthCert::VerifyCertificate(const DmCertChain &dmCertChain, const char *deviceIdHash)
{
    if (!IsDMAdapterAuthCertLoaded()) {
        LOGE("authCertSo load failed!");
        return ERR_DM_FAILED;
    }
    return dmAuthCertExt_->VerifyCertificate(dmCertChain, deviceIdHash);
}

int32_t AuthCert::GenerateCertificateV2(DmCertChain &dmCertChain, const uint64_t random)
{
    if (!IsDMAdapterAuthCertLoaded()) {
        LOGE("authCertSo load failed!");
        return ERR_DM_FAILED;
    }
    return dmAuthCertExt_->GenerateCertificateV2(dmCertChain, random);
}

int32_t AuthCert::VerifyCertificateV2(const DmCertChain &dmCertChain, const char *deviceIdHash, const uint64_t random)
{
    if (!IsDMAdapterAuthCertLoaded()) {
        LOGE("authCertSo load failed!");
        return ERR_DM_FAILED;
    }
    return dmAuthCertExt_->VerifyCertificateV2(dmCertChain, deviceIdHash, random);
}
} // namespace DistributedHardware
} // namespace OHOS