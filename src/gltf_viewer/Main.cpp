#include "App.hpp"
#include "vkfw/vkfw.hpp"

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
    vkfw::init();
    app::App(args.front()).run();
  }
  return 0;
}
