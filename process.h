#ifndef PROCESS_H
#define PROCESS_H

#include <Windows.h>
#include <TlHelp32.h>
#include <Wtsapi32.h>
#include <UserEnv.h>

BOOL RunAsInteractiveSystem(LPCWSTR lpszProcess);

#endif // PROCESS_H
