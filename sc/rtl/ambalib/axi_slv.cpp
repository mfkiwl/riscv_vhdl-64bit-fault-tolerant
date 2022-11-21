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

#include "axi_slv.h"
#include "api_core.h"

namespace debugger {

axi_slv::axi_slv(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    o_req_valid("o_req_valid"),
    o_req_addr("o_req_addr"),
    o_req_write("o_req_write"),
    o_req_wdata("o_req_wdata"),
    o_req_burst("o_req_burst"),
    o_req_last("o_req_last"),
    i_resp_valid("i_resp_valid"),
    i_resp_rdata("i_resp_rdata"),
    i_resp_err("i_resp_err") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_xslvi;
    sensitive << i_resp_valid;
    sensitive << i_resp_rdata;
    sensitive << i_resp_err;
    sensitive << r.state;
    sensitive << r.req_valid;
    sensitive << r.req_addr;
    sensitive << r.req_write;
    sensitive << r.req_wdata;
    sensitive << r.req_wstrb;
    sensitive << r.req_xsize;
    sensitive << r.req_len;
    sensitive << r.req_user;
    sensitive << r.req_burst;
    sensitive << r.req_last;
    sensitive << r.resp_valid;
    sensitive << r.resp_rdata;
    sensitive << r.resp_err;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void axi_slv::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, o_req_valid, o_req_valid.name());
        sc_trace(o_vcd, o_req_addr, o_req_addr.name());
        sc_trace(o_vcd, o_req_write, o_req_write.name());
        sc_trace(o_vcd, o_req_wdata, o_req_wdata.name());
        sc_trace(o_vcd, o_req_burst, o_req_burst.name());
        sc_trace(o_vcd, o_req_last, o_req_last.name());
        sc_trace(o_vcd, i_resp_valid, i_resp_valid.name());
        sc_trace(o_vcd, i_resp_rdata, i_resp_rdata.name());
        sc_trace(o_vcd, i_resp_err, i_resp_err.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.req_valid, pn + ".r_req_valid");
        sc_trace(o_vcd, r.req_addr, pn + ".r_req_addr");
        sc_trace(o_vcd, r.req_write, pn + ".r_req_write");
        sc_trace(o_vcd, r.req_wdata, pn + ".r_req_wdata");
        sc_trace(o_vcd, r.req_wstrb, pn + ".r_req_wstrb");
        sc_trace(o_vcd, r.req_xsize, pn + ".r_req_xsize");
        sc_trace(o_vcd, r.req_len, pn + ".r_req_len");
        sc_trace(o_vcd, r.req_user, pn + ".r_req_user");
        sc_trace(o_vcd, r.req_burst, pn + ".r_req_burst");
        sc_trace(o_vcd, r.req_last, pn + ".r_req_last");
        sc_trace(o_vcd, r.resp_valid, pn + ".r_resp_valid");
        sc_trace(o_vcd, r.resp_rdata, pn + ".r_resp_rdata");
        sc_trace(o_vcd, r.resp_err, pn + ".r_resp_err");
    }

}

void axi_slv::comb() {
    sc_uint<12> vb_req_addr_next;
    sc_uint<32> vb_rdata;
    apb_out_type vapbo;

    vb_req_addr_next = 0;
    vb_rdata = 0;
    vapbo = apb_out_none;

    v = r;

    v.req_valid = 0;
    v.req_last = 0;
    vb_req_addr_next = (r.req_addr.read()(11, 0) + r.req_xsize.read());

    switch (r.state.read()) {
    case State_Idle:
        v.req_burst = 0;
        v.resp_valid = 0;
        v.resp_err = 0;
        if (i_xslvi.read().aw_valid == 1) {
            v.req_write = 1;
            v.req_addr = i_xslvi.read().aw_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.read().aw_bits.size);
            v.req_len = i_xslvi.read().aw_bits.len;
            v.req_user = i_xslvi.read().aw_user;
            v.req_wdata = i_xslvi.read().w_data;            // AXI Lite compatible
            v.req_wstrb = i_xslvi.read().w_strb;
            if (i_xslvi.read().w_valid == 1) {
                // AXI Lite does not support burst transaction
                v.state = State_last_w;
                v.req_valid = 1;
                v.req_last = 1;
            } else {
                v.state = State_w;
            }
        } else if (i_xslvi.read().ar_valid == 1) {
            v.req_valid = 1;
            v.req_write = 0;
            v.req_addr = i_xslvi.read().ar_bits.addr;
            v.req_xsize = XSizeToBytes(i_xslvi.read().ar_bits.size);
            v.req_len = i_xslvi.read().ar_bits.len;
            v.req_user = i_xslvi.read().ar_user;
            if (i_xslvi.read().ar_bits.len.or_reduce() == 1) {
                v.state = State_burst_r;
                v.req_burst = 1;
            } else {
                v.state = State_last_r;
                v.req_last = 1;
            }
        }
        break;
    case State_w:
        v.req_wdata = i_xslvi.read().w_data;
        v.req_wstrb = i_xslvi.read().w_strb;
        if (i_xslvi.read().w_valid == 1) {
            v.req_valid = 1;
            if (r.req_len.read().or_reduce() == 1) {
                v.state = State_burst_w;
                v.req_burst = 1;
            } else {
                v.state = State_last_w;
                v.req_last = 1;
            }
        }
        break;
    case State_burst_w:
        if (i_xslvi.read().w_valid == 1) {
            v.req_valid = 1;
            v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_req_addr_next);
            v.req_wdata = i_xslvi.read().w_data;
            v.req_wstrb = i_xslvi.read().w_strb;
            if (r.req_len.read().or_reduce() == 1) {
                v.req_len = (r.req_len.read() - 1);
            }
            if (r.req_len.read() == 1) {
                v.state = State_last_w;
                v.req_last = 1;
            }
        }
        break;
    case State_last_w:
        v.resp_valid = i_resp_valid;
        if (i_resp_valid.read() == 1) {
            v.resp_err = i_resp_err;
            v.state = State_b;
        }
        break;
    case State_b:
        if (i_xslvi.read().b_ready == 1) {
            v.state = State_Idle;
        }
        break;
    case State_burst_r:
        v.req_valid = 1;
        if (i_xslvi.read().r_ready == 1) {
            v.req_addr = (r.req_addr.read()((CFG_SYSBUS_ADDR_BITS - 1), 12), vb_req_addr_next);
            if (r.req_len.read().or_reduce() == 1) {
                v.req_len = (r.req_len.read() - 1);
            }
            if (r.req_len.read() == 1) {
                v.state = State_last_w;
                v.req_last = 1;
            }
        }
        if (i_resp_valid.read() == 1) {
            v.resp_rdata = i_resp_rdata;
            v.resp_err = i_resp_err;
            v.state = State_b;
        }
        break;
    case State_last_r:
        if (i_xslvi.read().r_ready == 1) {
            v.state = State_Idle;
            v.resp_valid = 0;
        }
        break;
    default:
        break;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        axi_slv_r_reset(v);
    }

    o_req_valid = r.req_valid;
    o_req_addr = r.req_addr;
    o_req_write = r.req_write;
    o_req_wdata = r.req_wdata;

    vapbo.pready = r.resp_valid;
    vapbo.prdata = r.resp_rdata;
    vapbo.pslverr = r.resp_err;
}

void axi_slv::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axi_slv_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger
