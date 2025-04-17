#pragma once

#include <filemon/os/defs.h>

namespace filemon
{
#pragma region MacOS defs
//
#if FILEMON_TARGET_MACOS

  using FileHandle = int;

#endif
//
#pragma endregion

#pragma region Linux defs
//
#if FILEMON_TARGET_LINUX

  using FileHandle = int;

#endif
//
#pragma endregion

#pragma region Win64 defs
//
#if FILEMON_TARGET_WIN64

  using FileHandle = HANDLE;

#endif
//
#pragma endregion
}