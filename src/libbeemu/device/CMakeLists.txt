target_sources(libbeemu PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/processor.c
   ${CMAKE_CURRENT_SOURCE_DIR}/processor/registers.c
   ${CMAKE_CURRENT_SOURCE_DIR}/processor/executor.c
   ${CMAKE_CURRENT_SOURCE_DIR}/memory.c
   ${CMAKE_CURRENT_SOURCE_DIR}/device.c
)