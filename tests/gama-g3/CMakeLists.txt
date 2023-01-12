set(TEST_BASE_DIR ${PROJECT_SOURCE_DIR}/tests/gama-g3)
set(GAMA_LOCAL ${CMAKE_BINARY_DIR}/gama-g3)
#set(INPUT_DIR ${TEST_BASE_DIR}/input)
#set(INPUT_FILES )

#set(RESULT_DIR ${CMAKE_BINARY_DIR}/tests/gama-g3/results/${PROJECT_VERSION})
#file(MAKE_DIRECTORY ${RESULT_DIR})
#file(MAKE_DIRECTORY ${RESULT_DIR}/gama-g3)

# ------------------------------------------------------------------------
#
# check transformation XYZ --> BLH --> XYZ
#
add_executable(check_ellipsoid_xyz2blh src/check_ellipsoid_xyz2blh.cpp
  $<TARGET_OBJECTS:libgama> )

add_test(NAME gama-g3-ellipsoid-xyz2blh COMMAND check_ellipsoid_xyz2blh)
