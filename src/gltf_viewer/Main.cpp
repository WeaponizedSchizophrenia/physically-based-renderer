#include <filesystem>
#include <iterator>
#include <print>
#include <span>

namespace {
using ArgvType = char const* const*;
}

auto main(int const argc, ArgvType const argv) -> int {
  std::span const args(std::next(argv), argc - 1);

  if (!args.empty()) {
    std::filesystem::path const gltfFilePath(args.front());
    std::println("GLTF file: {}", gltfFilePath.c_str());
  }
  return 0;
}
