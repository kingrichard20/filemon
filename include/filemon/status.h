#pragma once

namespace filemon
{

  enum Status
  {
    Ok = 0,
    OpenFailed,
    CloseFailed,
    FileMoved,
    FileDeleted,

    // Couldn't get file size
    StatFailed,

    // Linux-only
    INotifyInitFailed,
    INotifyAddWatchFailed,
    INotifyReadFailed,
    INotifyRemoveWatchFailed,
    INotifyCloseFailed,

  };

}

#define FILEMON_OK(status) status == filemon::Status::Ok
