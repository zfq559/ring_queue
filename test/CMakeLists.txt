cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

add_executable(ring_queue_test
  ring_queue_test.cc
)

target_link_libraries(ring_queue_test
  gtest
  pthread
  dl
)
