#pragma once

// https://sourceforge.net/p/predef/wiki/OperatingSystems/

#define FILEMON_TARGET_MACOS (__APPLE__ && __MACH__)

#define FILEMON_TARGET_LINUX (__linux__)

#define FILEMON_TARGET_WIN64 (_WIN64) // May make a Windows version someday
