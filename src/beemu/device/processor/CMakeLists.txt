target_sources(beemu PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/processor.c
	${CMAKE_CURRENT_SOURCE_DIR}/registers.c
	${CMAKE_CURRENT_SOURCE_DIR}/executor.c
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tokenizer)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interpreter)

