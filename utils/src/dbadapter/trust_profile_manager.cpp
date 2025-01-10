/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "trust_profile_manager.h"
#include "subscribe_profile_manager.h"
#include "distributed_device_profile_log.h"
#include "rdb_adapter.h"
#include "profile_utils.h"
#include "device_manager.h"
#include "distributed_device_profile_constants.h"
#include "distributed_device_profile_errors.h"
#include "accesser.h"
#include "accessee.h"

namespace OHOS {
namespace DistributedDeviceProfile {
IMPLEMENT_SINGLE_INSTANCE(TrustProfileManager);
namespace {
    const std::string TAG = "TrustProfileManager";
}

int32_t TrustProfileManager::Init()
{
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        rdbStore_ = std::make_shared<RdbAdapter>();
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_INIT_DB_FAILED;
        }
        int32_t ret = rdbStore_->Init();
        if (ret != DP_SUCCESS) {
            HILOGE("rdbStore_ Init failed");
            return DP_INIT_DB_FAILED;
        }
    }
    this->CreateTable();
    this->CreateUniqueIndex();
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UnInit()
{
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->UnInit();
        if (ret != DP_SUCCESS) {
            HILOGE("rdbStore_ Uninit failed");
            return DP_UNINIT_FAIL;
        }
        rdbStore_ = nullptr;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutTrustDeviceProfile(const TrustDeviceProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::TrustDeviceProfileToEntries(profile, values);

    int64_t rowId = ROWID_INIT;
    int32_t ret = RET_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Put(rowId, TRUST_DEVICE_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("trust_device_table insert failed");
            return DP_PUT_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileAdd(profile);
    if (ret != DP_SUCCESS) {
        HILOGE("NotifyTrustDeviceProfileAdd failed");
        return DP_NOTIFY_TRUST_DEVICE_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccessControlProfile(const AccessControlProfile& profile)
{
    AccessControlProfile accessControlProfile(profile);
    int32_t ret = this->SetAccessControlProfileId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccessControlProfileId failed");
        return ret;
    }
    ret = this->PutAccesserProfile(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("PutAccesserProfile failed");
        return ret;
    }
    ret = this->PutAccesseeProfile(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("PutAccesseeProfile failed");
        return ret;
    }
    if (IsAclExists(accessControlProfile) == DP_DATA_EXISTS) {
        HILOGE("acl is exists");
        return DP_SUCCESS;
    }
    ValuesBucket values;
    ProfileUtils::AccessControlProfileToEntries(accessControlProfile, values);
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int64_t rowId = ROWID_INIT;
        if (rdbStore_->Put(rowId, ACCESS_CONTROL_TABLE, values) != DP_SUCCESS) {
            HILOGE("access_control_table insert failed");
            return DP_PUT_ACL_PROFILE_FAIL;
        }
    }
    HILOGI("PutAclProfile : %{public}s", accessControlProfile.dump().c_str());
    ret = this->PutAclCheck(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("PutAclCheck failed");
        return ret;
    }
    if (DistributedHardware::DeviceManager::GetInstance().DpAclAdd(accessControlProfile.GetAccessControlId(),
        accessControlProfile.GetTrustDeviceId(), accessControlProfile.GetBindType()) != DP_SUCCESS) {
        HILOGE("Notify accessControlProfile Add failed");
        return DP_NOTIFY_ACCESS_CONTROL_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateTrustDeviceProfile(const TrustDeviceProfile& profile)
{
    std::string deviceId = profile.GetDeviceId();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = resultSet->GoToFirstRow();
    if (ret != DP_SUCCESS) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    TrustDeviceProfile oldProfile;
    this->ConvertToTrustDeviceProfile(resultSet, oldProfile);
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::TrustDeviceProfileToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Update(changeRowCnt, TRUST_DEVICE_TABLE, values, DEVICEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetDeviceId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("Update trust_device_table failed");
            return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    this->UpdateTrustDeviceProfileNotify(oldProfile, profile);
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccessControlProfile(const AccessControlProfile& profile)
{
    int32_t ret = this->UpdateAclCheck(profile);
    if (ret != DP_SUCCESS) {
        HILOGE("UpdateAclCheck faild");
        return ret;
    }
    this->UpdateAccesserProfile(profile.GetAccesserId(), profile);
    this->UpdateAccesseeProfile(profile.GetAccesseeId(), profile);
    ValuesBucket values;
    ProfileUtils::AccessControlProfileToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        ret = rdbStore_->Update(changeRowCnt, ACCESS_CONTROL_TABLE, values, ACCESSCONTROLID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("update access_control_table failed");
            return DP_UPDATE_ACL_PROFILE_FAIL;
        }
    }
    HILOGI("UpdateAclProfile : %{public}s", profile.dump().c_str());
    int32_t status = STATUS_INIT;
    ret = this->GetResultStatus(profile.GetTrustDeviceId(), status);
    if (ret != DP_SUCCESS) {
        HILOGE("GetResultStatus failed");
        return DP_GET_RESULTSET_FAIL;
    }
    TrustDeviceProfile trustProfile;
    this->ConvertToTrustDeviceProfile(profile, trustProfile);
    trustProfile.SetStatus(status);
    ret = this->UpdateTrustDeviceProfile(trustProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("UpdateTrustDeviceProfile failed");
        return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetTrustDeviceProfile(const std::string& deviceId, TrustDeviceProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet = GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = resultSet->GoToFirstRow();
    if (ret != DP_SUCCESS) {
        HILOGE("not find trust device data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    this->ConvertToTrustDeviceProfile(resultSet, profile);
    resultSet->Close();
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAllTrustDeviceProfile(std::vector<TrustDeviceProfile>& profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_TRUST_DEVICE_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trust_device_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        TrustDeviceProfile trustProfile;
        this->ConvertToTrustDeviceProfile(resultSet, trustProfile);
        profile.push_back(trustProfile);
    }
    resultSet->Close();
    if (profile.empty()) {
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, const std::string& bundleName,
    int32_t bindType, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN) {
        HILOGE("bundleName is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, bundleName : %{public}s, bindtype : %{public}d, status : %{public}d",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(), bundleName.c_str(), bindType, status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_BINDTYPE_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(bindType), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("bindType and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByUserIdAndBundleName(resultSet, userId, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByUserIdAndBundleName faild");
        return DP_NOT_FIND_DATA;
    }
    if (profile.empty()) {
        HILOGE("by userId bundleName bindType status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, const std::string& bundleName,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN || trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("bundleName or trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, bundleName : %{public}s, trustDeviceId : %{public}s, status : %{public}d",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(),
        bundleName.c_str(), ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(trustDeviceId), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByUserIdAndBundleName(resultSet, userId, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByUserIdAndBundleName faild");
        return ret;
    }
    if (profile.empty()) {
        HILOGE("by userId bundleName trustDeviceId status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfileByTokenId(int64_t tokenId,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, tokenId : %{public}" PRId64 ", trustDeviceId : %{public}s, status : %{public}d",
        tokenId, ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_STATUS,
        std::vector<ValueObject>{ ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByTokenId(resultSet, trustDeviceId, tokenId, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByTokenId faild");
        return ret;
    }
    if (profile.empty()) {
        HILOGE("tokenId not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId,
    const std::string& accountId, std::vector<AccessControlProfile>& profile)
{
    if (accountId.size() > MAX_STRING_LEN) {
        HILOGE("accountId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, userId : %{public}s, accountId : %{public}s",
        ProfileUtils::GetAnonyString(std::to_string(userId)).c_str(),
        ProfileUtils::GetAnonyString(accountId).c_str());
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = this->GetAclProfileByUserIdAndAccountId(
            resultSet, accesserId, accesseeId, userId, accountId, profile);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAclProfileByUserIdAndAccountId faild");
            return ret;
        }
    }
    resultSet->Close();
    if (profile.empty()) {
        HILOGE("by userId accountId not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGD("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(int32_t userId, std::vector<AccessControlProfile> &profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = GetAccessControlProfiles(resultSet, accesserId, accesseeId, userId, profile);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAccessControlProfile faild");
            resultSet->Close();
            return ret;
        }
    }
    resultSet->Close();
    if (profile.empty()) {
        HILOGE("by userId not find data, userId: %{public}s",
            ProfileUtils::GetAnonyString(std::to_string(userId)).c_str());
        return DP_NOT_FIND_DATA;
    }
    HILOGD("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAllAccessControlProfile(std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("access_control_table no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t ret = this->GetAccessControlProfile(resultSet, accesserId, accesseeId, profile);
        if (ret != DP_SUCCESS) {
            HILOGE("GetAccessControlProfile faild");
            resultSet->Close();
            return ret;
        }
    }
    resultSet->Close();
    if (profile.empty()) {
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::string& bundleName,
    int32_t bindType, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN) {
        HILOGE("bundleName is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, bundleName : %{public}s, bindType : %{public}d, status : %{public}d",
        bundleName.c_str(), bindType, status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_BINDTYPE_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(bindType), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("bindType and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByBundleName(resultSet, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByBundleName faild");
        return ret;
    }
    if (profile.empty()) {
        HILOGE("by bundleName bindType status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::string& bundleName,
    const std::string& trustDeviceId, int32_t status, std::vector<AccessControlProfile>& profile)
{
    if (bundleName.size() > MAX_STRING_LEN || trustDeviceId.size() > MAX_STRING_LEN) {
        HILOGE("bundleName or trustDeviceId is invalid");
        return DP_INVALID_PARAMS;
    }
    HILOGI("Params, bundleName : %{public}s, trustDeviceId : %{public}s, status : %{public}d",
        bundleName.c_str(), ProfileUtils::GetAnonyString(trustDeviceId).c_str(), status);
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID_AND_STATUS,
        std::vector<ValueObject>{ ValueObject(trustDeviceId), ValueObject(status) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId and status not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->GetAclProfileByBundleName(resultSet, bundleName, profile);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("GetAclProfileByBundleName faild");
        return ret;
    }
    if (profile.empty()) {
        HILOGE("by bundleName trustDeviceId status not find data");
        return DP_NOT_FIND_DATA;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(const std::map<std::string, std::string>& params,
    std::vector<AccessControlProfile>& profile)
{
    if (params.find(TRUST_DEVICE_ID) != params.end() && params.find(STATUS) != params.end()) {
        if (params.find(USERID) != params.end() && params.find(BUNDLENAME) != params.end()) {
            int32_t ret = this->GetAccessControlProfile(std::atoi(params.at(USERID).c_str()),
                params.at(BUNDLENAME), params.at(TRUST_DEVICE_ID),
                std::atoi(params.at(STATUS).c_str()), profile);
            return ret;
        }
        if (params.find(BUNDLENAME) != params.end()) {
            int32_t ret = this->GetAccessControlProfile(params.at(BUNDLENAME),
                params.at(TRUST_DEVICE_ID), std::atoi(params.at(STATUS).c_str()), profile);
            return ret;
        }
        if (params.find(TOKENID) != params.end()) {
            int32_t ret = this->GetAccessControlProfileByTokenId(std::atoi(params.at(TOKENID).c_str()),
                params.at(TRUST_DEVICE_ID), std::atoi(params.at(STATUS).c_str()), profile);
            return ret;
        }
    }
    if (params.find(BIND_TYPE) != params.end() && params.find(STATUS) != params.end()) {
        if (params.find(USERID) != params.end() && params.find(BUNDLENAME) != params.end()) {
            int32_t ret = this->GetAccessControlProfile(std::atoi(params.at(USERID).c_str()),
                params.at(BUNDLENAME), std::atoi(params.at(BIND_TYPE).c_str()),
                std::atoi(params.at(STATUS).c_str()), profile);
            return ret;
        }
        if (params.find(BUNDLENAME) != params.end()) {
            int32_t ret = this->GetAccessControlProfile(params.at(BUNDLENAME),
                std::atoi(params.at(BIND_TYPE).c_str()),
                std::atoi(params.at(STATUS).c_str()), profile);
            return ret;
        }
    }
    if (params.find(USERID) != params.end()) {
        if (params.find(ACCOUNTID) != params.end()) {
            int32_t ret = this->GetAccessControlProfile(std::atoi(params.at(USERID).c_str()),
                params.at(ACCOUNTID), profile);
            return ret;
        }
        int32_t ret = this->GetAccessControlProfile(std::atoi(params.at(USERID).c_str()), profile);
        return ret;
    }
    HILOGE("params is error");
    return DP_INVALID_PARAMS;
}

int32_t TrustProfileManager::DeleteTrustDeviceProfile(const std::string& deviceId)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(deviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = resultSet->GoToFirstRow();
    if (ret != DP_SUCCESS) {
        HILOGE("deviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    TrustDeviceProfile profile;
    this->ConvertToTrustDeviceProfile(resultSet, profile);
    resultSet->Close();
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        ret = rdbStore_->Delete(deleteRows, TRUST_DEVICE_TABLE, DEVICEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(deviceId) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete trust_device_table data failed");
            return DP_DELETE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileDelete(profile);
    if (ret != DP_SUCCESS) {
        HILOGE("ProfileDelete failed");
        return DP_NOTIFY_TRUST_DEVICE_FAIL;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccessControlProfile(int64_t accessControlId)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSCONTROLID,
        std::vector<ValueObject>{ ValueObject(accessControlId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("no data");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t ret = this->DeleteAccessControlProfileCheck(resultSet);
    resultSet->Close();
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteAccessControlProfileCheck failed");
        return ret;
    }
    HILOGI("end!");
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CreateTable()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return DP_GET_RDBSTORE_FAIL;
    }
    int32_t ret = rdbStore_->CreateTable(CREATE_TURST_DEVICE_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("trust_device_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESS_CONTROL_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("access_control_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSER_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accesser_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSEE_TABLE_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accessee_table create failed");
        return DP_CREATE_TABLE_FAIL;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::CreateUniqueIndex()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return DP_GET_RDBSTORE_FAIL;
    }
    int32_t ret = rdbStore_->CreateTable(CREATE_TURST_DEVICE_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("trust_device_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESS_CONTROL_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("access_control_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSER_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accesser_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    ret = rdbStore_->CreateTable(CREATE_ACCESSEE_TABLE_UNIQUE_INDEX_SQL);
    if (ret != DP_SUCCESS) {
        HILOGE("accessee_table unique index create failed");
        return DP_CREATE_UNIQUE_INDEX_FAIL;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToTrustDeviceProfile(
    const AccessControlProfile& accessControlProfile, TrustDeviceProfile& trustDeviceProfile)
{
    trustDeviceProfile.SetDeviceId(accessControlProfile.GetTrustDeviceId());
    trustDeviceProfile.SetDeviceIdType(accessControlProfile.GetDeviceIdType());
    trustDeviceProfile.SetDeviceIdHash(accessControlProfile.GetDeviceIdHash());
    trustDeviceProfile.SetStatus(accessControlProfile.GetStatus());
    trustDeviceProfile.SetBindType(accessControlProfile.GetBindType());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByUserIdAndBundleName(std::shared_ptr<ResultSet> resultSet,
    int32_t userId, const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t>(BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t>(BindLevel::DEVICE)) {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId, accesseeId, userId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId,
                accesseeId, userId, bundleName, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByUserIdAndAccountId(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, int32_t userId, const std::string& accountId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID_ACCESSERACCOUNTID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId), ValueObject(accountId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSEEID_ACCESSEEACCOUNTID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId), ValueObject(accountId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByTokenId(std::shared_ptr<ResultSet> resultSet,
    const std::string& trustDeviceId, int64_t tokenId, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t> (BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t> (BindLevel::DEVICE)) {
            int32_t ret = this->GetAccessControlProfilesByDeviceId(
                resultSet, accesserId, accesseeId, trustDeviceId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfile failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfilesByTokenId(resultSet, accesserId,
                accesseeId, trustDeviceId, tokenId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfilesByTokenId failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAclProfileByBundleName(std::shared_ptr<ResultSet> resultSet,
    const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t columnIndex = COLUMNINDEX_INIT;
        int64_t accesserId = ACCESSERID_INIT;
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        int64_t accesseeId = ACCESSEEID_INIT;
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        int32_t bindType = BINDTYPE_INIT;
        resultSet->GetColumnIndex(BIND_TYPE, columnIndex);
        resultSet->GetInt(columnIndex, bindType);
        int32_t bindLevel = BINDLEVEL_INIT;
        resultSet->GetColumnIndex(BIND_LEVEL, columnIndex);
        resultSet->GetInt(columnIndex, bindLevel);
        if (bindType == static_cast<int32_t> (BindType::SAME_ACCOUNT) ||
            bindLevel == static_cast<int32_t> (BindLevel::DEVICE)) {
            int32_t ret = this->GetAccessControlProfile(resultSet, accesserId, accesseeId, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfile failed");
                return ret;
            }
        } else {
            int32_t ret = this->GetAccessControlProfiles(resultSet, accesserId, accesseeId, bundleName, profile);
            if (ret != DP_SUCCESS) {
                HILOGE("GetAccessControlProfiles failed");
                return ret;
            }
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToAccessControlProfiles(std::shared_ptr<ResultSet> resultSet,
    std::shared_ptr<ResultSet> accesserResultSet, std::shared_ptr<ResultSet> accesseeResultSet,
    std::vector<AccessControlProfile>& profile)
{
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    Accesser accesser;
    accesserResultSet->GoToNextRow();
    this->ConvertToAccesser(accesserResultSet, accesser);
    Accessee accessee;
    accesseeResultSet->GoToNextRow();
    this->ConvertToAccessee(accesseeResultSet, accessee);
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    AccessControlProfile accessControlProfile;
    this->ConvertToAccessControlProfile(resultSet, accessControlProfile);

    accessControlProfile.SetAccesser(accesser);
    accessControlProfile.SetAccessee(accessee);
    profile.push_back(accessControlProfile);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccesserProfile(const AccessControlProfile& profile)
{
    Accesser accesser = profile.GetAccesser();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accesser.GetAccesserDeviceId()), ValueObject(accesser.GetAccesserUserId()),
        ValueObject(accesser.GetAccesserAccountId()), ValueObject(accesser.GetAccesserTokenId()),
        ValueObject(accesser.GetAccesserBundleName()), ValueObject(accesser.GetAccesserHapSignature()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        HILOGI("accesser is exists");
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::AccesserToEntries(profile, values);
    int64_t rowId = ROWID_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Put(rowId, ACCESSER_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("accesser_table insert failed");
            return DP_PUT_ACCESSER_PROFILE_FAIL;
        }
    }
    HILOGI("PutAccesser : %{public}s", profile.GetAccesser().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAccesseeProfile(const AccessControlProfile& profile)
{
    Accessee accessee = profile.GetAccessee();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accessee.GetAccesseeDeviceId()), ValueObject(accessee.GetAccesseeUserId()),
        ValueObject(accessee.GetAccesseeAccountId()), ValueObject(accessee.GetAccesseeTokenId()),
        ValueObject(accessee.GetAccesseeBundleName()), ValueObject(accessee.GetAccesseeHapSignature()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        HILOGI("accessee is exists");
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    ValuesBucket values;
    ProfileUtils::AccesseeToEntries(profile, values);
    int64_t rowId = ROWID_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Put(rowId, ACCESSEE_TABLE, values);
        if (ret != DP_SUCCESS) {
            HILOGE("accessee_table insert failed");
            return DP_PUT_ACCESSEE_PROFILE_FAIL;
        }
    }
    HILOGI("PutAccessee : %{public}s", profile.GetAccessee().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccessControlId(AccessControlProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.SetAccessControlId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    int64_t accessControlId = ACCESSCONTROLID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESS_CONTROL_ID, columnIndex);
    resultSet->GetLong(columnIndex, accessControlId);
    resultSet->Close();
    profile.SetAccessControlId(accessControlId+1);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccesserId(AccessControlProfile& profile)
{
    Accesser accesser = profile.GetAccesser();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accesser.GetAccesserDeviceId()), ValueObject(accesser.GetAccesserUserId()),
        ValueObject(accesser.GetAccesserAccountId()), ValueObject(accesser.GetAccesserTokenId()),
        ValueObject(accesser.GetAccesserBundleName()), ValueObject(accesser.GetAccesserHapSignature()),
        ValueObject(static_cast<int32_t>(accesser.GetAccesserBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    int64_t accesserId = ACCESSERID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesserId);
        profile.SetAccesserId(accesserId);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSER_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.GetAccesser().SetAccesserId(1);
        profile.SetAccesserId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESSER_ID, columnIndex);
    resultSet->GetLong(columnIndex, accesserId);
    resultSet->Close();
    accesserId = accesserId + 1;
    profile.SetAccesserId(accesserId);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::SetAccesseeId(AccessControlProfile& profile)
{
    Accessee accessee = profile.GetAccessee();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(accessee.GetAccesseeDeviceId()), ValueObject(accessee.GetAccesseeUserId()),
        ValueObject(accessee.GetAccesseeAccountId()), ValueObject(accessee.GetAccesseeTokenId()),
        ValueObject(accessee.GetAccesseeBundleName()), ValueObject(accessee.GetAccesseeHapSignature()),
        ValueObject(static_cast<int32_t>(accessee.GetAccesseeBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    int64_t accesseeId = ACCESSEEID_INIT;
    int32_t columnIndex = COLUMNINDEX_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        resultSet->GoToFirstRow();
        resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
        resultSet->GetLong(columnIndex, accesseeId);
        profile.SetAccesseeId(accesseeId);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->Close();
    resultSet = GetResultSet(SELECT_ACCESSEE_TABLE, std::vector<ValueObject> {});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        profile.GetAccessee().SetAccesseeId(1);
        profile.SetAccesseeId(1);
        resultSet->Close();
        return DP_SUCCESS;
    }
    resultSet->GoToLastRow();
    resultSet->GetColumnIndex(ACCESSEE_ID, columnIndex);
    resultSet->GetLong(columnIndex, accesseeId);
    resultSet->Close();
    accesseeId = accesseeId + 1;
    profile.SetAccesseeId(accesseeId);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccesserProfile(int64_t accesserId, const AccessControlProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::AccesserToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Update(changeRowCnt, ACCESSER_TABLE, values, ACCESSERID_EQUAL_CONDITION,
            std::vector<ValueObject> {ValueObject(accesserId)});
        if (ret != DP_SUCCESS) {
            HILOGE("accesser_table update failed");
            return DP_UPDATE_ACCESSER_PROFILE_FAIL;
        }
    }
    HILOGI("UpdateAccesser : %{public}s", profile.GetAccesser().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAccesseeProfile(int64_t accesseeId, const AccessControlProfile& profile)
{
    ValuesBucket values;
    ProfileUtils::AccesseeToEntries(profile, values);
    int32_t changeRowCnt = CHANGEROWCNT_INIT;
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t ret = rdbStore_->Update(changeRowCnt, ACCESSEE_TABLE, values, ACCESSEEID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (ret != DP_SUCCESS) {
            HILOGE("accessee_table update failed");
            return DP_UPDATE_ACCESSEE_PROFILE_FAIL;
        }
    }
    HILOGI("UpdateAccessee : %{public}s", profile.GetAccessee().dump().c_str());
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateTrustDeviceProfileNotify(const TrustDeviceProfile& oldProfile,
    const TrustDeviceProfile &newProfile)
{
    if (oldProfile.GetStatus() == 1 && newProfile.GetStatus() == 0) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileDelete(newProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileDelete failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    if (oldProfile.GetStatus() == 0 && newProfile.GetStatus() == 1) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileAdd(newProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileAdd failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    if (oldProfile.GetDeviceId() != newProfile.GetDeviceId() ||
        oldProfile.GetDeviceIdHash() != newProfile.GetDeviceIdHash() ||
        oldProfile.GetDeviceIdType() != newProfile.GetDeviceIdType()) {
        int32_t ret = SubscribeProfileManager::GetInstance().NotifyTrustDeviceProfileUpdate(oldProfile, newProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("NotifyTrustDeviceProfileUpdate failed");
            return DP_NOTIFY_TRUST_DEVICE_FAIL;
        }
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetResultStatus(const std::string& trustDeviceId, int32_t& trustDeviceStatus)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID,
        std::vector<ValueObject>{ ValueObject(trustDeviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("trustDeviceId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    int32_t columnIndex = COLUMNINDEX_INIT;
    trustDeviceStatus = 0;
    while (resultSet->GoToNextRow() == DP_SUCCESS) {
        int32_t status = STATUS_INIT;
        resultSet->GetColumnIndex(STATUS, columnIndex);
        resultSet->GetInt(columnIndex, status);
        if (status == 1) {
            trustDeviceStatus = 1;
            break;
        }
    }
    resultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfile(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
        std::vector<ValueObject>{ ValueObject(accesserId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("not find data");
        accesserResultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
        std::vector<ValueObject>{ ValueObject(accesseeId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("not find data");
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
    accesserResultSet->Close();
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfilesByDeviceId(
    std::shared_ptr<ResultSet> resultSet, int64_t accesserId, int64_t accesseeId,
    const std::string& trustDeviceId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERDEVICEID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(trustDeviceId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEDEVICEID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(trustDeviceId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccessControlProfileCheck(std::shared_ptr<ResultSet> resultSet)
{
    int32_t ret = resultSet->GoToNextRow();
    if (ret != DP_SUCCESS) {
        HILOGE("get AccessControlProfileResult failed");
        return DP_NOT_FIND_DATA;
    }
    AccessControlProfile profile;
    this->ConvertToAccessControlProfile(resultSet, profile);
    resultSet->Close();

    ret = this->DeleteAccesseeCheck(profile.GetAccesseeId());
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteAccesseeCheck failed");
        return ret;
    }
    ret = this->DeleteAccesserCheck(profile.GetAccesserId());
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteAccesserCheck failed");
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        ret = rdbStore_->Delete(deleteRows, ACCESS_CONTROL_TABLE, ACCESSCONTROLID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete access_control_table failed");
            return DP_DELETE_ACCESS_CONTROL_PROFILE_FAIL;
        }
    }
    HILOGI("DeleteAclProfile : %{public}s", profile.dump().c_str());
    ret = this->DeleteTrustDeviceCheck(profile);
    if (ret != DP_SUCCESS) {
        HILOGE("DeleteTrustDeviceCheck failed");
        return ret;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToTrustDeviceProfile(
    std::shared_ptr<ResultSet> trustResultSet, TrustDeviceProfile& trustDeviceProfile)
{
    if (trustResultSet == nullptr) {
        HILOGE("trustResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    RowEntity rowEntity;
    if (trustResultSet->GetRow(rowEntity) != DP_SUCCESS) {
        HILOGE("get trustResultSet failed");
        return DP_GET_RESULTSET_FAIL;
    }
    std::string deviceId = rowEntity.Get(DEVICE_ID);
    int32_t deviceIdType = rowEntity.Get(DEVICE_ID_TYPE);
    std::string deviceIdHash = rowEntity.Get(DEVICE_ID_HASH);
    int32_t status = rowEntity.Get(STATUS);

    trustDeviceProfile.SetDeviceId(deviceId);
    trustDeviceProfile.SetDeviceIdType(deviceIdType);
    trustDeviceProfile.SetDeviceIdHash(deviceIdHash);
    trustDeviceProfile.SetStatus(status);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToAccesser(std::shared_ptr<ResultSet> accesserResultSet,
    Accesser& accesser)
{
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    RowEntity rowEntity;
    if (accesserResultSet->GetRow(rowEntity) != DP_SUCCESS) {
        HILOGE("get accesserResultSet failed");
        return DP_GET_RESULTSET_FAIL;
    }
    int64_t accesserId = rowEntity.Get(ACCESSER_ID);
    std::string accesserDeviceId = rowEntity.Get(ACCESSER_DEVICE_ID);
    int32_t accesserUserId = rowEntity.Get(ACCESSER_USER_ID);
    std::string accesserAccountId = rowEntity.Get(ACCESSER_ACCOUNT_ID);
    int64_t accesserTokenId = rowEntity.Get(ACCESSER_TOKEN_ID);
    std::string accesserBundleName = rowEntity.Get(ACCESSER_BUNDLE_NAME);
    std::string accesserHapSignature = rowEntity.Get(ACCESSER_HAP_SIGNATURE);
    int32_t accesserBindLevel = rowEntity.Get(ACCESSER_BIND_LEVEL);

    accesser.SetAccesserId(accesserId);
    accesser.SetAccesserDeviceId(accesserDeviceId);
    accesser.SetAccesserUserId(accesserUserId);
    accesser.SetAccesserAccountId(accesserAccountId);
    accesser.SetAccesserTokenId(accesserTokenId);
    accesser.SetAccesserBundleName(accesserBundleName);
    accesser.SetAccesserHapSignature(accesserHapSignature);
    accesser.SetAccesserBindLevel(accesserBindLevel);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToAccessee(std::shared_ptr<ResultSet> accesseeResultSet,
    Accessee& accessee)
{
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    RowEntity rowEntity;
    if (accesseeResultSet->GetRow(rowEntity) != DP_SUCCESS) {
        HILOGE("get accesseeResultSet failed");
        return DP_GET_RESULTSET_FAIL;
    }
    int64_t accesseeId = rowEntity.Get(ACCESSEE_ID);
    std::string accesseeDeviceId = rowEntity.Get(ACCESSEE_DEVICE_ID);
    int32_t accesseeUserId = rowEntity.Get(ACCESSEE_USER_ID);
    std::string accesseeAccountId = rowEntity.Get(ACCESSEE_ACCOUNT_ID);
    int64_t accesseeTokenId = rowEntity.Get(ACCESSEE_TOKEN_ID);
    std::string accesseeBundleName = rowEntity.Get(ACCESSEE_BUNDLE_NAME);
    std::string accesseeHapSignature = rowEntity.Get(ACCESSEE_HAP_SIGNATURE);
    int32_t accesseeBindLevel = rowEntity.Get(ACCESSEE_BIND_LEVEL);

    accessee.SetAccesseeId(accesseeId);
    accessee.SetAccesseeDeviceId(accesseeDeviceId);
    accessee.SetAccesseeUserId(accesseeUserId);
    accessee.SetAccesseeAccountId(accesseeAccountId);
    accessee.SetAccesseeTokenId(accesseeTokenId);
    accessee.SetAccesseeBundleName(accesseeBundleName);
    accessee.SetAccesseeHapSignature(accesseeHapSignature);
    accessee.SetAccesseeBindLevel(accesseeBindLevel);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::ConvertToAccessControlProfile(
    std::shared_ptr<ResultSet> accessControlResultSet, AccessControlProfile& accessControlProfile)
{
    if (accessControlResultSet == nullptr) {
        HILOGE("accessControlResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    RowEntity rowEntity;
    if (accessControlResultSet->GetRow(rowEntity) != DP_SUCCESS) {
        HILOGE("get accessControlResultSet failed");
        return DP_GET_RESULTSET_FAIL;
    }
    int64_t accessControlId = rowEntity.Get(ACCESS_CONTROL_ID);
    int64_t accesserId = rowEntity.Get(ACCESSER_ID);
    int64_t accesseeId = rowEntity.Get(ACCESSEE_ID);
    std::string trustDeviceId = rowEntity.Get(TRUST_DEVICE_ID);
    std::string sessionKey = rowEntity.Get(SESSION_KEY);
    int32_t bindType = rowEntity.Get(BIND_TYPE);
    int32_t authenticationType = rowEntity.Get(AUTHENTICATION_TYPE);
    int32_t deviceIdType = rowEntity.Get(DEVICE_ID_TYPE);
    std::string deviceIdHash = rowEntity.Get(DEVICE_ID_HASH);
    int32_t status = rowEntity.Get(STATUS);
    int32_t validPeriod = rowEntity.Get(VALID_PERIOD);
    int32_t lastAuthTime = rowEntity.Get(LAST_AUTH_TIME);
    int32_t bindLevel = rowEntity.Get(BIND_LEVEL);

    accessControlProfile.SetAccessControlId(accessControlId);
    accessControlProfile.SetAccesserId(accesserId);
    accessControlProfile.SetAccesseeId(accesseeId);
    accessControlProfile.SetTrustDeviceId(trustDeviceId);
    accessControlProfile.SetSessionKey(sessionKey);
    accessControlProfile.SetBindType(bindType);
    accessControlProfile.SetAuthenticationType(authenticationType);
    accessControlProfile.SetDeviceIdType(deviceIdType);
    accessControlProfile.SetDeviceIdHash(deviceIdHash);
    accessControlProfile.SetStatus(status);
    accessControlProfile.SetValidPeriod(validPeriod);
    accessControlProfile.SetLastAuthTime(lastAuthTime);
    accessControlProfile.SetBindLevel(bindLevel);
    return DP_SUCCESS;
}

std::shared_ptr<ResultSet> TrustProfileManager::GetResultSet(
    const std::string& sql, std::vector<ValueObject> condition)
{
    if (sql.empty() || sql.length() > MAX_STRING_LEN) {
        HILOGE("sql is invalid");
        return nullptr;
    }
    if (condition.size() > MAX_PARAM_SIZE) {
        HILOGE("condition is invalid");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        HILOGE("rdbStore_ is nullptr");
        return nullptr;
    }
    return rdbStore_->Get(sql, condition);
}

int32_t TrustProfileManager::SetAccessControlProfileId(AccessControlProfile& accessControlProfile)
{
    int32_t ret = this->SetAccessControlId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccessControlId failed");
        return ret;
    }
    ret = this->SetAccesserId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccesserId failed");
        return ret;
    }
    ret = this->SetAccesseeId(accessControlProfile);
    if (ret != DP_SUCCESS) {
        HILOGE("SetAccesseeId failed");
        return ret;
    }
    Accesser accesser(accessControlProfile.GetAccesser());
    accesser.SetAccesserId(accessControlProfile.GetAccesserId());
    accessControlProfile.SetAccesser(accesser);

    Accessee accessee(accessControlProfile.GetAccessee());
    accessee.SetAccesseeId(accessControlProfile.GetAccesseeId());
    accessControlProfile.SetAccessee(accessee);
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, int32_t userId, const std::string& bundleName,
    std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID_ACCESSERBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId), ValueObject(bundleName) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSEEID_ACCESSEEBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId), ValueObject(bundleName) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, int32_t userId, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERUSERID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(userId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEUSERID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(userId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfiles(std::shared_ptr<ResultSet> resultSet, int64_t accesserId,
    int64_t accesseeId, const std::string& bundleName, std::vector<AccessControlProfile>& profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_ACCESSERBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(bundleName) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_ACCESSEEBUNDLENAME,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(bundleName) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::GetAccessControlProfilesByTokenId(std::shared_ptr<ResultSet> resultSet,
    int64_t accesserId, int64_t accesseeId, const std::string& trustDeviceId,
    int64_t tokenId, std::vector<AccessControlProfile> &profile)
{
    std::shared_ptr<ResultSet> accesserResultSet =
        GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID_AND_DEVICEID_AND_ACCESSERTOKENID,
        std::vector<ValueObject>{ ValueObject(accesserId), ValueObject(trustDeviceId), ValueObject(tokenId) });
    if (accesserResultSet == nullptr) {
        HILOGE("accesserResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    accesserResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        std::shared_ptr<ResultSet> accesseeResultSet =
            GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID,
            std::vector<ValueObject>{ ValueObject(accesseeId) });
        if (accesseeResultSet == nullptr) {
            HILOGE("accesseeResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
        accesseeResultSet->Close();
        return DP_SUCCESS;
    }
    accesserResultSet->Close();

    std::shared_ptr<ResultSet> accesseeResultSet =
        GetResultSet(SELECT_ACCESSEE_TABLE_WHERE_ACCESSEEID_AND_DEVICEID_AND_ACCESSEETOKENID,
        std::vector<ValueObject>{ ValueObject(accesseeId), ValueObject(trustDeviceId), ValueObject(tokenId) });
    if (accesseeResultSet == nullptr) {
        HILOGE("accesseeResultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    accesseeResultSet->GetRowCount(rowCount);
    if (rowCount != 0) {
        accesserResultSet = GetResultSet(SELECT_ACCESSER_TABLE_WHERE_ACCESSERID,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (accesserResultSet == nullptr) {
            HILOGE("accesserResultSet is nullptr");
            return DP_GET_RESULTSET_FAIL;
        }
        this->ConvertToAccessControlProfiles(resultSet, accesserResultSet, accesseeResultSet, profile);
        accesserResultSet->Close();
    }
    accesseeResultSet->Close();
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccesserCheck(int64_t accesserId)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSERID,
        std::vector<ValueObject>{ ValueObject(accesserId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount == DELETE_ACCESSER_CONDITION) {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        int32_t ret = rdbStore_->Delete(deleteRows, ACCESSER_TABLE, ACCESSERID_EQUAL_CONDITION,
            std::vector<ValueObject>{ ValueObject(accesserId) });
        if (ret != DP_SUCCESS) {
            HILOGE("delete accesser_table accesserId failed");
            return DP_DELETE_ACCESSER_PROFILE_FAIL;
        }
        HILOGI("DeleteAccesserId : %{public}" PRId64, accesserId);
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::UpdateAclCheck(const AccessControlProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSCONTROLID,
        std::vector<ValueObject>{ ValueObject(profile.GetAccessControlId()) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    if (rowCount == 0) {
        HILOGE("accessControlId not find");
        resultSet->Close();
        return DP_NOT_FIND_DATA;
    }
    resultSet->GoToNextRow();
    AccessControlProfile oldProfile;
    this->ConvertToAccessControlProfile(resultSet, oldProfile);
    resultSet->Close();
    if (oldProfile.GetAccesseeId() != profile.GetAccessee().GetAccesseeId() ||
        oldProfile.GetAccesserId() != profile.GetAccesser().GetAccesserId() ||
        oldProfile.GetAccesserId() != profile.GetAccesserId() ||
        oldProfile.GetAccesseeId() != profile.GetAccesseeId()) {
        HILOGE("UpdateAclCheck:Can't Update not allowed attribute");
        return DP_UPDATE_ACL_NOT_ALLOW;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::PutAclCheck(const AccessControlProfile& profile)
{
    TrustDeviceProfile trustProfile;
    this->ConvertToTrustDeviceProfile(profile, trustProfile);
    std::string trustDeviceId = profile.GetTrustDeviceId();
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_TRUST_DEVICE_TABLE_WHERE_DEVICEID,
        std::vector<ValueObject>{ ValueObject(trustDeviceId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount == 0) {
        this->PutTrustDeviceProfile(trustProfile);
        return DP_SUCCESS;
    }
    int32_t status = STATUS_INIT;
    if (this->GetResultStatus(trustDeviceId, status) != DP_SUCCESS) {
        HILOGE("GetResultStatus failed");
        return DP_GET_RESULTSET_FAIL;
    }
    trustProfile.SetStatus(status);
    if (this->UpdateTrustDeviceProfile(trustProfile) != DP_SUCCESS) {
        HILOGE("UpdateTrustDeviceProfile failed");
        return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::IsAclExists(const AccessControlProfile &profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ALL, std::vector<ValueObject>{
        ValueObject(profile.GetAccesserId()), ValueObject(profile.GetAccesseeId()),
        ValueObject(profile.GetTrustDeviceId()), ValueObject(profile.GetSessionKey()),
        ValueObject(static_cast<int32_t>(profile.GetBindType())),
        ValueObject(static_cast<int32_t>(profile.GetAuthenticationType())),
        ValueObject(static_cast<int32_t>(profile.GetDeviceIdType())),
        ValueObject(profile.GetDeviceIdHash()), ValueObject(profile.GetStatus()),
        ValueObject(profile.GetValidPeriod()), ValueObject(profile.GetLastAuthTime()),
        ValueObject(static_cast<int32_t>(profile.GetBindLevel()))});
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount != 0) {
        HILOGI("accessControlProfile is exists");
        return DP_DATA_EXISTS;
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteAccesseeCheck(int64_t accesseeId)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_ACCESSEEID,
        std::vector<ValueObject>{ ValueObject(accesseeId) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    if (rowCount == DELETE_ACCESSEE_CONDITION) {
        std::lock_guard<std::mutex> lock(rdbMutex_);
        if (rdbStore_ == nullptr) {
            HILOGE("rdbStore_ is nullptr");
            return DP_GET_RDBSTORE_FAIL;
        }
        int32_t deleteRows = DELETEROWS_INIT;
        int32_t ret = rdbStore_->Delete(deleteRows, ACCESSEE_TABLE, ACCESSEEID_EQUAL_CONDITION,
            std::vector<ValueObject> {ValueObject(accesseeId)});
        if (ret != DP_SUCCESS) {
            HILOGE("delete accessee_table accesseeId failed");
            return DP_DELETE_ACCESSEE_PROFILE_FAIL;
        }
        HILOGI("DeleteAccesseeId : %{public}" PRId64, accesseeId);
    }
    return DP_SUCCESS;
}

int32_t TrustProfileManager::DeleteTrustDeviceCheck(const AccessControlProfile& profile)
{
    std::shared_ptr<ResultSet> resultSet =
        GetResultSet(SELECT_ACCESS_CONTROL_TABLE_WHERE_TRUSTDEVICEID,
        std::vector<ValueObject>{ ValueObject(profile.GetTrustDeviceId()) });
    if (resultSet == nullptr) {
        HILOGE("resultSet is nullptr");
        return DP_GET_RESULTSET_FAIL;
    }
    int32_t rowCount = ROWCOUNT_INIT;
    resultSet->GetRowCount(rowCount);
    resultSet->Close();
    int32_t ret = RET_INIT;
    if (rowCount == DELETE_TRUST_CONDITION) {
        ret = this->DeleteTrustDeviceProfile(profile.GetTrustDeviceId());
        if (ret != DP_SUCCESS) {
            HILOGE("DeleteTrustDeviceProfile failed");
            return DP_DELETE_TRUST_DEVICE_PROFILE_FAIL;
        }
    } else {
        int32_t status = STATUS_INIT;
        this->GetResultStatus(profile.GetTrustDeviceId(), status);
        TrustDeviceProfile trustDeviceProfile;
        trustDeviceProfile.SetDeviceId(profile.GetTrustDeviceId());
        trustDeviceProfile.SetDeviceIdType(profile.GetDeviceIdType());
        trustDeviceProfile.SetDeviceIdHash(profile.GetDeviceIdHash());
        trustDeviceProfile.SetStatus(status);
        ret = this->UpdateTrustDeviceProfile(trustDeviceProfile);
        if (ret != DP_SUCCESS) {
            HILOGE("UpdateTrustDeviceProfile failed");
            return DP_UPDATE_TRUST_DEVICE_PROFILE_FAIL;
        }
    }
    return DP_SUCCESS;
}
} // namespace DistributedDeviceProfile
} // namespace OHOS