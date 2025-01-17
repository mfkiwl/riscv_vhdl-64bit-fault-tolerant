/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <api_core.h>
#include "cpu_stub_fpga.h"

namespace debugger {

CpuStubRiscVFpga::CpuStubRiscVFpga(const char *name) :
    IService(name) {
    registerAttribute("CmdExecutor", &cmdexec_);
    registerAttribute("DmiBAR", &dmibar_);
    registerAttribute("Tap", &tap_);
}

void CpuStubRiscVFpga::postinitService() {
    icmdexec_ = static_cast<ICmdExecutor *>(
       RISCV_get_service_iface(cmdexec_.to_string(), IFACE_CMD_EXECUTOR));
    if (!icmdexec_) {
        RISCV_error("ICmdExecutor interface '%s' not found", 
                    cmdexec_.to_string());
        return;
    }

    itap_ = static_cast<ITap *>(
       RISCV_get_service_iface(tap_.to_string(), IFACE_TAP));
    if (!itap_) {
        RISCV_error("ITap interface '%s' not found", tap_.to_string());
        return;
    }

    pcmd_br_ = new CmdBrRiscv(dmibar_.to_uint64(), itap_);
    icmdexec_->registerCommand(static_cast<ICommand *>(pcmd_br_));
}

void CpuStubRiscVFpga::predeleteService() {
    icmdexec_->unregisterCommand(static_cast<ICommand *>(pcmd_br_));
    delete pcmd_br_;
}

}  // namespace debugger

