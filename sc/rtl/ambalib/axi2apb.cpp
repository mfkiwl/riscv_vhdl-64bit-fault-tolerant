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

#include "axi2apb.h"
#include "api_core.h"

namespace debugger {

axi2apb::axi2apb(sc_module_name name,
                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_mapinfo("i_mapinfo"),
    o_cfg("o_cfg"),
    i_xslvi("i_xslvi"),
    o_xslvo("o_xslvo"),
    i_apbmi("i_apbmi"),
    o_apbmo("o_apbmo") {

    async_reset_ = async_reset;
    axi0 = 0;

    axi0 = new axi_slv("axi0", async_reset, VENDOR_OPTIMITECH, OPTIMITECH_AXI2APB_BRIDGE);
    axi0->i_clk(i_clk);
    axi0->i_nrst(i_nrst);
    axi0->i_mapinfo(i_mapinfo);
    axi0->o_cfg(o_cfg);
    axi0->i_xslvi(i_xslvi);
    axi0->o_xslvo(o_xslvo);
    axi0->o_req_valid(w_req_valid);
    axi0->o_req_addr(wb_req_addr);
    axi0->o_req_write(w_req_write);
    axi0->o_req_wdata(wb_req_wdata);
    axi0->o_req_wstrb(wb_req_wstrb);
    axi0->o_req_last(w_req_last);
    axi0->i_req_ready(w_req_ready);
    axi0->i_resp_valid(r.pvalid);
    axi0->i_resp_rdata(r.prdata);
    axi0->i_resp_err(r.pslverr);



    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_mapinfo;
    sensitive << i_xslvi;
    sensitive << i_apbmi;
    sensitive << w_req_valid;
    sensitive << wb_req_addr;
    sensitive << w_req_write;
    sensitive << wb_req_wdata;
    sensitive << wb_req_wstrb;
    sensitive << w_req_last;
    sensitive << w_req_ready;
    sensitive << r.state;
    sensitive << r.pvalid;
    sensitive << r.paddr;
    sensitive << r.pwdata;
    sensitive << r.prdata;
    sensitive << r.pwrite;
    sensitive << r.pstrb;
    sensitive << r.pprot;
    sensitive << r.pselx;
    sensitive << r.penable;
    sensitive << r.pslverr;
    sensitive << r.xsize;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

axi2apb::~axi2apb() {
    if (axi0) {
        delete axi0;
    }
}

void axi2apb::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_mapinfo, i_mapinfo.name());
        sc_trace(o_vcd, o_cfg, o_cfg.name());
        sc_trace(o_vcd, i_xslvi, i_xslvi.name());
        sc_trace(o_vcd, o_xslvo, o_xslvo.name());
        sc_trace(o_vcd, i_apbmi, i_apbmi.name());
        sc_trace(o_vcd, o_apbmo, o_apbmo.name());
        sc_trace(o_vcd, r.state, pn + ".r_state");
        sc_trace(o_vcd, r.pvalid, pn + ".r_pvalid");
        sc_trace(o_vcd, r.paddr, pn + ".r_paddr");
        sc_trace(o_vcd, r.pwdata, pn + ".r_pwdata");
        sc_trace(o_vcd, r.prdata, pn + ".r_prdata");
        sc_trace(o_vcd, r.pwrite, pn + ".r_pwrite");
        sc_trace(o_vcd, r.pstrb, pn + ".r_pstrb");
        sc_trace(o_vcd, r.pprot, pn + ".r_pprot");
        sc_trace(o_vcd, r.pselx, pn + ".r_pselx");
        sc_trace(o_vcd, r.penable, pn + ".r_penable");
        sc_trace(o_vcd, r.pslverr, pn + ".r_pslverr");
        sc_trace(o_vcd, r.xsize, pn + ".r_xsize");
    }

    if (axi0) {
        axi0->generateVCD(i_vcd, o_vcd);
    }
}

void axi2apb::comb() {
    apb_in_type vapbmo;

    vapbmo = apb_in_none;

    v = r;

    w_req_ready = 0;
    v.pvalid = 0;

    switch (r.state.read()) {
    case State_Idle:
        w_req_ready = 1;
        v.pslverr = 0;
        v.penable = 0;
        v.pselx = 0;
        v.xsize = 0;
        if (w_req_valid.read() == 1) {
            v.pwrite = w_req_write;
            v.pselx = 1;
            v.paddr = (wb_req_addr.read()(31, 2) << 2);
            v.pprot = 0;
            v.pwdata = wb_req_wdata;
            v.pstrb = wb_req_wstrb;
            v.state = State_setup;
            v.xsize = wb_req_wstrb.read().and_reduce();
            if (w_req_last.read() == 0) {
                v.state = State_out;                        // Burst is not supported
                v.pselx = 0;
                v.pslverr = 1;
                v.prdata = ~0ull;
            }
        }
        break;
    case State_setup:
        v.penable = 1;
        v.state = State_access;
        break;
    case State_access:
        v.pslverr = i_apbmi.read().pslverr;
        if (i_apbmi.read().pready == 1) {
            v.penable = 0;
            v.pselx = 0;
            v.pwrite = 0;
            if (r.paddr.read()[2] == 0) {
                v.prdata = (r.prdata.read()(63, 32), i_apbmi.read().prdata);
            } else {
                v.prdata = (i_apbmi.read().prdata, r.prdata.read()(31, 0));
            }
            if (r.xsize.read().or_reduce() == 1) {
                v.xsize = (r.xsize.read() - 1);
                v.paddr = (r.paddr.read() + 4);
                v.state = State_setup;
            } else {
                v.pvalid = 1;
                v.state = State_out;
            }
        }
        break;
    case State_out:
        v.pvalid = 0;
        v.pslverr = 0;
        v.state = State_Idle;
        break;
    default:
        break;
    }

    vapbmo.paddr = r.paddr;
    vapbmo.pwrite = r.pwrite;
    if (r.paddr.read()[2] == 0) {
        vapbmo.pwdata = r.pwdata.read()(31, 0);
        vapbmo.pstrb = r.pstrb.read()(3, 0);
    } else {
        vapbmo.pwdata = r.pwdata.read()(63, 32);
        vapbmo.pstrb = r.pstrb.read()(7, 4);
    }
    vapbmo.pselx = r.pselx;
    vapbmo.penable = r.penable;
    vapbmo.pprot = r.pprot;

    if (!async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(v);
    }

    o_apbmo = vapbmo;
}

void axi2apb::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        axi2apb_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

