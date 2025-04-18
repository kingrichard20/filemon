#pragma once

#include <cstddef>

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
  };

}
