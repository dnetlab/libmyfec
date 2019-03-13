add_definitions(--std=gnu99)
set(SOURCE_FILES myfec.c mycommon.c my_debug.c rs.c fec.c sfxhash.c sfhashfcn.c sfmemcap.c sfprimetable.c)

add_library(myfec SHARED ${SOURCE_FILES})

install(
	FILES
		${CMAKE_CURRENT_SOURCE_DIR}/myfec.h
		${CMAKE_CURRENT_SOURCE_DIR}/my_debug.h
		${CMAKE_CURRENT_SOURCE_DIR}/sfxhash.h
		${CMAKE_CURRENT_SOURCE_DIR}/sfhashfcn.h
		${CMAKE_CURRENT_SOURCE_DIR}/sfmemcap.h
		${CMAKE_CURRENT_SOURCE_DIR}/sfprimetable.h
	DESTINATION
		myfec/include
)

install(
	TARGETS myfec LIBRARY
	DESTINATION lib
)
