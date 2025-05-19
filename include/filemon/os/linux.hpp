#pragma once

#include <filemon/monitor.h>
#include <filemon/os/defs.h>

#if FILEMON_TARGET_LINUX
//

#pragma region Static methods
void filemon::FileMonitor::ThreadFn(filemon::FileMonitor *monitor)
{

  // Poll setup
  struct pollfd pollDescs[2] = {{monitor->m_pipeReadDesc, POLLIN, 0}, {monitor->m_notifInstance, POLLIN, 0}};
  nfds_t pollDescCount = (sizeof(pollDescs) / sizeof(pollfd));
  pollfd *pollPipeRead = &pollDescs[0];
  pollfd *pollNotif = &pollDescs[1];

  bool shouldRun = true;
  inotify_event notifEvent;

  while (shouldRun)
  {
    int pollResult = poll(pollDescs, pollDescCount, -1);
    assert(0 < pollResult);

    // Stop looping?
    shouldRun = ((pollPipeRead->revents & POLLIN) != 1);

    // Continue if there's nothing
    if (!(pollNotif->revents & POLLIN))
    {
      continue;
    }

    size_t bytesRead = read(monitor->m_notifInstance, &notifEvent, sizeof(inotify_event));
    assert(bytesRead != -1);

    if (notifEvent.mask & (IN_DELETE_SELF | IN_MOVE_SELF))
    {

      // Call callback
      Event ev = {(notifEvent.mask & IN_DELETE_SELF) ? EventType::Delete : EventType::Move, 0};
      monitor->m_callback(ev);
    }
    else
    {
      size_t fileSize = filemon::util::getFileSize(monitor->m_file);

      // Call callback
      Event ev = {EventType::Modify, fileSize};
      monitor->m_callback(ev);
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

  m_notifInstance = -1;
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

  m_notifInstance = -1;
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

  inotify_rm_watch(m_notifInstance, m_notifWatch);
  close(m_notifInstance);
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
  if (m_notifInstance == -1)
  {
    // Is it realistic to expect these to fail?
    m_notifInstance = inotify_init();
    m_notifWatch = inotify_add_watch(m_notifInstance, m_fileName, (IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF));

    int pipeDescs[2];
    pipe(pipeDescs);
    m_pipeReadDesc = pipeDescs[0];
    m_pipeWriteDesc = pipeDescs[1];
  }

  m_callback = callback;

  m_runThread = std::thread(filemon::FileMonitor::ThreadFn, this);

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