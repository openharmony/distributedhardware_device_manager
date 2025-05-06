#include "ohos.distributedDeviceManager.ani.hpp"
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result) {
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        return ANI_ERROR;
    }
    if (ANI_OK != ohos::distributedDeviceManager::ANIRegister(env)) {
        std::cerr << "Error from ohos::distributedDeviceManager::ANIRegister" << std::endl;
        return ANI_ERROR;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
