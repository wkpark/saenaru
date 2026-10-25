# Unit tests for the hangul automata and the dictionary lookup.
#
# Windows (MSVC):
#   cmake -S tests -B build-tests -A x64
#   cmake --build build-tests --config Release
#   ctest --test-dir build-tests -C Release --output-on-failure
#
# Linux (clang-cl and xwin, run with wine): see clang-cl-xwin.cmake
#   cmake -S tests -B build-tests -G Ninja -DCMAKE_BUILD_TYPE=Release \
#         -DCMAKE_TOOLCHAIN_FILE=tests/clang-cl-xwin.cmake
#   cmake --build build-tests
#   ctest --test-dir build-tests --output-on-failure

cmake_minimum_required(VERSION 3.20)
project(saenaru_tests C)

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_executable(saenaru_tests
    test_main.c
    test_globals.c
    ${SRC}/hangul.c
    ${SRC}/engtypo.c
    ${SRC}/regconf.c
    ${SRC}/reg.c
    ${SRC}/strsubs.c
    ${SRC}/dicfile.c
    ${SRC}/hanjaidx.c
    ${SRC}/immsec.c)
target_include_directories(saenaru_tests PRIVATE ${SRC})
target_compile_definitions(saenaru_tests PRIVATE WIN32 _WINDOWS UNICODE _UNICODE)
target_compile_options(saenaru_tests PRIVATE /utf-8)
target_link_libraries(saenaru_tests PRIVATE advapi32 user32)

# generate resource/engwords.dic from a word list. see gen_engwords.c
add_executable(gen_engwords gen_engwords.c)
target_include_directories(gen_engwords PRIVATE ${SRC})
target_compile_definitions(gen_engwords PRIVATE WIN32 _WINDOWS UNICODE _UNICODE)

enable_testing()
# the dictionaries are read from the resource directory
add_test(NAME saenaru_tests COMMAND saenaru_tests
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../resource)
