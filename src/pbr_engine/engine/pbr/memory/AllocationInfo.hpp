#pragma once

#include <cstdint>

namespace pbr {
enum struct AllocationPriority : std::uint8_t { Time, Memory };
enum struct AllocationPreference : std::uint8_t { Device, Host };
struct AllocationInfo {
  AllocationPreference preference = AllocationPreference::Device;
  AllocationPriority priority = AllocationPriority::Time;
  bool ableToBeMapped = false;
  bool persistentlyMapped = false;
  bool randomAccess = false;
};
} // namespace pbr
