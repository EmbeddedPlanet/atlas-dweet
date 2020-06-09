/*
 * Copyright (c) 2018, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "TELIT_ME310_CellularContext.h"
#include "TELIT_ME310_CellularStack.h"
#include "CellularLog.h"

#include "Semaphore.h"

using namespace mbed_cellular_util;

namespace mbed {

TELIT_ME310_CellularContext::TELIT_ME310_CellularContext(ATHandler &at, CellularDevice *device, const char *apn, bool cp_req, bool nonip_req) :
    AT_CellularContext(at, device, apn, cp_req, nonip_req)
{
}

TELIT_ME310_CellularContext::~TELIT_ME310_CellularContext()
{
}

#if !NSAPI_PPP_AVAILABLE
NetworkStack *TELIT_ME310_CellularContext::get_stack()
{
    if (_pdp_type == NON_IP_PDP_TYPE || (_nonip_req && _pdp_type != DEFAULT_PDP_TYPE)) {
        tr_error("Requesting stack for NON-IP context! Should request control plane netif: get_cp_netif()");
        return NULL;
    }

    if (!_stack) {
        _stack = new TELIT_ME310_CellularStack(_at, _cid, (nsapi_ip_stack_t)_pdp_type);
    }

    return _stack;
}
#endif // #if !NSAPI_PPP_AVAILABLE

bool TELIT_ME310_CellularContext::get_context()
{
    _at.cmd_start_stop("+CGDCONT", "?");
    _at.resp_start("+CGDCONT:");
    _cid = -1;
    int cid_max = 0; // needed when creating new context
    char apn[MAX_ACCESSPOINT_NAME_LENGTH];
    int apn_len = 0;

    while (_at.info_resp()) {
        int cid = _at.read_int();
        if (cid > cid_max) {
            cid_max = cid;
        }
        char pdp_type_from_context[10];
        int pdp_type_len = _at.read_string(pdp_type_from_context, sizeof(pdp_type_from_context));
        if (pdp_type_len > 0) {
            apn_len = _at.read_string(apn, sizeof(apn));
            if (apn_len >= 0) {
                if (_apn && apn_len > 0 && (strcmp(apn, _apn) != 0)) {
                    continue;
                }

                // APN matched -> Check PDP type
                pdp_type_t pdp_type = string_to_pdp_type(pdp_type_from_context);

                // Accept exact matching PDP context type or dual PDP context for modems that support both IPv4 and IPv6 stacks
                if (get_property(pdp_type_t_to_cellular_property(pdp_type)) ||
                        ((pdp_type == IPV4V6_PDP_TYPE && (get_property(PROPERTY_IPV4_PDP_TYPE) &&
                                                          get_property(PROPERTY_IPV6_PDP_TYPE))) && !_nonip_req)) {
                    _pdp_type = pdp_type;
                    set_new_context(cid);
                }
            }
        }
    }

    _at.resp_stop();
    if (_cid == -1) { // no suitable context was found so create a new one
        if (!set_new_context(1)) {
            return false;
        }
    }

    // save the apn
    if (apn_len > 0 && !_apn) {
        memcpy(_found_apn, apn, apn_len + 1);
    }

    tr_info("Found PDP context %d", _cid);
    return true;
}

} /* namespace mbed */
