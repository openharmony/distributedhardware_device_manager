#include "dm_dialog_manager_mock.h"

namespace OHOS {
namespace DistributedHardware {

void DmDialogManager::ShowConfirmDialog(const std::string param)
{
    DmDialogManagerMock::dmDialogManagerMock->ShowConfirmDialog(param);
}
}
}