#include "signalhandler.h"
#include <assert.h>
#include <cstddef>

#if 1

#include <signal.h>

#else

#include <windows.h>

#endif //!__MINGW32_MAJOR_VERSION

// There can be only ONE SignalHandler per process
SignalHandler *g_handler(NULL);

#if 0

BOOL WINAPI WIN32_handleFunc(DWORD);
int WIN32_physicalToLogical(DWORD);
DWORD WIN32_logicalToPhysical(int);
std::set<int> g_registry;

#else //__MINGW32_MAJOR_VERSION

void POSIX_handleFunc(int);
int POSIX_physicalToLogical(int);
int POSIX_logicalToPhysical(int);

#endif //__MINGW32_MAJOR_VERSION

SignalHandler::SignalHandler(int mask) : _mask(mask) {
    assert(g_handler == NULL);
    g_handler = this;

#if 0
    SetConsoleCtrlHandler(WIN32_handleFunc, TRUE);
#endif //__MINGW32_MAJOR_VERSION

    for (int i = 0; i < numSignals; i++) {
        int logical = 0x1 << i;
        if (_mask & logical) {
#if 0
            g_registry.insert(logical);
#else
            int sig = POSIX_logicalToPhysical(logical);
            bool failed = signal(sig, POSIX_handleFunc) == SIG_ERR;
            assert(!failed);
            (void)failed; // Silence the warning in non _DEBUG; TODO: something better

#endif //__MINGW32_MAJOR_VERSION
        }
    }
}

SignalHandler::~SignalHandler() {
#if 0
    SetConsoleCtrlHandler(WIN32_handleFunc, FALSE);
#else
    for (int i = 0; i < numSignals; i++) {
        int logical = 0x1 << i;
        if (_mask & logical) {
            signal(POSIX_logicalToPhysical(logical), SIG_DFL);
        }
    }
#endif //__MINGW32_MAJOR_VERSION

    // permit creation of a new SignalHandler
    g_handler = NULL;
}

#if 0
DWORD WIN32_logicalToPhysical(int signal)
{
    switch (signal)
    {
    case SignalHandler::SIG_INT: return CTRL_C_EVENT;
    case SignalHandler::SIG_TERM: return CTRL_BREAK_EVENT;
    case SignalHandler::SIG_CLOSE: return CTRL_CLOSE_EVENT;
    default:
        return ~(unsigned int)0; // SIG_ERR = -1
    }
}
#else
int POSIX_logicalToPhysical(int signal) {
    switch (signal) {
    case SignalHandler::SIG_INT:
        return SIGINT;
    case SignalHandler::SIG_TERM:
        return SIGTERM;
    // In case the client asks for a SIG_CLOSE handler, accept and
    // bind it to a SIGTERM. Anyway the signal will never be raised
    case SignalHandler::SIG_CLOSE:
        return SIGTERM;
    // case SignalHandler::SIG_RELOAD: return SIGHUP;
    default:
        return -1; // SIG_ERR = -1
    }
}
#endif //__MINGW32_MAJOR_VERSION

#if 0
int WIN32_physicalToLogical(DWORD signal)
{
    switch (signal)
    {
    case CTRL_C_EVENT: return SignalHandler::SIG_INT;
    case CTRL_BREAK_EVENT: return SignalHandler::SIG_TERM;
    case CTRL_CLOSE_EVENT: return SignalHandler::SIG_CLOSE;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}
#else
int POSIX_physicalToLogical(int signal) {
    switch (signal) {
    case SIGINT:
        return SignalHandler::SIG_INT;
    case SIGTERM:
        return SignalHandler::SIG_TERM;
    // case SIGHUP: return SignalHandler::SIG_RELOAD;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}
#endif //__MINGW32_MAJOR_VERSION

#if 0
BOOL WINAPI WIN32_handleFunc(DWORD signal)
{
    if (g_handler)
    {
        int signo = WIN32_physicalToLogical(signal);
        // The std::set is thread-safe in const reading access and we never
        // write to it after the program has started so we don't need to
        // protect this search by a mutex
        std::set<int>::const_iterator found = g_registry.find(signo);
        if (signo != -1 && found != g_registry.end())
        {
            return g_handler->handleSignal(signo) ? TRUE : FALSE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}
#else
void POSIX_handleFunc(int signal) {
    if (g_handler) {
        int signo = POSIX_physicalToLogical(signal);
        g_handler->handleSignal(signo);
    }
}
#endif //__MINGW32_MAJOR_VERSION
