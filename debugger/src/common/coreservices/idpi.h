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

#ifndef __DEBUGGER_COMMON_CORESERVICES_IDPI_H__
#define __DEBUGGER_COMMON_CORESERVICES_IDPI_H__

#include <iface.h>

namespace debugger {

static const char *IFACE_DPI = "IDpi";

class IDpi : public IFace {
 public:
    IDpi() : IFace(IFACE_DPI) {}

    virtual void axi4_write(uint64_t addr, uint64_t data) = 0;
    virtual void axi4_read(uint64_t addr, uint64_t *data) = 0;
    virtual bool is_irq() = 0;
    virtual int get_irq() = 0;
};

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_CORESERVICES_IDPI_H__
