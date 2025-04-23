#ifndef DM_DIALOG_MANAGER_MOCK_H
#define DM_DIALOG_MANAGER_MOCK_H

#include "dm_dialog_manager.h"

namespace OHOS {
namespace DistributedHardware {
class DmDialogManagerMock {
public:
    MOCK_METHOD(void, ShowConfirmDialog, (const std::string));
    static inline std::shared_ptr<DmDialogManagerMock> dmDialogManagerMock = nullptr;
};
}
}
#endif