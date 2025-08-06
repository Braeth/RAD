#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

int enable_virtual_processing() {

    #ifdef _WIN32
        HANDLE hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
        DWORD dwMode;

        GetConsoleMode( hOutput, &dwMode );
        dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        if (!SetConsoleMode(hOutput, dwMode)) {
            return 0;
        } else {
            return 1;
        }
    #endif

}