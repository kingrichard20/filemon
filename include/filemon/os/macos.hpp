#pragma once

#include <filemon/os/file.h>
#include <filemon/monitor.h>
#include <filemon/util.hpp>

#if FILEMON_TARGET_MACOS
//

const int KQUEUE_STOP_ID = -1;
const unsigned long KQUEUE_NCHANGESEVENTS = 2;

#pragma region Static methods
void filemon::FileMonitor::ThreadFn(int file, int kqueueDesc, const kevent64_s *changeList, MonitorCallback *callback)
{
  size_t fileSize = filemon::util::getFileSize(file);

  bool shouldRun = true;
  while (shouldRun)
  {
    kevent64_s eventList[2];

    int evCount = kevent64(kqueueDesc, changeList, KQUEUE_NCHANGESEVENTS, eventList, KQUEUE_NCHANGESEVENTS, 0, nullptr);
    // TODO: Handle errors
    if (evCount == -1)
    {
      int error = errno;
      assert(false);
    }

    for (int i = evCount - 1; i >= 0; --i)
    {

      kevent64_s *currentEvent = &eventList[i];

      // User-fired event, exit loop
      // if ((currentEvent->filter & EVFILT_USER) == 1)
      if (currentEvent->ident == KQUEUE_STOP_ID)
      {
        shouldRun = false;
        break;
      }

      // On return, fflags contains	the events which triggered the filter.
      // https://man.freebsd.org/cgi/man.cgi?kevent

      Event ev;

      if (currentEvent->fflags & (NOTE_WRITE | NOTE_ATTRIB))
      {
        size_t newFileSize = filemon::util::getFileSize(file);

        // Size did not change
        if (newFileSize == fileSize)
        {
          continue;
        }

        // Set new file size
        fileSize = newFileSize;

        ev.type = EventType::Modify;
        ev.fileSize = fileSize;
      }
      else if (currentEvent->fflags & (NOTE_RENAME | NOTE_DELETE))
      {
        ev.type = ((currentEvent->fflags & NOTE_RENAME) ? EventType::Move : EventType::Delete);
        ev.fileSize = 0;
      }

      // Call callback
      callback(ev);
    }
  }
}

#pragma endregion

#pragma region C/dtor impl

filemon::FileMonitor::FileMonitor(std::string &fileName)
{
  m_fileName = fileName.c_str();
  m_file = -1;
  m_isRunning = false;
  m_callback = nullptr;
  m_kqueueDesc = -1;
}
filemon::FileMonitor::FileMonitor(const char *fileName)
{
  m_fileName = fileName;
  m_file = -1;
  m_isRunning = false;
  m_callback = nullptr;
  m_kqueueDesc = -1;
}

filemon::FileMonitor::~FileMonitor()
{
  if (m_isRunning)
  {
    this->stop();
  }

  close(m_kqueueDesc);
  close(m_file);
}

#pragma endregion

#pragma region Methods

filemon::Status filemon::FileMonitor::start(MonitorCallback *callback)
{
  std::lock_guard<std::mutex> lock(m_runMutex);

  if (m_isRunning)
  {
    return filemon::Status::AlreadyRunning;
  }

  if (m_file == -1 || fcntl(m_file, F_GETFD) == -1)
  {
    m_file = open(m_fileName, O_RDONLY);
    if (m_file == -1)
    {
      return Status::OpenFailed;
    }
  }

  // Create kqueue and set up events
  if (m_kqueueDesc == -1)
  {
    m_kqueueDesc = kqueue();
  }
  // Changelist
  // File descriptor event
  EV_SET64(
      &m_changeList[0],
      m_file,
      EVFILT_VNODE,
      (EV_ADD | EV_ENABLE | EV_CLEAR),
      (NOTE_WRITE | NOTE_ATTRIB | NOTE_DELETE | NOTE_RENAME),
      0, 0, 0, 0);

  // User event
  EV_SET64(
      &m_changeList[1],
      KQUEUE_STOP_ID,
      EVFILT_USER,
      (EV_ADD | EV_ENABLE | EV_CLEAR),
      NOTE_FFNOP,
      0, 0, 0, 0);

  // There may be some scenario where we open the file
  // but another process deletes the file before we can call fstat()
  // m_fileSize = util::getFileSize(m_file);

  m_callback = callback;

  // void ThreadFn(int file, int kqueueDesc, const kevent64_s *changeList, MonitorCallback *callback)

  m_runThread = std::thread(filemon::FileMonitor::ThreadFn, m_file, m_kqueueDesc, m_changeList, m_callback);

  m_isRunning = true;

  // m_runThread.join();

  return Status::Ok;
}

filemon::FileHandle filemon::FileMonitor::getFile()
{
  std::lock_guard<std::mutex> lock(m_runMutex);

  return m_file;
}

void filemon::FileMonitor::stop()
{
  std::lock_guard<std::mutex> lock(m_runMutex);

  if (!m_isRunning)
  {
    return;
  }

  // Send stop event to the kqueue
  struct kevent64_s stopEvent;
  // Use -1 as the event ID. It shouldn't conflict with anything
  EV_SET64(&stopEvent, KQUEUE_STOP_ID, EVFILT_USER, 0, NOTE_TRIGGER, 0, 0, 0, 0);

  kevent64(m_kqueueDesc, &stopEvent, 1, nullptr, 0, 0, nullptr);

  // kevent64(m_kqueueDesc, &stopEvent, 1, nullptr, 0, 0, nullptr);

  // Join thread
  m_runThread.join();

  m_isRunning = false;
}

#pragma endregion

//
#endif