target_sources(beemu PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/memory.c
   ${CMAKE_CURRENT_SOURCE_DIR}/device.c
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/processor)
