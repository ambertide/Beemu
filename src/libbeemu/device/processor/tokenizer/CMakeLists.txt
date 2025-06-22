target_sources( libbeemu PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_common.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_common.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_cbxx.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_cbxx.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_load.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_load.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_arithmatic.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_arithmatic.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_system.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_system.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_jump.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_jump.c
)