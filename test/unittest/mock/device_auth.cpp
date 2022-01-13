#include "device_auth.h"
#include "dm_constants.h"

int32_t (*deleteGroup)(int64_t requestId, const char *appId, const char *disbandParams) 
{
    if (disbandParams == 0) {
        return DM_FAILED;
    }

    return DM_OK;
}

int32_t (*getRelatedGroups)(const char *appId, const char *peerDeviceId, char **returnGroupVec, uint32_t *groupNum) 
{
     return 0;
}


int32_t (*createGroup)(int64_t requestId, const char *appId, const char *createParams) 
{
    if (requestId == 0) {
        return DM_FAILED;
    }
    return 0;
}


int32_t (*getRelatedGroups)(const char *appId, const char *peerDeviceId, char **returnGroupVec, uint32_t *groupNum) 
{
    if (peerDeviceId == "123") {
        returnGroupVec = nullptr;
    }

    if (peerDeviceId == "12345") {
        groupNum = 0;
    }

    if (peerDeviceId == "34567") {
        groupNum = 1;
        returnGroupVec = "123";
    }
}