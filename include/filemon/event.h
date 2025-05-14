#pragma once

#include <cstddef>
#include <filemon/status.h>

namespace filemon
{

  enum EventType
  {
    Modify,
    Delete,
    Move
  };

  struct Event
  {
    EventType type;
    size_t fileSize;
    // Status status;
  };

}
