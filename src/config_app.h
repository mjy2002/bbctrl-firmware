/*
 * config_app.h - application-specific part of configuration sub-system
 * This file is part of the TinyG project
 *
 * Copyright (c) 2010 - 2014 Alden S. Hart, Jr.
 * Copyright (c) 2013 - 2014 Robert Giseburt
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CONFIG_APP_H_ONCE
#define CONFIG_APP_H_ONCE

enum nvType {                       // classification of commands
  NV_TYPE_0 = 0,
  NV_TYPE_CONFIG,                   // configuration commands
  NV_TYPE_GCODE,                    // gcode
  NV_TYPE_REPORT,                   // SR, QR and any other report
  NV_TYPE_MESSAGE,                  // nv object carries a message
  NV_TYPE_LINENUM                   // nv object carries a gcode line number
};


typedef struct cfgParameters {      // mostly communications variables
  uint16_t magic_start;             // magic number to test memory integrity

  // Communications settings
  uint8_t comm_mode;                // TG_TEXT_MODE or TG_JSON_MODE
  uint8_t enable_cr;                // enable CR in CRFL expansion on TX
  uint8_t enable_echo;              // enable text-mode echo
  uint8_t enable_flow_control;      // enable XON/XOFF or RTS/CTS flow control

  uint8_t baud_rate;                // see usart.h for BAUD values
  uint8_t baud_flag;                // should be in controller singleton

  // User defined data groups
  uint32_t user_data_a[4];
  uint32_t user_data_b[4];
  uint32_t user_data_c[4];
  uint32_t user_data_d[4];

  uint16_t magic_end;
} cfgParameters_t;
extern cfgParameters_t cfg;


stat_t set_baud_callback();


// Job config
void job_print_job(nvObj_t *nv);
stat_t job_get(nvObj_t *nv);
stat_t job_set(nvObj_t *nv);
uint8_t job_report_callback();


#ifdef __TEXT_MODE
void cfg_print_ec(nvObj_t *nv);
void cfg_print_ee(nvObj_t *nv);
void cfg_print_ex(nvObj_t *nv);
void cfg_print_baud(nvObj_t *nv);
void cfg_print_net(nvObj_t *nv);
void cfg_print_rx(nvObj_t *nv);
#else
#define cfg_print_ec tx_print_stub
#define cfg_print_ee tx_print_stub
#define cfg_print_ex tx_print_stub
#define cfg_print_baud tx_print_stub
#define cfg_print_net tx_print_stub
#define cfg_print_rx tx_print_stub
#endif // __TEXT_MODE

#endif // CONFIG_APP_H_ONCE