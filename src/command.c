/******************************************************************************\

                This file is part of the Buildbotics firmware.

                  Copyright (c) 2015 - 2016 Buildbotics LLC
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

#include "command.h"

#include "gcode_parser.h"
#include "usart.h"
#include "hardware.h"
#include "report.h"
#include "vars.h"
#include "estop.h"
#include "plan/jog.h"
#include "plan/calibrate.h"
#include "plan/buffer.h"
#include "config.h"

#include <avr/pgmspace.h>
#include <avr/wdt.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>


// Command forward declarations
// (Don't be afraid, X-Macros rock!)
#define CMD(NAME, ...)                          \
  uint8_t command_##NAME(int, char *[]);

#include "command.def"
#undef CMD

// Command names & help
#define CMD(NAME, MINARGS, MAXARGS, HELP)      \
  static const char pstr_##NAME[] PROGMEM = #NAME;   \
  static const char NAME##_help[] PROGMEM = HELP;

#include "command.def"
#undef CMD

// Command table
#define CMD(NAME, MINARGS, MAXARGS, HELP)                       \
  {pstr_##NAME, command_##NAME, MINARGS, MAXARGS, NAME##_help},

static const command_t commands[] PROGMEM = {
#include "command.def"
#undef CMD
  {}, // Sentinel
};


static char *_cmd = 0;


int command_find(const char *match) {
  for (int i = 0; ; i++) {
    const char *name = pgm_read_word(&commands[i].name);
    if (!name) break;

    if (strcmp_P(match, name) == 0) return i;
  }

  return -1;
}


int command_exec(int argc, char *argv[]) {
  putchar('\n');

  int i = command_find(argv[0]);
  if (i != -1) {
    uint8_t minArgs = pgm_read_byte(&commands[i].minArgs);
    uint8_t maxArgs = pgm_read_byte(&commands[i].maxArgs);

    if (argc <= minArgs) return STAT_TOO_FEW_ARGUMENTS;
    else if (maxArgs < argc - 1) return STAT_TOO_MANY_ARGUMENTS;
    else {
      command_cb_t cb = pgm_read_word(&commands[i].cb);
      return cb(argc, argv);
    }

  } else if (argc != 1)
    return STAT_INVALID_OR_MALFORMED_COMMAND;

  // Get or set variable
  char *value = strchr(argv[0], '=');
  if (value) {
    *value++ = 0;
    if (vars_set(argv[0], value)) return STAT_OK;

  } else if (vars_print(argv[0])) {
    putchar('\n');
    return STAT_OK;
  }

  return STAT_UNRECOGNIZED_NAME;
}


int command_parser(char *cmd) {
  // Parse line
  char *p = cmd + 1; // Skip `$`
  int argc = 0;
  char *argv[MAX_ARGS] = {};

  while (argc < MAX_ARGS && *p) {
    // Skip space
    while (*p && isspace(*p)) *p++ = 0;

    // Start of token
    if (*p) argv[argc++] = p;

    // Find end
    while (*p && !isspace(*p)) p++;
  }

  // Exec command
  if (argc) return command_exec(argc, argv);

  return STAT_OK;
}


stat_t command_hi() {
  // Get next command
  if (!_cmd) {
    // Read input line or return if incomplete, usart_readline() is non-blocking
    _cmd = usart_readline();
    if (!_cmd) return STAT_OK;

    // Skip leading whitespace
    while (*_cmd && isspace(*_cmd)) _cmd++;
  }

  if (usart_tx_full()) return STAT_OK;

  stat_t status = STAT_OK;

  switch (*_cmd) {
  case 0: report_request_full(); break; // Full report
  case '{': status = vars_parser(_cmd); break;
  case '$': status = command_parser(_cmd); break;
  default: return STAT_OK; // Continue processing in command_lo()
  }

  _cmd = 0; // Command complete
  return status;
}


stat_t command_lo() {
  if (!_cmd || mp_get_planner_buffers_available() < PLANNER_BUFFER_HEADROOM ||
      usart_tx_full())
    return STAT_OK; // Wait

  if (estop_triggered()) return STAT_MACHINE_ALARMED;
  if (calibrate_busy()) return STAT_BUSY;
  if (mp_jog_busy()) return STAT_BUSY;

  stat_t status = gc_gcode_parser(_cmd);
  _cmd = 0;
  return status;
}


// Command functions
void static print_command_help(int i) {
  static const char fmt[] PROGMEM = "  $%-12S  %S\n";
  const char *name = pgm_read_word(&commands[i].name);
  const char *help = pgm_read_word(&commands[i].help);

  printf_P(fmt, name, help);
}


uint8_t command_help(int argc, char *argv[]) {
  if (argc == 2) {
    int i = command_find(argv[1]);

    if (i == -1) return STAT_UNRECOGNIZED_NAME;
    else print_command_help(i);

    return STAT_OK;
  }

  puts_P(PSTR("\nCommands:"));
  for (int i = 0; ; i++) {
    const char *name = pgm_read_word(&commands[i].name);
    if (!name) break;
    print_command_help(i);
    wdt_reset();
  }

  puts_P(PSTR("\nVariables:"));
  vars_print_help();

  return STAT_OK;
}


uint8_t command_reboot(int argc, char *argv[]) {
  hw_request_hard_reset();
  return 0;
}


uint8_t command_bootloader(int argc, char *argv[]) {
  hw_request_bootloader();
  return 0;
}


uint8_t command_save(int argc, char *argv[]) {
  vars_save();
  return 0;
}


uint8_t command_valid(int argc, char *argv[]) {
  printf_P(vars_valid() ? PSTR("true\n") : PSTR("false\n"));
  return 0;
}


uint8_t command_restore(int argc, char *argv[]) {
  return vars_restore();
}


uint8_t command_clear(int argc, char *argv[]) {
  vars_clear();
  return 0;
}