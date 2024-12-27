add_executable(tests)

target_link_libraries(tests PRIVATE
  Catch2::Catch2WithMain

  Vulkan::Headers
  vkfw

  pbr_engine_core
  pbr_engine
)

target_sources(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Sanity_Tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PbrCore_Tests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PbrEngine_Tests.cpp
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)
