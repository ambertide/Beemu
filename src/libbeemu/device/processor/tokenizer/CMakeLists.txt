target_sources( libbeemu PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/tokenizer.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_common.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_common.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_cbxx.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_cbxx.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_load8.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_load8.c
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_arithmatic8.h
	${CMAKE_CURRENT_SOURCE_DIR}/tokenize_arithmatic8.c
)