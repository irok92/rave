#include "rv_cli.h"
#include "rv_context.h"
#include "rv_cli_os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rv_cli {
    rv_context* ctx;
    rv_cli_flags flags;
} rv_cli;

rv_cli g_rv_cli;

rv_cli_flags rv_cli_find_flags() {
    unsigned long long flags = RV_CLI_HAS_NONE;

    const char* term = rv_getenv("TERM");
    const char* term_program = rv_getenv("TERM_PROGRAM");
    const char* colorterm = rv_getenv("COLORTERM");
    const char* lc_all = rv_getenv("LC_ALL");
    const char* lc_ctype = rv_getenv("LC_CTYPE");
    const char* lang = rv_getenv("LANG");

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

    if(rv_getenv("SSH_CONNECTION") || rv_getenv("SSH_CLIENT") || rv_getenv("SSH_TTY")) {
        flags |= RV_CLI_IS_REMOTE;
    }

    if((term && strstr(term, "xterm-kitty")) || rv_getenv("KITTY_WINDOW_ID") || rv_getenv("KITTY_PID")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD |
            RV_CLI_HAS_KITTY_GRAPHICS;
    }
    else if((term_program && strcmp(term_program, "WezTerm") == 0) || rv_getenv("WEZTERM_EXECUTABLE") || rv_getenv("WEZTERM_PANE")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD |
            RV_CLI_HAS_KITTY_GRAPHICS;
    }
    else if((term_program && strcmp(term_program, "iTerm.app") == 0) || rv_getenv("ITERM_SESSION_ID")) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_MODIFY_OTHER_KEYS |
            RV_CLI_HAS_ITERM2_INLINE_IMAGES;
    }
    else if((term_program && strcmp(term_program, "ghostty") == 0) || rv_getenv("GHOSTTY_RESOURCES_DIR") || (term && strstr(term, "ghostty"))) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD |
            RV_CLI_HAS_KITTY_KEYBOARD;
    }
    else if((term_program && strcmp(term_program, "alacritty") == 0) || rv_getenv("ALACRITTY_SOCKET") || (term && strstr(term, "alacritty"))) {
        flags |= modern_terminal_flags;
    }
    else if(term_program && strcmp(term_program, "vscode") == 0) {
        flags |= modern_terminal_flags;
    }
    else if(rv_getenv("FOOT_CLIENT") || (term && strstr(term, "foot"))) {
        flags |= modern_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT |
            RV_CLI_HAS_CSI_U_KEYBOARD;
    }
    else if(rv_getenv("KONSOLE_VERSION")) {
        flags |= modern_terminal_flags;
    }
    else if(rv_getenv("VTE_VERSION")) {
        flags |= modern_terminal_flags;
    }
    else if(rv_getenv("WT_SESSION")) {
        flags |= modern_terminal_flags;
    }
    else if(rv_getenv("DOMTERM")) {
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
    else if(rv_getenv("XTERM_VERSION") || (term && strstr(term, "xterm"))) {
        flags |= basic_terminal_flags |
            RV_CLI_HAS_MODIFY_OTHER_KEYS;
    }

    if(rv_getenv("TMUX") || (term && strstr(term, "tmux"))) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags |
            RV_CLI_HAS_SYNC_OUTPUT;
    }
    else if(rv_getenv("STY") || (term && strstr(term, "screen"))) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags;
    }
    else if(rv_getenv("ZELLIJ")) {
        flags |= RV_CLI_IS_MULTIPLEXED |
            basic_terminal_flags;
    }

    if(term && strstr(term, "sixel")) {
        flags |= RV_CLI_HAS_SIXEL_GRAPHICS;
    }

    return (rv_cli_flags) flags;
}

bool rv_cli_init(int argc, char* argv[]) {

    g_rv_cli.ctx = rv_context_create(RV_NULL);
    g_rv_cli.flags = rv_cli_find_flags();

    printf("Detected terminal capabilities:\n");
    if(g_rv_cli.flags & RV_CLI_HAS_VT_ESCAPE) printf("- VT Escape\n");
    if(g_rv_cli.flags & RV_CLI_HAS_UTF8) printf("- UTF-8\n");
    if(g_rv_cli.flags & RV_CLI_HAS_COLOR16) printf("- 16 color\n");
	if(g_rv_cli.flags & RV_CLI_HAS_COLOR256) printf("- 256 color\n");
	if(g_rv_cli.flags & RV_CLI_HAS_TRUECOLOR) printf("- Truecolor\n");
	if(g_rv_cli.flags & RV_CLI_HAS_OSC8_HYPERLINKS) printf("- OSC8 Hyperlinks\n");
	if(g_rv_cli.flags & RV_CLI_HAS_BRACKETED_PASTE) printf("- Bracketed Paste\n");
	if(g_rv_cli.flags & RV_CLI_HAS_FOCUS_EVENTS) printf("- Focus Events\n");
	if(g_rv_cli.flags & RV_CLI_HAS_MOUSE_SGR) printf("- SGR Mouse\n");
	if(g_rv_cli.flags & RV_CLI_HAS_ALT_SCREEN) printf("- Alternate Screen\n");
	if(g_rv_cli.flags & RV_CLI_HAS_SYNC_OUTPUT) printf("- Synchronous Output\n");
	if(g_rv_cli.flags & RV_CLI_HAS_OSC52_CLIPBOARD) printf("- OSC52 Clipboard\n");
	if(g_rv_cli.flags & RV_CLI_HAS_CURSOR_SHAPE) printf("- Cursor Shape\n");
	if(g_rv_cli.flags & RV_CLI_HAS_MODIFY_OTHER_KEYS) printf("- Modify Other Keys\n");
	if(g_rv_cli.flags & RV_CLI_HAS_CSI_U_KEYBOARD) printf("- CSI u Keyboard\n");
	if(g_rv_cli.flags & RV_CLI_HAS_KITTY_KEYBOARD) printf("- Kitty Keyboard\n");
	if(g_rv_cli.flags & RV_CLI_HAS_SIXEL_GRAPHICS) printf("- Sixel Graphics\n");
	if(g_rv_cli.flags & RV_CLI_HAS_KITTY_GRAPHICS) printf("- Kitty Graphics\n");
	if(g_rv_cli.flags & RV_CLI_HAS_ITERM2_INLINE_IMAGES) printf("- iTerm2 Inline Images\n");
	if(g_rv_cli.flags & RV_CLI_IS_MULTIPLEXED) printf("- Multiplexed\n");
	if(g_rv_cli.flags & RV_CLI_IS_REMOTE) printf("- Remote\n");


    if(!g_rv_cli.ctx) {
        return false;
    }

    return true;
}

static int counter = 30;
bool rv_cli_update() {
    //char buffer[64];
    //int out = fread(STDIN_FILENO, &buffer, 64);

    printf("%i\n", counter);
    return counter-- > 0;
}

void rv_cli_destroy() {
    rv_context_destroy(RV_NULL);
}

rv_cli_flags rv_cli_get_flags() {
    return g_rv_cli.flags;
}

bool rv_cli_has_flag(rv_cli_flags flag) {
    return ((unsigned long long)g_rv_cli.flags & (unsigned long long)flag) != 0ull;
}
