#pragma once

enum rv_cli_terminal_type {

};

enum rv_cli_flags : unsigned long long {
  RV_CLI_HAS_NONE = 0ull,

  RV_CLI_HAS_VT_ESCAPE = 1ull << 0,
  RV_CLI_HAS_UTF8 = 1ull << 1,
  RV_CLI_HAS_COLOR16 = 1ull << 2,
  RV_CLI_HAS_COLOR256 = 1ull << 3,
  RV_CLI_HAS_TRUECOLOR = 1ull << 4,

  RV_CLI_HAS_OSC8_HYPERLINKS = 1ull << 5,
  RV_CLI_HAS_BRACKETED_PASTE = 1ull << 6,
  RV_CLI_HAS_FOCUS_EVENTS = 1ull << 7,
  RV_CLI_HAS_MOUSE_SGR = 1ull << 8,
  RV_CLI_HAS_ALT_SCREEN = 1ull << 9,
  RV_CLI_HAS_SYNC_OUTPUT = 1ull << 10,
  RV_CLI_HAS_OSC52_CLIPBOARD = 1ull << 11,
  RV_CLI_HAS_CURSOR_SHAPE = 1ull << 12,

  RV_CLI_HAS_MODIFY_OTHER_KEYS = 1ull << 13,
  RV_CLI_HAS_CSI_U_KEYBOARD = 1ull << 14,
  RV_CLI_HAS_KITTY_KEYBOARD = 1ull << 15,

  RV_CLI_HAS_SIXEL_GRAPHICS = 1ull << 16,
  RV_CLI_HAS_KITTY_GRAPHICS = 1ull << 17,
  RV_CLI_HAS_ITERM2_INLINE_IMAGES = 1ull << 18,

  RV_CLI_IS_MULTIPLEXED = 1ull << 19,
  RV_CLI_IS_REMOTE = 1ull << 20,
};

bool rv_cli_init(int argc, char* argv[]);
bool rv_cli_update();
void rv_cli_destroy();

rv_cli_flags rv_cli_get_flags();
bool rv_cli_has_flag(rv_cli_flags flag);
