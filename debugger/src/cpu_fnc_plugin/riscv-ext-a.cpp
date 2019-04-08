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
 *
 * @brief      RISC-V extension-A (Atomic Instructions).
 */

#include "api_core.h"
#include "riscv-isa.h"
#include "cpu_riscv_func.h"

namespace debugger {

void CpuRiver_Functional::addIsaExtensionA() {
    // TODO
    /*
    addInstr("AMOADD_W",           "00000????????????010?????0101111", NULL, out);
    addInstr("AMOXOR_W",           "00100????????????010?????0101111", NULL, out);
    addInstr("AMOOR_W",            "01000????????????010?????0101111", NULL, out);
    addInstr("AMOAND_W",           "01100????????????010?????0101111", NULL, out);
    addInstr("AMOMIN_W",           "10000????????????010?????0101111", NULL, out);
    addInstr("AMOMAX_W",           "10100????????????010?????0101111", NULL, out);
    addInstr("AMOMINU_W",          "11000????????????010?????0101111", NULL, out);
    addInstr("AMOMAXU_W",          "11100????????????010?????0101111", NULL, out);
    addInstr("AMOSWAP_W",          "00001????????????010?????0101111", NULL, out);
    addInstr("LR_W",               "00010??00000?????010?????0101111", NULL, out);
    addInstr("SC_W",               "00011????????????010?????0101111", NULL, out);
    addInstr("AMOADD_D",           "00000????????????011?????0101111", NULL, out);
    addInstr("AMOXOR_D",           "00100????????????011?????0101111", NULL, out);
    addInstr("AMOOR_D",            "01000????????????011?????0101111", NULL, out);
    addInstr("AMOAND_D",           "01100????????????011?????0101111", NULL, out);
    addInstr("AMOMIN_D",           "10000????????????011?????0101111", NULL, out);
    addInstr("AMOMAX_D",           "10100????????????011?????0101111", NULL, out);
    addInstr("AMOMINU_D",          "11000????????????011?????0101111", NULL, out);
    addInstr("AMOMAXU_D",          "11100????????????011?????0101111", NULL, out);
    addInstr("AMOSWAP_D",          "00001????????????011?????0101111", NULL, out);
    addInstr("LR_D",               "00010??00000?????011?????0101111", NULL, out);
    addInstr("SC_D",               "00011????????????011?????0101111", NULL, out);
    */
    uint64_t isa = portCSR_.read(CSR_misa).val;
    portCSR_.write(CSR_misa, isa | (1LL << ('A' - 'A')));
}

}  // namespace debugger
