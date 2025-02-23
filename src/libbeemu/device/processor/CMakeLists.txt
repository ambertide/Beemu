target_sources( libbeemu PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/processor.c
	${CMAKE_CURRENT_SOURCE_DIR}/registers.c
	${CMAKE_CURRENT_SOURCE_DIR}/executor.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.c
)