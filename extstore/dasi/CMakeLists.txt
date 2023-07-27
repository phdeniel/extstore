
SET(extstore_dasi_LIB_SRCS
   extstore_dasi.c
)

add_library(extstore_dasi SHARED ${extstore_dasi_LIB_SRCS})

target_link_libraries(extstore_dasi ini_config dasi)

add_executable(test_dasi test_dasi.c)
target_link_libraries(test_dasi extstore_dasi)

install(TARGETS extstore_dasi DESTINATION lib)
