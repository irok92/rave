#include "rv_cli.h"
#include "rv_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct rv_cli {
    rv_context* ctx;
    rv_cli_flags flags;
};

static rv_cli g_rv_cli;

rv_cli_flags rv_cli_find_flags() {
    unsigned long long flags = RV_CLI_HAS_NONE;

    const char* term = getenv("TERM");
    const char* term_program = getenv("TERM_PROGRAM");
    const char* colorterm = getenv("COLORTERM");
    const char* lc_all = getenv("LC_ALL");
    const char* lc_ctype = getenv("LC_CTYPE");
    const char* lang = getenv("LANG");

    const unsigned long long input_flags =
        RV_CLI_HAS_BRACKETED_PASTE |
        RV_CLI_HAS_FOCUS_EVENTS |
        RV_CLI_HAS_MOUSE_SGR |
        RV_CLI_HAS_ALT_SCREEN |
        RV_CLI_HAS_OSC52_CLIPBOARD |
        RV_CLI_HAS_CURSOR_SHAPE;

    const unsigned long long basic_terminal_flags =
        RV_CLI_HAS_UTF8 |
        RV_CLI_HAS_COLOR256 |
        input_flags;

    const unsigned long long modern_terminal_flags =
        basic_terminal_flags |
        RV_CLI_HAS_TRUECOLOR |
        RV_CLI_HAS_OSC8_HYPERLINKS;

    if(term && strcmp(term, "dumb") != 0) {
        flags |= RV_CLI_HAS_VT_ESCAPE | RV_CLI_HAS_COLOR16;
    }

    if((lc_all && (strstr(lc_all, "UTF-8") || strstr(lc_all, "UTF8") || strstr(lc_all, "utf8")))
    || (lc_ctype && (strstr(lc_ctype, "UTF-8") || strstr(lc_ctype, "UTF8") || strstr(lc_ctype, "utf8")))
    || (lang && (strstr(lang, "UTF-8") || strstr(lang, "UTF8") || strstr(lang, "utf8")))) {
        flags |= RV_CLI_HAS_UTF8;
    }

    if((term && strstr(term, "256color"))
    || (colorterm && strstr(colorterm, "256"))) {
        flags |= RV_CLI_HAS_COLOR256;
    }

    if((colorterm && (strstr(colorterm, "truecolor") || strstr(colorterm, "24bit")))
    || (term && (strstr(term, "truecolor") || strstr(term, "24bit")))) {
        flags |= RV_CLI_HAS_TRUECOLOR;
    }

    if(getenv("SSH_CONNECTION") || getenv("SSH_CLIENT") || getenv("SSH_TTY")) {
        flags |= RV_CLI_IS_REMOTE;
    }

    if((term && strstr(term, "xterm-kitty")) || getenv("KITTY_WINDOW_ID") || getenv("KITTY_PID")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD |
            RV_CLI_HAS_KITTY_GRAPHICS;
    }
    else if((term_program && strcmp(term_program, "WezTerm") == 0) || getenv("WEZTERM_EXECUTABLE") || getenv("WEZTERM_PANE")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD |
            RV_CLI_HAS_KITTY_GRAPHICS;
    }
    else if((term_program && strcmp(term_program, "iTerm.app") == 0) || getenv("ITERM_SESSION_ID")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_MODIFY_OTHER_KEYS |
            RV_CLI_HAS_ITERM2_INLINE_IMAGES;
    }
    else if((term_program && strcmp(term_program, "ghostty") == 0) || getenv("GHOSTTY_RESOURCES_DIR") || (term && strstr(term, "ghostty"))) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD;
    }
    else if((term_program && strcmp(term_program, "alacritty") == 0) || getenv("ALACRITTY_SOCKET") || (term && strstr(term, "alacritty"))) {
        flags |= modern_terminal_flags;
    }
    else if(term_program && strcmp(term_program, "vscode") == 0) {
        flags |= modern_terminal_flags;
    }
    else if(getenv("FOOT_CLIENT") || (term && strstr(term, "foot"))) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD;
    }
    else if(getenv("KONSOLE_VERSION")) {
        flags |= modern_terminal_flags;
    }
    else if(getenv("VTE_VERSION")) {
        flags |= modern_terminal_flags;
    }
    else if(getenv("WT_SESSION")) {
        flags |= modern_terminal_flags;
    }
    else if(getenv("DOMTERM")) {
        flags |= modern_terminal_flags;
    }
    else if(term && strstr(term, "mlterm")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SIXEL_GRAPHICS;
    }
    else if(term && strstr(term, "rxvt")) {
        flags |= basic_terminal_flags;
    }
    else if(term && (strcmp(term, "st") == 0 || strstr(term, "st-") || strstr(term, "-st"))) {
        flags |= basic_terminal_flags;
    }
    else if(getenv("XTERM_VERSION") || (term && strstr(term, "xterm"))) {
        flags |= basic_terminal_flags |
            RV_CLI_HAS_MODIFY_OTHER_KEYS;
    }

    if(getenv("TMUX") || (term && strstr(term, "tmux"))) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT;
    }
    else if(getenv("STY") || (term && strstr(term, "screen"))) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags;
    }
    else if(getenv("ZELLIJ")) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags;
    }

    if(term && strstr(term, "sixel")) {
        flags |= RV_CLI_HAS_SIXEL_GRAPHICS;
    }

    return (rv_cli_flags) flags;
}

bool rv_cli_init(int argc, char* argv[]) {
    rv_cli_find_flags();

    g_rv_cli.ctx = rv_context_create();

    if(!g_rv_cli.ctx) {
        return false;
    }

    return true;
}

static int counter = 30;
bool rv_cli_update() {
    char buffer[64];
    int out = read(STDIN_FILENO, &buffer, 64);

    printf("%i\n", out);
    return counter-- > 0;
}

void rv_cli_destroy() {
    rv_context_destroy();
}

rv_cli_flags rv_cli_get_flags() {
    return g_rv_cli.flags;
}

bool rv_cli_has_flag(rv_cli_flags flag) {
    return ((unsigned long long)g_rv_cli.flags & (unsigned long long)flag) != 0ull;
}
