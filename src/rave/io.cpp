
#include "rave/io.hpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


struct IO_Context {
    HANDLE in;
    HANDLE out;
    Rave::SignalHandler handlers;
} io;


static volatile LONG s_signal = 0;

void
Rave::InitIO() {
    io.in = GetStdHandle(STD_INPUT_HANDLE);
    io.out = GetStdHandle(STD_OUTPUT_HANDLE);
    s_signal = 0;
}


int 
Rave::ReadIO(char *n, int len)
{
    if (io.in == INVALID_HANDLE_VALUE) return -1;
    DWORD bytesRead = 0;
    if (!ReadFile(io.in, n, static_cast<DWORD>(len), &bytesRead, nullptr)) return -1;
    return static_cast<int>(bytesRead);
}



int
Rave::WriteIO(const char *buffer, int len)
{
    if (io.out == INVALID_HANDLE_VALUE) return -1;
    DWORD n = 0;
    if (!WriteFile(io.out, buffer, static_cast<DWORD>(len), &n, nullptr)) return -1;
    return static_cast<int>(n);
}


static BOOL WINAPI SignalCtrlHandler(DWORD ctrl_type) {
    switch(ctrl_type) {
        case CTRL_BREAK_EVENT:
        case CTRL_C_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_CLOSE_EVENT:
            InterlockedExchange(&s_signal, 1);
            if(io.handlers.on_terminate) {
                io.handlers.on_terminate();
            }
            return TRUE;
        default:
            return FALSE;
    }
};


bool Rave::SignalInit(Rave::SignalHandler* handlers) {
    if(handlers) {
        io.handlers = *handlers;
    }
    return SetConsoleCtrlHandler(SignalCtrlHandler, TRUE) != 0;
}

bool Rave::SignalPoll() {
    return InterlockedCompareExchange(&s_signal, 0, 0) == 0;
}


#else


#include <unistd.h>
#include <signal.h>

struct IO_Context {

} io;

static volatile sig_atomic_t s_signal = 0;

void 
Rave::Init() {

void 
Rave::InitIO() {

    
}

int
Rave::ReadIO(char *n, int len) {
    return read(0, n, len);
}

int
Rave::WriteIO(const char *buffer, int len) {
    return write(1, buffer, len);
}

static void
SignalPosixHandler(int signum) {
    void(signum);
    s_signal = 1;
}

bool
Rave::SignalInit() {
    struct sigaction sa;
    sa.sa_handler = SignalPosixHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGINT, &sa, 0) != 0) return false;

    if(sigaction(SIGTERM, &sa, 0) != 0) return false;

    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    return true;
}



bool Rave::SignalPoll() {
    return s_shutdown == 0;
}



#endif