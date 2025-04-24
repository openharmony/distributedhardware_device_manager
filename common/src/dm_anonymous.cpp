/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dm_anonymous.h"
#include "dm_log.h"
#include <sstream>
namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
constexpr uint32_t MAX_MAP_LEN = 1000;
constexpr uint32_t MAX_INT_LEN = 20;
constexpr uint32_t MAX_ID_LEN = 256;
}
const char* PRINT_LIST_SPLIT = ", ";
const int32_t LIST_SPLIT_LEN = 2;

std::string GetAnonyString(const std::string &value)
{
    const int32_t INT32_SHORT_ID_LENGTH = 20;
    const int32_t INT32_PLAINTEXT_LENGTH = 4;
    const int32_t INT32_MIN_ID_LENGTH = 3;

    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    std::string res;
    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

std::string GetAnonyStringList(const std::vector<std::string> &values)
{
    std::string temp = "[ ";
    bool flag = false;
    for (auto const &v : values) {
        temp += GetAnonyString(v) + PRINT_LIST_SPLIT;
        flag = true;
    }
    if (flag) {
        temp.erase(temp.length() - LIST_SPLIT_LEN);
    }
    temp += " ]";
    return temp;
}

std::string GetAnonyInt32(const int32_t value)
{
    std::string tempString = std::to_string(value);
    size_t length = tempString.length();
    if (length == 0x01) {
        tempString[0] = '*';
        return tempString;
    }
    for (size_t i = 1; i < length - 1; i++) {
        tempString[i] = '*';
    }
    return tempString;
}

std::string GetAnonyInt32List(const std::vector<int32_t> &values)
{
    std::string temp = "[ ";
    bool flag = false;
    for (auto const &v : values) {
        temp += GetAnonyInt32(v) + PRINT_LIST_SPLIT;
        flag = true;
    }
    if (flag) {
        temp.erase(temp.length() - LIST_SPLIT_LEN);
    }
    temp += " ]";
    return temp;
}

bool IsNumberString(const std::string &inputString)
{
    if (inputString.length() == 0 || inputString.length() > MAX_INT_LEN) {
        LOGE("inputString is Null or inputString length is too long");
        return false;
    }
    const int32_t MIN_ASCII_NUM = 48;
    const int32_t MAX_ASCII_NUM = 57;
    for (size_t i = 0; i < inputString.length(); i++) {
        int num = (int)inputString[i];
        if (num >= MIN_ASCII_NUM && num <= MAX_ASCII_NUM) {
            continue;
        } else {
            return false;
        }
    }
    return true;
}

bool IsString(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = (jsonObj.Contains(key) && jsonObj[key].IsString()
        && jsonObj[key].Get<std::string>().size() <= MAX_MESSAGE_LEN);
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt32(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= INT32_MIN &&
        jsonObj[key].Get<int64_t>() <= INT32_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUint32(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= 0 &&
        jsonObj[key].Get<int64_t>() <= UINT32_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsInt64(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<int64_t>() >= INT64_MIN &&
        jsonObj[key].Get<int64_t>() <= INT64_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsUint64(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsNumberInteger() && jsonObj[key].Get<uint64_t>() >= 0 &&
        jsonObj[key].Get<uint64_t>() <= UINT64_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsArray(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsArray();
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

bool IsBool(const JsonItemObject &jsonObj, const std::string &key)
{
    bool res = jsonObj.Contains(key) && jsonObj[key].IsBoolean();
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}

std::string ConvertMapToJsonString(const std::map<std::string, std::string> &paramMap)
{
    std::string jsonStr = "";
    if (paramMap.size() > MAX_MAP_LEN) {
        LOGE("invalid paramMap");
        return jsonStr;
    }
    if (!paramMap.empty()) {
        JsonObject jsonObj;
        for (const auto &it : paramMap) {
            jsonObj[it.first] = it.second;
        }
        jsonStr = SafetyDump(jsonObj);
    }
    return jsonStr;
}

void ParseMapFromJsonString(const std::string &jsonStr, std::map<std::string, std::string> &paramMap)
{
    if (jsonStr.empty()) {
        return;
    }
    if (paramMap.size() > MAX_MAP_LEN) {
        LOGE("invalid paramMap");
        return;
    }
    JsonObject paramJson(jsonStr);
    if (paramJson.IsDiscarded()) {
        return;
    }
    for (auto &element : paramJson.Items()) {
        if (element.IsString()) {
            paramMap.insert(std::pair<std::string, std::string>(element.Key(), element.Get<std::string>()));
        }
    }
}

bool IsInvalidPeerTargetId(const PeerTargetId &targetId)
{
    return targetId.deviceId.empty() && targetId.brMac.empty() && targetId.bleMac.empty() && targetId.wifiIp.empty();
}

std::string ConvertCharArray2String(const char *srcData, uint32_t srcLen)
{
    if (srcData == nullptr || srcLen == 0 || srcLen >= MAX_MESSAGE_LEN) {
        LOGE("Invalid parameter.");
        return "";
    }
    char *dstData = new char[srcLen + 1]();
    if (memcpy_s(dstData, srcLen + 1, srcData, srcLen) != 0) {
        LOGE("memcpy_s failed.");
        delete[] dstData;
        return "";
    }
    std::string temp(dstData);
    delete[] dstData;
    return temp;
}

int32_t StringToInt(const std::string &str, int32_t base)
{
    if (str.empty()) {
        LOGE("Str is empty.");
        return 0;
    }
    char *nextPtr = nullptr;
    long result = strtol(str.c_str(), &nextPtr, base);
    if (errno == ERANGE || *nextPtr != '\0') {
        LOGE("parse int error");
        return 0;
    }
    return static_cast<int32_t>(result);
}

int64_t StringToInt64(const std::string &str, int32_t base)
{
    if (str.empty()) {
        LOGE("Str is empty.");
        return 0;
    }
    char *nextPtr = nullptr;
    int64_t result = strtoll(str.c_str(), &nextPtr, base);
    if (errno == ERANGE || nextPtr == nullptr || nextPtr == str.c_str() || *nextPtr != '\0') {
        LOGE("parse int error");
        return 0;
    }
    return result;
}

void VersionSplitToInt(const std::string &str, const char split, std::vector<int> &numVec)
{
    std::istringstream iss(str);
    std::string item = "";
    while (getline(iss, item, split)) {
        numVec.push_back(atoi(item.c_str()));
    }
}

bool CompareVecNum(const std::vector<int32_t> &srcVecNum, const std::vector<int32_t> &sinkVecNum)
{
    for (uint32_t index = 0; index < std::min(srcVecNum.size(), sinkVecNum.size()); index++) {
        if (srcVecNum[index] > sinkVecNum[index]) {
            return true;
        } else if (srcVecNum[index] < sinkVecNum[index]) {
            return false;
        } else {
            continue;
        }
    }
    if (srcVecNum.size() > sinkVecNum.size()) {
        return true;
    }
    return false;
}

bool CompareVersion(const std::string &remoteVersion, const std::string &oldVersion)
{
    LOGI("remoteVersion %{public}s, oldVersion %{public}s.", remoteVersion.c_str(), oldVersion.c_str());
    std::vector<int32_t> remoteVersionVec;
    std::vector<int32_t> oldVersionVec;
    VersionSplitToInt(remoteVersion, '.', remoteVersionVec);
    VersionSplitToInt(oldVersion, '.', oldVersionVec);
    return CompareVecNum(remoteVersionVec, oldVersionVec);
}

bool GetVersionNumber(const std::string dmVersion, int32_t &versionNum)
{
    LOGI("dmVersion %{public}s,", dmVersion.c_str());
    std::string number = "";
    std::istringstream iss(dmVersion);
    std::string item = "";
    while (getline(iss, item, '.')) {
        number += item;
    }
    LOGI("number %{public}s,", number.c_str());
    versionNum = atoi(number.c_str());
    if (versionNum <= 0) {
        LOGE("convert failed, number: %{public}s,", number.c_str());
        return false;
    }
    return true;
}

std::string ComposeStr(const std::string &pkgName, uint16_t subscribeId)
{
    std::string strTemp = pkgName + "#" + std::to_string(subscribeId);
    return strTemp;
}

std::string GetCallerPkgName(const std::string &pkgName)
{
    std::istringstream stream(pkgName);
    std::string item = "";
    getline(stream, item, '#');
    return item;
}

uint16_t GetSubscribeId(const std::string &pkgName)
{
    std::vector<std::string> strVec;
    size_t subIdIndex = 1;
    size_t start = 0;
    size_t end = pkgName.find("#");

    while (end != std::string::npos) {
        strVec.push_back(pkgName.substr(start, end - start));
        start = end + 1;
        end = pkgName.find("#", start);
    }
    strVec.push_back(pkgName.substr(start));
    if (strVec.size() >= subIdIndex + 1) {
        return std::atoi(strVec.at(subIdIndex).c_str());
    }
    return 0;
}

bool IsIdLengthValid(const std::string &inputID)
{
    if (inputID.empty() || inputID.length() > MAX_ID_LEN) {
        LOGE("On parameter length error, maybe empty or beyond MAX_ID_LEN!");
        return false;
    }
    return true;
}

bool IsMessageLengthValid(const std::string &inputMessage)
{
    if (inputMessage.empty() || inputMessage.length() > MAX_MESSAGE_LEN) {
        LOGE("On parameter error, maybe empty or beyond MAX_MESSAGE_LEN!");
        return false;
    }
    return true;
}

bool IsValueExist(const std::multimap<std::string, int32_t> unorderedmap, const std::string &udid, int32_t userId)
{
    for (const auto &item : unorderedmap) {
        if (item.first == udid && item.second == userId) {
            return true;
        }
    }
    return false;
}

bool IsDmCommonNotifyEventValid(DmCommonNotifyEvent dmCommonNotifyEvent)
{
    if (dmCommonNotifyEvent > DmCommonNotifyEvent::MIN && dmCommonNotifyEvent < DmCommonNotifyEvent::MAX) {
        return true;
    }
    return false;
}

DM_EXPORT std::string SafetyDump(const JsonItemObject &jsonObj)
{
    return jsonObj.Dump();
}

std::string GetSubStr(const std::string &rawStr, const std::string &separator, int32_t index)
{
    if (rawStr.empty() || separator.empty() || index < 0) {
        LOGE("param invalid");
        return "";
    }
    std::vector<std::string> strVec;
    size_t start = 0;
    size_t end = rawStr.find(separator);

    while (end != std::string::npos) {
        strVec.push_back(rawStr.substr(start, end - start));
        start = end + separator.size();
        end = rawStr.find(separator, start);
    }
    strVec.push_back(rawStr.substr(start));
    if (strVec.size() >= static_cast<size_t>(index + 1)) {
        return strVec.at(index);
    }
    LOGE("get failed");
    return "";
}

bool IsJsonValIntegerString(const JsonItemObject &jsonObj, const std::string &key)
{
    if (!IsString(jsonObj, key)) {
        LOGE("%{public}s is not string", key.c_str());
        return false;
    }
    std::string retValStr = jsonObj[key].Get<std::string>();
    if (!IsNumberString(retValStr)) {
        LOGE("%{public}s is not number", key.c_str());
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS