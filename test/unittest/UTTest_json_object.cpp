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

#include "UTTest_json_object.h"

namespace OHOS {
namespace DistributedHardware {
struct TestJsonStru {
    int32_t value1 = 0;
    std::string value2 = "";
    bool value3 = false;
};

void ToJson(JsonItemObject &itemObject, const TestJsonStru &testObj)
{
    itemObject["value1"] = testObj.value1;
    itemObject["value2"] = testObj.value2;
    itemObject["value3"] = testObj.value3;
}

void FromJson(const JsonItemObject &itemObject, TestJsonStru &testObj)
{
    if (itemObject.Contains("value1") && itemObject["value1"].IsNumberInteger()) {
        testObj.value1 = itemObject["value1"].Get<int32_t>();
    }
    if (itemObject.Contains("value2") && itemObject["value2"].IsString()) {
        testObj.value2 = itemObject["value2"].Get<std::string>();
    }
    if (itemObject.Contains("value3") && itemObject["value3"].IsBoolean()) {
        testObj.value3 = itemObject["value3"].Get<bool>();
    }
}

bool CheckJsonString(const std::vector<std::string>& checkJsons, const std::string& strJson)
{
    for (const auto& checkStr : checkJsons) {
        if (checkStr == strJson) {
            return true;
        }
    }
    return false;
}

void JsonObjectTest::SetUp()
{
}

void JsonObjectTest::TearDown()
{
}

void JsonObjectTest::SetUpTestCase()
{
}

void JsonObjectTest::TearDownTestCase()
{
}

HWTEST_F(JsonObjectTest, SetValue_001, testing::ext::TestSize.Level1)
{
    uint8_t value1 = 200;
    int16_t value2 = -20000;
    uint16_t value3 = 65000;
    int32_t value4 = 100000;
    uint32_t value5 = 1000000000;
    int64_t value6 = -9999999;
    uint64_t value7 = 200000000000;
    JsonObject object1;
    object1["TEST1"] = value1;
    object1["TEST2"] = value2;
    object1["TEST3"] = value3;
    object1["TEST4"] = value4;
    JsonObject object2;
    object2["TEST5"] = value5;
    object2["TEST6"] = value6;
    object2["TEST7"] = value7;
    std::string strRet1 = R"({"TEST1":200,"TEST2":-20000,"TEST3":65000,"TEST4":100000})";
    std::string strRet2 = R"({"TEST5":1000000000,"TEST6":-9999999,"TEST7":200000000000})";
    EXPECT_EQ(strRet1, object1.Dump());
    EXPECT_EQ(strRet2, object2.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_002, testing::ext::TestSize.Level1)
{
    JsonObject object;
    const char* strValue1 = "value1";
    object["TEST1"] = strValue1;
    std::string strValue2 = "value2";
    object["TEST2"] = strValue2;
    std::string strRet = R"({"TEST1":"value1","TEST2":"value2"})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_003, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST"] = 15.3;
    std::string strRet = R"({"TEST":15.3})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_004, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = true;
    object["TEST2"] = false;
    std::string strRet = R"({"TEST1":true,"TEST2":false})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_005, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "value1";
    object["TEST2"] = 1000000000;
    object["TEST3"] = 5589.532;
    object["TEST4"] = true;
    std::string strRet = R"({"TEST1":"value1","TEST2":1000000000,"TEST3":5589.532,"TEST4":true})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_006, testing::ext::TestSize.Level1)
{
    JsonObject object1;
    object1["TEST1"] = "value1";
    JsonObject object2;
    object2["TEST2"] = "value2";
    object1.Insert("OBJ", object2);
    std::vector<std::string> checkJsons;
    checkJsons.push_back(R"({"TEST1":"value1","OBJ":{"TEST2":"value2"}})");
    checkJsons.push_back(R"({"OBJ":{"TEST2":"value2"},"TEST1":"value1"})");
    EXPECT_TRUE(CheckJsonString(checkJsons, object1.Dump()));
}

HWTEST_F(JsonObjectTest, SetValue_007, testing::ext::TestSize.Level1)
{
    JsonObject subObj;
    subObj["TEST"] = "test";
    JsonObject object(JsonCreateType::JSON_CREATE_TYPE_ARRAY);
    object.PushBack(int64_t(15));
    object.PushBack(int64_t(23));
    object.PushBack(17.1);
    object.PushBack("value");
    object.PushBack(subObj);
    std::string strRet = R"([15,23,17.1,"value",{"TEST":"test"}])";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_008, testing::ext::TestSize.Level1)
{
    std::vector<int32_t> verData = {45, 23, 68, 74, 56};
    JsonObject object;
    object["ARRAY"] = verData;
    std::string strRet = R"({"ARRAY":[45,23,68,74,56]})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_009, testing::ext::TestSize.Level1)
{
    TestJsonStru testObj;
    testObj.value1 = 44523;
    testObj.value2 = "testValue";
    testObj.value3 = true;
    JsonObject object;
    object["Object"] = testObj;
    std::string strRet = R"({"Object":{"value1":44523,"value2":"testValue","value3":true}})";
    EXPECT_EQ(strRet, object.Dump());

    JsonObject object1;
    object1 = testObj;
    strRet = R"({"value1":44523,"value2":"testValue","value3":true})";
    EXPECT_EQ(strRet, object1.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_010, testing::ext::TestSize.Level1)
{
    TestJsonStru testObj;
    testObj.value1 = 10;
    testObj.value2 = "val1";
    testObj.value3 = true;
    std::vector<TestJsonStru> verData;
    verData.push_back(testObj);
    testObj.value1 = 12;
    testObj.value2 = "val2";
    testObj.value3 = false;
    verData.push_back(testObj);

    JsonObject object;
    object["Obj"] = verData;
    std::string strRet = R"({"Obj":[{"value1":10,"value2":"val1","value3":true},)";
    strRet += R"({"value1":12,"value2":"val2","value3":false}]})";
    EXPECT_EQ(strRet, object.Dump());
}

HWTEST_F(JsonObjectTest, SetValue_011, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST"] = 25;
    std::vector<std::string> checkJsons;
    checkJsons.push_back("{\n\t\"TEST\":\t25\n}");
    checkJsons.push_back("{\n\t\"TEST\": 25\n}");
    EXPECT_TRUE(CheckJsonString(checkJsons, object.DumpFormated()));
}

HWTEST_F(JsonObjectTest, SetValue_012, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "value1";
    object["TEST2"] = 1000000000;
    object["TEST3"] = 5589.532;
    object["TEST4"] = true;
    std::string strRet = R"({"TEST1":"value1","TEST2":1000000000,"TEST3":5589.532,"TEST4":true})";
    EXPECT_EQ(strRet, object.Dump());

    JsonObject object1;
    object1.Duplicate(object);
    EXPECT_EQ(strRet, object1.Dump());
}

HWTEST_F(JsonObjectTest, DumpIgnoreError_01, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "special_characters!@#.123";
    object["TEST2"] = 1;
    object["TEST3"] = 0.0;
    object["TEST4"] = true;
    std::string strRet = R"({"TEST1":"special_characters!@#.123","TEST2":1,"TEST3":0.0,"TEST4":true})";
    EXPECT_EQ(strRet, object.DumpIgnoreError());
}

HWTEST_F(JsonObjectTest, Dump_01, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "long_string";
    object["TEST2"] = 36854775807;
    object["TEST3"] = -314.37;
    object["TEST4"] = false;
    std::string strRet =
        "{\n\t\"TEST1\": \"long_string\",\n\t\"TEST2\": 36854775807,\n\t\"TEST3\": -314.37,\n\t\"TEST4\": false\n}";
    EXPECT_EQ(strRet, object.Dump(true, true));
}

HWTEST_F(JsonObjectTest, Dump_02, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "null_string";
    object["TEST2"] = 922337203;
    object["TEST3"] = 5.358;
    object["TEST4"] = false;
    std::string strRet =
        "{\n\t\"TEST1\": \"null_string\",\n\t\"TEST2\": 922337203,\n\t\"TEST3\": 5.358,\n\t\"TEST4\": false\n}";
    EXPECT_EQ(strRet, object.Dump(true, false));
}

HWTEST_F(JsonObjectTest, Dump_03, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "escaped_string";
    object["TEST2"] = -42;
    object["TEST3"] = 0.0012345;
    object["TEST4"] = true;
    std::string strRet = R"({"TEST1":"escaped_string","TEST2":-42,"TEST3":0.0012345,"TEST4":true})";
    EXPECT_EQ(strRet, object.Dump(false, true));
}

HWTEST_F(JsonObjectTest, Dump_04, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "test_value";
    object["TEST2"] = 0;
    object["TEST3"] = -0.0001;
    object["TEST4"] = false;
    std::string strRet = R"({"TEST1":"test_value","TEST2":0,"TEST3":-0.0001,"TEST4":false})";
    EXPECT_EQ(strRet, object.Dump(false, false));
}

HWTEST_F(JsonObjectTest, Parse_001, testing::ext::TestSize.Level1)
{
    std::string strJson = R"(
    {
        "TEST1":"value1",
        "TEST2":15,
        "TEST3":true,
        "TEST4":0.03
    }
    )";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        std::vector<JsonItemObject> verItems = object.Items();
        EXPECT_EQ(verItems.size(), 4);
        EXPECT_TRUE(object.Contains("TEST1"));
        EXPECT_TRUE(object.Contains("TEST2"));
        EXPECT_TRUE(object.Contains("TEST3"));
        EXPECT_TRUE(object.Contains("TEST4"));
        EXPECT_FALSE(object.Contains("ABC"));
        EXPECT_TRUE(object.IsObject());

        EXPECT_FALSE(object["TEST1"].IsNumber());
        EXPECT_FALSE(object["TEST1"].IsNumberInteger());
        EXPECT_TRUE(object["TEST1"].IsString());
        EXPECT_FALSE(object["TEST1"].IsObject());
        EXPECT_FALSE(object["TEST1"].IsArray());
        EXPECT_FALSE(object["TEST1"].IsBoolean());

        EXPECT_TRUE(object["TEST2"].IsNumber());
        EXPECT_TRUE(object["TEST2"].IsNumberInteger());
        EXPECT_FALSE(object["TEST2"].IsString());
        EXPECT_FALSE(object["TEST2"].IsObject());
        EXPECT_FALSE(object["TEST2"].IsArray());
        EXPECT_FALSE(object["TEST2"].IsBoolean());

        EXPECT_FALSE(object["TEST3"].IsNumber());
        EXPECT_FALSE(object["TEST3"].IsNumberInteger());
        EXPECT_FALSE(object["TEST3"].IsString());
        EXPECT_FALSE(object["TEST3"].IsObject());
        EXPECT_FALSE(object["TEST3"].IsArray());
        EXPECT_TRUE(object["TEST3"].IsBoolean());

        EXPECT_TRUE(object["TEST4"].IsNumber());
        EXPECT_FALSE(object["TEST4"].IsNumberInteger());
        EXPECT_FALSE(object["TEST4"].IsString());
        EXPECT_FALSE(object["TEST4"].IsObject());
        EXPECT_FALSE(object["TEST4"].IsArray());
        EXPECT_FALSE(object["TEST4"].IsBoolean());
    }
}

HWTEST_F(JsonObjectTest, Parse_002, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"TEST1":"value1","TEST2":15,"TEST3":true, "TEST4":0.03)";
    JsonObject object(strJson);
    EXPECT_TRUE(object.IsDiscarded());
}

HWTEST_F(JsonObjectTest, Parse_003, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"TEST1":15.0,"TEST2":15.01})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_TRUE(object["TEST1"].IsNumber());
        EXPECT_TRUE(object["TEST2"].IsNumber());
        EXPECT_FALSE(object["TEST1"].IsNumberInteger());
        EXPECT_FALSE(object["TEST2"].IsNumberInteger());
    }
}

HWTEST_F(JsonObjectTest, Parse_004, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"TEST1":15.0,"TEST2":15.01})";
    JsonObject object;
    EXPECT_TRUE(object.Parse(strJson));

    std::string strJson1 = "";
    JsonObject object1;
    EXPECT_FALSE(object1.Parse(strJson1));
}

HWTEST_F(JsonObjectTest, Get_001, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"TEST1":"value1","TEST2":15,"TEST3":true, "TEST4":0.03})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_EQ(object["TEST1"].Get<std::string>(), "value1");
        EXPECT_EQ(object["TEST2"].Get<int32_t>(), 15);
        EXPECT_EQ(object["TEST3"].Get<bool>(), true);
        EXPECT_EQ(object["TEST4"].Get<double>(), 0.03);

        EXPECT_EQ(object["TEST1"].Get<bool>(), false);
        EXPECT_EQ(object["TEST1"].Get<int32_t>(), 0);
        EXPECT_EQ(object["TEST2"].Get<std::string>(), "");
    }
}

HWTEST_F(JsonObjectTest, Get_002, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"TEST1":"value1","TEST2":15,"TEST3":true, "TEST4":0.03})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        std::string strValue;
        object["TEST1"].GetTo(strValue);
        EXPECT_EQ(strValue, "value1");

        int32_t value1 = 0;
        object["TEST2"].GetTo(value1);
        EXPECT_EQ(value1, 15);
        uint32_t value2 = 0;
        object["TEST2"].GetTo(value2);
        EXPECT_EQ(value2, 15);
        int64_t value3 = 0;
        object["TEST2"].GetTo(value3);
        EXPECT_EQ(value3, 15);

        bool value4 = false;
        object["TEST3"].GetTo(value4);
        EXPECT_EQ(value4, true);

        double value5 = 0.0;
        object["TEST4"].GetTo(value5);
        EXPECT_EQ(value5, 0.03);
    }
}

HWTEST_F(JsonObjectTest, Get_003, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"value1":124,"value2":"MyTest","value3":true})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        TestJsonStru testData = object.Get<TestJsonStru>();
        EXPECT_EQ(testData.value1, 124);
        EXPECT_EQ(testData.value2, "MyTest");
        EXPECT_EQ(testData.value3, true);
    }
}

HWTEST_F(JsonObjectTest, Get_004, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Object":{"value1":124,"value2":"MyTest","value3":true}})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        TestJsonStru testData = object["Object"].Get<TestJsonStru>();
        EXPECT_EQ(testData.value1, 124);
        EXPECT_EQ(testData.value2, "MyTest");
        EXPECT_EQ(testData.value3, true);
    }
}

HWTEST_F(JsonObjectTest, Get_005, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Object":{"value1":124,"value2":"MyTest","value3":true}})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        JsonItemObject item = object["Object"];
        EXPECT_EQ(item.Key(), "Object");
    }
}

HWTEST_F(JsonObjectTest, Get_006, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Object":{"value1":124,"value2":"MyTest","value3":true}})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        JsonItemObject item = object.At("Object");
        EXPECT_EQ(item.Key(), "Object");
    }
}

HWTEST_F(JsonObjectTest, Get_007, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"ARRAY":[13, 24, 36, 48]})";
    std::vector<int32_t> verValues = {13, 24, 36, 48};
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_TRUE(object["ARRAY"].IsArray());
        std::vector<JsonItemObject> verItems = object["ARRAY"].Items();
        EXPECT_EQ(verItems.size(), verValues.size());
        for (size_t i = 0; i < verValues.size() && i < verItems.size(); ++i) {
            EXPECT_EQ(verValues[i], verItems[i].Get<int32_t>());
        }
    }
}

HWTEST_F(JsonObjectTest, Get_008, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"ARRAY":[13, 24, 36, 48]})";
    std::vector<int32_t> verValues = {13, 24, 36, 48};
    JsonObject object;
    EXPECT_TRUE(object.Parse(strJson));
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_TRUE(object["ARRAY"].IsArray());
        std::vector<int32_t> verVal;
        object["ARRAY"].Get(verVal);
        EXPECT_EQ(verVal.size(), verValues.size());
        for (size_t i = 0; i < verValues.size() && i < verVal.size(); ++i) {
            EXPECT_EQ(verValues[i], verVal[i]);
        }
    }
}

HWTEST_F(JsonObjectTest, Get_009, testing::ext::TestSize.Level1)
{
    std::string strJson = R"(
    {
        "ARRAY":[
            {"value1": 124, "value2": "MyTest", "value3": true},
            {"value1": 230, "value2": "test1", "value3": false},
            {"value1": 430, "value2": "test2", "value3": true}
        ]
    }
    )";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_TRUE(object["ARRAY"].IsArray());
        std::vector<TestJsonStru> verVal;
        object["ARRAY"].Get(verVal);
        EXPECT_EQ(verVal.size(), 3);
        EXPECT_EQ(verVal[0].value1, 124);
        EXPECT_EQ(verVal[0].value2, "MyTest");
        EXPECT_EQ(verVal[0].value3, true);
        EXPECT_EQ(verVal[1].value1, 230);
        EXPECT_EQ(verVal[1].value2, "test1");
        EXPECT_EQ(verVal[1].value3, false);
        EXPECT_EQ(verVal[2].value1, 430);
        EXPECT_EQ(verVal[2].value2, "test2");
        EXPECT_EQ(verVal[2].value3, true);
    }
}

HWTEST_F(JsonObjectTest, Get_010, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Object":{"value1":124,"value2":"MyTest","value3":true}})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        JsonItemObject item = object.At("Object");
        item.Erase("value2");
        std::string newStrJson = R"({"Object":{"value1":124,"value3":true}})";
        EXPECT_EQ(object.Dump(), newStrJson);
    }
}

HWTEST_F(JsonObjectTest, Get_011, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"value1":124,"value2":"MyTest","value3":true})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        object.Erase("value2");
        JsonObject subJsonObj;
        subJsonObj["TEST"] = 13.5;
        object.Insert("SUB", subJsonObj);
        std::vector<std::string> checkJsons;
        checkJsons.push_back(R"({"value1":124,"value3":true,"SUB":{"TEST":13.5}})");
        checkJsons.push_back(R"({"SUB":{"TEST":13.5},"value1":124,"value3":true})");
        EXPECT_TRUE(CheckJsonString(checkJsons, object.Dump()));
    }
}

HWTEST_F(JsonObjectTest, Get_012, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"value1":124,"value2":"MyTest","value3":true})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        JsonObject subJsonObj;
        subJsonObj["TEST"] = 13.5;
        object.Insert("value2", subJsonObj);
        std::string newStrJson = R"({"value1":124,"value3":true,"value2":{"TEST":13.5}})";
        std::vector<std::string> checkJsons;
        checkJsons.push_back(R"({"value1":124,"value3":true,"value2":{"TEST":13.5}})");
        checkJsons.push_back(R"({"value1":124,"value2":{"TEST":13.5},"value3":true})");
        EXPECT_TRUE(CheckJsonString(checkJsons, object.Dump()));
    }
}

HWTEST_F(JsonObjectTest, Get_013, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"ARRAY":[13,24,36,48]})";
    JsonObject object;
    EXPECT_TRUE(object.Parse(strJson));
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_EQ(object.Dump(), strJson);
    }
}

HWTEST_F(JsonObjectTest, Get_014, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"value1":124,"value2":"MyTest","value3":true})";
    JsonObject object;
    EXPECT_TRUE(object.Parse(strJson));
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_EQ(object.Dump(), strJson);
    }
}

HWTEST_F(JsonObjectTest, Get_015, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Object":{"value1":124,"value2":"MyTest","value3":true}})";
    JsonObject object;
    EXPECT_TRUE(object.Parse(strJson));
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_EQ(object.Dump(), strJson);
    }
}

HWTEST_F(JsonObjectTest, Get_016, testing::ext::TestSize.Level1)
{
    JsonObject object;
    object["TEST1"] = "value1";
    object["TEST2"] = 1000000000;
    object["TEST3"] = 5589.532;
    object["TEST4"] = true;
    std::string strJson = object.Dump();
    JsonObject object1(strJson);
    bool ret = object1.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        EXPECT_EQ(object1["TEST1"].Get<std::string>(), "value1");
        EXPECT_EQ(object1["TEST2"].Get<int64_t>(), 1000000000);
        EXPECT_EQ(object1["TEST3"].Get<double>(), 5589.532);
        EXPECT_EQ(object1["TEST4"].Get<bool>(), true);
    }
}

HWTEST_F(JsonObjectTest, Get_017, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"VALUE1":319691941099823986, "VALUE2":-319691941099823986})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        int64_t value1 = object["VALUE1"].Get<int64_t>();
        int64_t value2 = object["VALUE2"].Get<int64_t>();
        int64_t checkValue1 = 319691941099823986;
        int64_t checkValue2 = -319691941099823986;
        EXPECT_EQ(value1, checkValue1);
        EXPECT_EQ(value2, checkValue2);
    }
}

HWTEST_F(JsonObjectTest, Get_018, testing::ext::TestSize.Level1)
{
    std::string strJson = R"({"Values":[319691941099823986, -319691941099823986, 419691941099823986]})";
    JsonObject object(strJson);
    bool ret = object.IsDiscarded();
    EXPECT_FALSE(ret);
    if (!ret) {
        std::vector<int64_t> verValues;
        object["Values"].Get(verValues);
        std::vector<int64_t> verCheckValues = {319691941099823986, -319691941099823986, 419691941099823986};
        EXPECT_EQ(verValues, verCheckValues);
        for (size_t i = 0; i < verValues.size() && i < verCheckValues.size(); ++i) {
            EXPECT_EQ(verValues[i], verCheckValues[i]);
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS