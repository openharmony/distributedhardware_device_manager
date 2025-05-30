/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DM_DATASHARE_HELPER_MOCK_H
#define OHOS_DM_DATASHARE_HELPER_MOCK_H

#include <gmock/gmock.h>
#include "datashare_helper.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::AAFwk::IDataAbilityObserver;
using OHOS::DataShare::BatchUpdateResult;
using OHOS::DataShare::Data;
using OHOS::DataShare::DatashareBusinessError;
using OHOS::DataShare::DataShareHelper;
using OHOS::DataShare::DataSharePredicates;
using OHOS::DataShare::DataShareResultSet;
using OHOS::DataShare::DataShareValuesBucket;
using OHOS::DataShare::OperationStatement;
using OHOS::DataShare::ExecResultSet;
using OHOS::DataShare::OperationResult;
using OHOS::DataShare::PublishedDataChangeNode;
using OHOS::DataShare::RdbChangeNode;
using OHOS::DataShare::Template;
using OHOS::DataShare::TemplateId;
using OHOS::DataShare::UpdateOperations;
using namespace OHOS::DataShare;

class DataShareHelperMock : public DataShareHelper {
public:
    static std::shared_ptr<DataShareHelperMock> GetOrCreateInstance();
    static void ReleaseInstance();

    MOCK_METHOD(bool, Release, ());
    MOCK_METHOD(std::vector<std::string>, GetFileTypes, (Uri &, const std::string &));
    MOCK_METHOD(int, OpenFile, (Uri &, const std::string &));
    MOCK_METHOD(int, OpenRawFile, (Uri &, const std::string &));
    MOCK_METHOD(int, Insert, (Uri &, const DataShareValuesBucket &));
    MOCK_METHOD(int, InsertExt, (Uri &, const DataShareValuesBucket &, std::string &));
    MOCK_METHOD(int, Update, (Uri &, const DataSharePredicates &, const DataShareValuesBucket &));
    MOCK_METHOD(int, BatchUpdate, (const UpdateOperations &, std::vector<BatchUpdateResult> &));
    MOCK_METHOD(int, Delete, (Uri &, const DataSharePredicates &));
    MOCK_METHOD(std::shared_ptr<DataShareResultSet>, Query,
        (Uri &, const DataSharePredicates &, std::vector<std::string> &, DatashareBusinessError *));
    MOCK_METHOD(std::string, GetType, (Uri &));
    MOCK_METHOD(int, BatchInsert, (Uri &, const std::vector<DataShareValuesBucket> &));
    MOCK_METHOD(int, ExecuteBatch, (const std::vector<OperationStatement> &, ExecResultSet &));
    MOCK_METHOD(int, RegisterObserver, (const Uri &, const sptr<IDataAbilityObserver> &));
    MOCK_METHOD(int, UnregisterObserver, (const Uri &, const sptr<IDataAbilityObserver> &));
    MOCK_METHOD(void, NotifyChange, (const Uri &));
    MOCK_METHOD(
        void, RegisterObserverExtProvider, (const Uri &uri, const std::shared_ptr<DataShareObserver> dataObserver,
            bool isDescendants), (override));
    MOCK_METHOD(void, UnregisterObserverExtProvider, (const Uri &uri,
        const std::shared_ptr<DataShareObserver> dataObserver), (override));
    MOCK_METHOD(void, NotifyChangeExtProvider, (const DataShareObserver::ChangeInfo &changeInfo), (override));
    MOCK_METHOD(Uri, NormalizeUri, (Uri &));
    MOCK_METHOD(Uri, DenormalizeUri, (Uri &));
    MOCK_METHOD(int, AddQueryTemplate, (const std::string &, int64_t, Template &));
    MOCK_METHOD(int, DelQueryTemplate, (const std::string &, int64_t));
    MOCK_METHOD(std::vector<OperationResult>, Publish, (const Data &, const std::string &));
    MOCK_METHOD(Data, GetPublishedData, (const std::string &, int &));
    MOCK_METHOD(std::vector<OperationResult>, SubscribeRdbData, (const std::vector<std::string> &,
        const TemplateId &, const std::function<void(const RdbChangeNode &)> &));
    MOCK_METHOD(std::vector<OperationResult>, UnsubscribeRdbData, (const std::vector<std::string> &,
        const TemplateId &));
    MOCK_METHOD(std::vector<OperationResult>, EnableRdbSubs, (const std::vector<std::string> &,
        const TemplateId &));
    MOCK_METHOD(std::vector<OperationResult>, DisableRdbSubs, (const std::vector<std::string> &,
        const TemplateId &));
    MOCK_METHOD(std::vector<OperationResult>, SubscribePublishedData, (const std::vector<std::string> &, int64_t,
        const std::function<void(const PublishedDataChangeNode &)> &));
    MOCK_METHOD(std::vector<OperationResult>, UnsubscribePublishedData, (const std::vector<std::string> &, int64_t));
    MOCK_METHOD(std::vector<OperationResult>, EnablePubSubs, (const std::vector<std::string> &, int64_t));
    MOCK_METHOD(std::vector<OperationResult>, DisablePubSubs, (const std::vector<std::string> &, int64_t));
private:
    DataShareHelperMock() = default;
    ~DataShareHelperMock() override = default;
    static std::shared_ptr<DataShareHelperMock> instance_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DM_DATASHARE_HELPER_MOCK_H
