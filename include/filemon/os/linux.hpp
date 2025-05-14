// #pragma once

// #if FILEMON_TARGET_LINUX
// //

// #include <fcntl.h>
// #include <sys/stat.h>
// #include <sys/inotify.h>
// #include <unistd.h>

// #include <filemon/os/defs.h>
// #include <filemon/monitor.h>

// #pragma region C/dtor impl

// filemon::FileMonitor::FileMonitor(std::string &fileName)
// {
//   m_fileName = fileName.c_str();
// }

// filemon::FileMonitor::FileMonitor(const char *fileName)
// {
//   m_fileName = fileName;
// }

// filemon::FileMonitor::~FileMonitor()
// {
//   // this->close() ?
// }

// #pragma endregion

// #pragma region Method impl

// filemon::Status filemon::FileMonitor::open()
// {
//   m_file = ::open(m_fileName, O_RDONLY);
//   if (m_file == -1)
//   {
//     return filemon::Status::OpenFailed;
//   }

//   m_notifInstance = inotify_init();
//   if (m_notifInstance == -1)
//   {
//     return filemon::Status::INotifyInitFailed;
//   }

//   m_notifWatch = inotify_add_watch(m_notifInstance, m_fileName, (IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF));
//   if (m_notifWatch == -1)
//   {
//     return filemon::Status::INotifyAddWatchFailed;
//   }

//   return filemon::Status::Ok;
// }

// filemon::Status filemon::FileMonitor::close()
// {

//   // These can fail?

//   if (inotify_rm_watch(m_notifInstance, m_notifWatch) == -1)
//   {
//     return filemon::Status::INotifyRemoveWatchFailed;
//   }

//   if (::close(m_notifInstance) == -1)
//   {
//     return filemon::Status::INotifyCloseFailed;
//   }

//   if (::close(m_file) == -1)
//   {
//     return filemon::Status::CloseFailed;
//   }

//   return filemon::Status::Ok;
// }

// filemon::Status filemon::FileMonitor::getEvent(filemon::Event &ev)
// {

//   inotify_event notifEvent;

//   // This blocks. Should I change that in the future?
//   // Read from the inotify descriptor, not the watch
//   size_t bytesRead = read(m_notifInstance, &notifEvent, sizeof(inotify_event));
//   if (bytesRead == -1)
//   {
//     return filemon::Status::INotifyReadFailed;
//   }

//   // Was it moved or deleted?
//   if (notifEvent.mask & (IN_DELETE_SELF | IN_MOVE_SELF))
//   {

//     ev.type = (notifEvent.mask & IN_DELETE_SELF) ? filemon::EventType::Delete : filemon::EventType::Modify;
//     ev.fileSize = 0;

//     return (notifEvent.mask & IN_DELETE_SELF) ? filemon::Status::FileDeleted : filemon::Status::FileMoved;
//   }

//   if (notifEvent.mask & IN_MODIFY)
//   {
//     struct stat64 fileStats;
//     if (fstat64(m_file, &fileStats) == -1)
//     {
//       return filemon::Status::StatFailed;
//     }

//     // Either the status or type are redundant
//     ev.type = filemon::EventType::Modify;
//     ev.fileSize = fileStats.st_size;
//   }

//   return filemon::Status::Ok;
// }

// #pragma endregion

// //
// #endif