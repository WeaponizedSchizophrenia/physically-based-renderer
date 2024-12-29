#pragma once

#include "vk_mem_alloc.h"

namespace pbr {
class Allocation {
  VmaAllocator _allocator;
  VmaAllocation _allocation;
  VmaAllocationInfo _info;

public:
  class Mapping {
    Allocation const* _allocation;
    void* _memory;

  public:
    explicit Mapping(Allocation const& allocation);
    Mapping(const Mapping&) = delete;
    Mapping(Mapping&&) = delete;
    auto operator=(const Mapping&) -> Mapping& = delete;
    auto operator=(Mapping&&) -> Mapping& = delete;
    ~Mapping() noexcept;

    [[nodiscard]]
    constexpr auto get() const noexcept -> void*;
  };
  Allocation(VmaAllocator allocator, VmaAllocation allocation,
             VmaAllocationInfo info) noexcept;

  Allocation(const Allocation&) = delete;
  auto operator=(const Allocation&) -> Allocation& = delete;
  Allocation(Allocation&&) noexcept;
  auto operator=(Allocation&&) noexcept -> Allocation&;

  ~Allocation() noexcept;

  [[nodiscard]]
  auto map() const -> Mapping;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::Allocation::Mapping::get() const noexcept -> void* { return _memory; }
