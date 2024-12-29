#include "pbr/memory/Allocation.hpp"

#include "pbr/Vulkan.hpp"

#include "vk_mem_alloc.h"

#include <source_location>
#include <utility>

pbr::Allocation::Mapping::Mapping(Allocation const& allocation)
    : _allocation(&allocation), _memory(_allocation->_info.pMappedData) {
  if (_memory == nullptr) {
    vk::Result const result {
        vmaMapMemory(_allocation->_allocator, _allocation->_allocation, &_memory)};
    if(result != vk::Result::eSuccess) {
      vk::detail::throwResultException(result,
                                       std::source_location::current().function_name());
    }
  }
}

pbr::Allocation::Mapping::~Mapping() noexcept {
  // If _memory is not persistently mapped then unmap it.
  if (_memory != _allocation->_info.pMappedData) {
    vmaUnmapMemory(_allocation->_allocator, _allocation->_allocation);
  }
}

pbr::Allocation::Allocation(VmaAllocator allocator, VmaAllocation allocation,
                            VmaAllocationInfo info) noexcept
    : _allocator(allocator), _allocation(allocation), _info(info) {}

pbr::Allocation::Allocation(Allocation&& other) noexcept
    : _allocator(other._allocator)
    , _allocation(std::exchange(other._allocation, nullptr))
    , _info(other._info) {}

auto pbr::Allocation::operator=(Allocation&& rhs) noexcept -> Allocation& {
  _allocator = rhs._allocator;
  std::swap(_allocation, rhs._allocation);
  _info = rhs._info;
  return *this;
}

pbr::Allocation::~Allocation() noexcept { vmaFreeMemory(_allocator, _allocation); }

auto pbr::Allocation::map() const -> Mapping { return Mapping(*this); }
