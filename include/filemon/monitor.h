#pragma once

#include <string>
#include <thread>
#include <mutex>

#include <filemon/status.h>
#include <filemon/event.h>
#include <filemon/os/file.h>

namespace filemon
{

  using MonitorCallback = void(filemon::Event &);
  // using ErrorCallback = void(filemon::Status);

  // TODO: Let's not use blocking and run the event check on another thread
  class FileMonitor
  {

    // Private props
  private:
    const char *m_fileName;
    FileHandle m_file;

    std::mutex m_runMutex;

    bool m_isRunning;
    std::thread m_runThread;
    MonitorCallback *m_callback;

    // We can use this to ensure an attribute change is for the size of a file
    // size_t m_fileSize;

// OS-dependent
#if FILEMON_TARGET_LINUX

    int m_notifDesc;
    int m_notifWatch;

    // Self-pipe to stop read() from blocking
    int m_pipeReadDesc;
    int m_pipeWriteDesc;

#endif

#if FILEMON_TARGET_MACOS

    int m_kqueueDesc;

    // One will have a filter for file descriptors, the other will have a filter for user events
    kevent64_s m_changeList[2];

    // One will be a file-related event, the other will be the user event to stop kevent from blocking
    // kevent64_s m_eventList[2];

#endif

    // Private methods
  private:
#ifdef FILEMON_TARGET_LINUX
    static void ThreadFn(int file, int notifDesc, int pipeReadDesc, MonitorCallback *callback);

#elif
    // May not need to pass the entire class
    static void ThreadFn(FileMonitor *monitor);
#endif

  public:
    // C/dtors
    FileMonitor(std::string &fileName);
    FileMonitor(const char *fileName);
    ~FileMonitor();

    // Methods

    // Start monitoring
    filemon::Status start(MonitorCallback *callback);

    FileHandle getFile();

    // Stop monitoring
    void stop();
  };

}