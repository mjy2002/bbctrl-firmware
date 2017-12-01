/******************************************************************************\

                This file is part of the Buildbotics firmware.

                  Copyright (c) 2015 - 2017 Buildbotics LLC
                  Copyright (c) 2013 - 2015 Alden S. Hart, Jr.
                  Copyright (c) 2013 - 2015 Robert Giseburt
                            All rights reserved.

     This file ("the software") is free software: you can redistribute it
     and/or modify it under the terms of the GNU General Public License,
      version 2 as published by the Free Software Foundation. You should
      have received a copy of the GNU General Public License, version 2
     along with the software. If not, see <http://www.gnu.org/licenses/>.

     The software is distributed in the hope that it will be useful, but
          WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
               Lesser General Public License for more details.

       You should have received a copy of the GNU Lesser General Public
                License along with the software.  If not, see
                       <http://www.gnu.org/licenses/>.

                For information regarding this software email:
                  "Joseph Coffland" <joseph@buildbotics.com>

\******************************************************************************/

#pragma once

#include "pgmspace.h"

#include <stdbool.h>


typedef enum {
  STATE_READY,
  STATE_ESTOPPED,
  STATE_RUNNING,
  STATE_STOPPING,
  STATE_HOLDING,
} state_t;


typedef enum {
  CYCLE_MACHINING,
  CYCLE_HOMING,
  CYCLE_PROBING,
  CYCLE_JOGGING,
} cycle_t;


typedef enum {
  HOLD_REASON_USER_PAUSE,
  HOLD_REASON_PROGRAM_PAUSE,
  HOLD_REASON_PROGRAM_END,
  HOLD_REASON_PALLET_CHANGE,
  HOLD_REASON_TOOL_CHANGE,
} hold_reason_t;


PGM_P state_get_pgmstr(state_t state);
PGM_P state_get_cycle_pgmstr(cycle_t cycle);
PGM_P state_get_hold_reason_pgmstr(hold_reason_t reason);

state_t state_get();
cycle_t state_get_cycle();
void state_set_cycle(cycle_t cycle);
void state_set_hold_reason(hold_reason_t reason);

bool state_is_flushing();
bool state_is_resuming();
bool state_is_quiescent();
bool state_is_ready();
void state_pause_queue(bool x);

void state_optional_pause();
void state_holding();
void state_running();
void state_idle();
void state_estop();

void state_request_hold();
void state_request_start();
void state_request_flush();
void state_request_resume();
void state_request_optional_pause();
void state_request_step();

void state_callback();