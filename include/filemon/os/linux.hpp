#pragma once

#include <filemon/os/defs.h>

#if FILEMON_TARGET_LINUX
//

#include <poll.h>
#include <fcntl.h>
#include <cassert>
#include <unistd.h>
#include <sys/inotify.h>

#include <filemon/monitor.h>
#include <filemon/util.hpp>

#pragma region Static methods
void filemon::FileMonitor::ThreadFn(int file, int notifDesc, int pipeReadDesc, MonitorCallback *callback)
{

  // Poll setup
  struct pollfd pollDescs[2] = {{pipeReadDesc, POLLIN, 0}, {notifDesc, POLLIN, 0}};
  pollfd *pollPipe = &pollDescs[0];
  pollfd *pollNotif = &pollDescs[1];

  bool shouldRun = true;
  while (shouldRun)
  {

    int pollResult = poll(pollDescs, (sizeof(pollDescs) / sizeof(pollfd)), -1);

    // Continue if zero
    if (pollResult == 0)
    {
      continue;
    }

    // Received message to stop
    if ((pollPipe->revents & POLLIN) == 1)
    {
      shouldRun = false;
    }
    // Received notif
    else if ((pollNotif->revents & POLLIN) == 1)
    {

      inotify_event notifEvent;
      size_t bytesRead = read(notifDesc, &notifEvent, sizeof(inotify_event));
      // assert(bytesRead != -1); Should never be -1 because poll accepted it

      Event ev;

      if (notifEvent.mask & (IN_DELETE_SELF | IN_MOVE_SELF))
      {
        ev.type = ((notifEvent.mask & IN_DELETE_SELF) ? EventType::Delete : EventType::Move);
        ev.fileSize = 0;
      }
      else
      {
        ev.type = EventType::Modify;
        ev.fileSize = filemon::util::getFileSize(file);
      }

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

  m_notifDesc = -1;
  m_notifWatch = -1;
  m_pipeReadDesc = -1;
  m_pipeWriteDesc = -1;
}
filemon::FileMonitor::FileMonitor(const char *fileName)
{
  m_fileName = fileName;
  m_file = -1;
  m_isRunning = false;
  m_callback = nullptr;

  m_notifDesc = -1;
  m_notifWatch = -1;
  m_pipeReadDesc = -1;
  m_pipeWriteDesc = -1;
}

filemon::FileMonitor::~FileMonitor()
{
  if (m_isRunning)
  {
    this->stop();
  }

  inotify_rm_watch(m_notifDesc, m_notifWatch);
  close(m_notifDesc);
  close(m_file);
}

#pragma endregion

#pragma region Methods

// Maybe have an error callback and remove the return type?
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

  // We haven't created everything
  if (m_notifDesc == -1)
  {
    // Is it realistic to expect these to fail?
    m_notifDesc = inotify_init();
    m_notifWatch = inotify_add_watch(m_notifDesc, m_fileName, (IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF));

    int pipeDescs[2];
    pipe(pipeDescs);
    m_pipeReadDesc = pipeDescs[0];
    m_pipeWriteDesc = pipeDescs[1];
  }

  m_callback = callback;

  m_runThread = std::thread(filemon::FileMonitor::ThreadFn, m_file, m_notifDesc, m_pipeReadDesc, m_callback);

  m_isRunning = true;

  return Status::Ok;
}

void filemon::FileMonitor::stop()
{

  std::lock_guard<std::mutex> lock(m_runMutex);

  if (!m_isRunning)
  {
    return;
  }

  write(m_pipeWriteDesc, "\0", 1);

  // Join thread
  m_runThread.join();

  m_isRunning = false;
}

#pragma endregion

//
#endif