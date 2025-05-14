#pragma once

namespace filemon
{

  // Not sure which I will use
  enum Status
  {
    Ok = 0,

    // The file probably does not exist
    OpenFailed,
    // CloseFailed,
    FileMoved,
    FileDeleted,

    AlreadyRunning,

    // Couldn't get file size
    // StatFailed,

    // Linux-only
    // INotifyInitFailed,
    // INotifyAddWatchFailed,
    // INotifyReadFailed,
    // INotifyRemoveWatchFailed,
    // INotifyCloseFailed,

  };

}

#define FILEMON_OK(status) status == filemon::Status::Ok
