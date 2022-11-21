// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "types_amba.h"

namespace debugger {

SC_MODULE(axi_slv) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<axi4_slave_in_type> i_xslvi;                      // AXI Slave input interface
    sc_out<axi4_slave_out_type> o_xslvo;                    // AXI Slave output interface
    sc_out<bool> o_req_valid;
    sc_out<sc_uint<CFG_SYSBUS_ADDR_BITS>> o_req_addr;
    sc_out<bool> o_req_write;
    sc_out<sc_uint<CFG_SYSBUS_DATA_BITS>> o_req_wdata;
    sc_out<bool> o_req_burst;
    sc_out<bool> o_req_last;
    sc_in<bool> i_resp_valid;
    sc_in<sc_uint<CFG_SYSBUS_DATA_BITS>> i_resp_rdata;
    sc_in<bool> i_resp_err;

    void comb();
    void registers();

    SC_HAS_PROCESS(axi_slv);

    axi_slv(sc_module_name name,
            bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t State_Idle = 0;
    static const uint8_t State_w = 1;
    static const uint8_t State_burst_w = 2;
    static const uint8_t State_last_w = 3;
    static const uint8_t State_burst_r = 4;
    static const uint8_t State_last_r = 5;
    static const uint8_t State_b = 6;

    struct axi_slv_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> req_valid;
        sc_signal<sc_uint<CFG_SYSBUS_ADDR_BITS>> req_addr;
        sc_signal<bool> req_write;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> req_wdata;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BYTES>> req_wstrb;
        sc_signal<sc_uint<8>> req_xsize;
        sc_signal<sc_uint<8>> req_len;
        sc_signal<sc_uint<CFG_SYSBUS_USER_BITS>> req_user;
        sc_signal<bool> req_burst;
        sc_signal<bool> req_last;
        sc_signal<bool> resp_valid;
        sc_signal<sc_uint<CFG_SYSBUS_DATA_BITS>> resp_rdata;
        sc_signal<bool> resp_err;
    } v, r;

    void axi_slv_r_reset(axi_slv_registers &iv) {
        iv.state = State_Idle;
        iv.req_valid = 0;
        iv.req_addr = 0ull;
        iv.req_write = 0;
        iv.req_wdata = 0ull;
        iv.req_wstrb = 0;
        iv.req_xsize = 0;
        iv.req_len = 0;
        iv.req_user = 0;
        iv.req_burst = 0;
        iv.req_last = 0;
        iv.resp_valid = 0;
        iv.resp_rdata = 0ull;
        iv.resp_err = 0;
    }

};

}  // namespace debugger
