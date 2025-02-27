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

#include "datashare_result_set_mock.h"

namespace OHOS {
namespace DataShare {
DataShareAbsResultSet::DataShareAbsResultSet()
{}

DataShareAbsResultSet::~DataShareAbsResultSet() {}

int DataShareAbsResultSet::GetRowCount(int &count)
{
    return E_OK;
}

int DataShareAbsResultSet::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    return E_OK;
}

int DataShareAbsResultSet::GetBlob(int columnIndex, std::vector<uint8_t> &blob)
{
    return E_OK;
}

int DataShareAbsResultSet::GetString(int columnIndex, std::string &value)
{
    return E_OK;
}

int DataShareAbsResultSet::GetInt(int columnIndex, int &value)
{
    return E_OK;
}

int DataShareAbsResultSet::GetLong(int columnIndex, int64_t &value)
{
    return E_OK;
}

int DataShareAbsResultSet::GetDouble(int columnIndex, double &value)
{
    return E_OK;
}

int DataShareAbsResultSet::IsColumnNull(int columnIndex, bool &isNull)
{
    return E_OK;
}

int DataShareAbsResultSet::GoToRow(int position)
{
    return E_OK;
}

int DataShareAbsResultSet::GetDataType(int columnIndex, DataType &dataType)
{
    return E_OK;
}

int DataShareAbsResultSet::GetRowIndex(int &position) const
{
    return E_OK;
}

int DataShareAbsResultSet::GoTo(int offset)
{
    return E_OK;
}

int DataShareAbsResultSet::GoToFirstRow()
{
    return E_OK;
}

int DataShareAbsResultSet::GoToLastRow()
{
    return E_OK;
}

int DataShareAbsResultSet::GoToNextRow()
{
    return E_OK;
}

int DataShareAbsResultSet::GoToPreviousRow()
{
    return E_OK;
}

int DataShareAbsResultSet::IsAtFirstRow(bool &result) const
{
    return E_OK;
}

int DataShareAbsResultSet::IsAtLastRow(bool &result)
{
    return E_OK;
}

int DataShareAbsResultSet::IsStarted(bool &result) const
{
    return E_OK;
}

int DataShareAbsResultSet::IsEnded(bool &result)
{
    return E_OK;
}

int DataShareAbsResultSet::GetColumnCount(int &count)
{
    return E_OK;
}

int DataShareAbsResultSet::GetColumnIndex(const std::string &columnName, int &columnIndex)
{
    return E_OK;
}

int DataShareAbsResultSet::GetColumnName(int columnIndex, std::string &columnName)
{
    return E_OK;
}

bool DataShareAbsResultSet::IsClosed() const
{
    return true;
}

int DataShareAbsResultSet::Close()
{
    return E_OK;
}
} // namespace DataShare
} // namespace OHOS
