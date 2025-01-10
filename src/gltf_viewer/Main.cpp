#include "App.hpp"
#include "vkfw/vkfw.hpp"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <print>
#include <span>
#include <string_view>

namespace {
using ArgvType = char const* const*;
}

auto main(int const argc, ArgvType const argv) -> int {
  std::span const args(std::next(argv), argc - 1);

  if (!args.empty()) {
    vkfw::init({
        .platform = vkfw::Platform::eX11,
    });
    app::App(args.front(), std::ranges::find(args, std::string_view("-vulkan-validation"))
                               != args.end())
        .run();
    vkfw::terminate();
  }
  return 0;
}
