#pragma once

#include <cstddef>
#include <sys/stat.h>
#include <filemon/os/file.h>

#if FILEMON_TARGET_MACOS
//

// Needed for stat.st_size to be 64 bits?
// https://groups.google.com/g/zfs-macos/c/W6JnWSRYK4Y?pli=1
#define _DARWIN_USE_64_BIT_INODE 1

namespace filemon::util
{

  size_t getFileSize(FileHandle file)
  {
    struct stat fileStats;
    fstat(file, &fileStats);

    return fileStats.st_size;
  }

}
//
#endif

#if FILEMON_TARGET_LINUX
//

namespace filemon::util
{

  size_t getFileSize(FileHandle file)
  {
    struct stat64 fileStats;
    fstat64(file, &fileStats);

    return fileStats.st_size;
  }

}

//
#endif