# set(IS_ARM_ARCHITECTURE TRUE)
# execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
# if (ARCHITECTURE MATCHES "^(arm)")
  # set(IS_ARM_ARCHITECTURE TRUE)
# endif()

# if (NOT ANDROID AND NOT IS_ARM_ARCHITECTURE)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
# elseif (IS_ARM_ARCHITECTURE)
  # message(STATUS "Setting ARM compilation flags.")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -march=armv7-a")
# endif()

add_definitions(-std=c++11 -march=native)
get_filename_component(_currentDir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${_currentDir}/OptimizeForArchitecture.cmake")
include("${_currentDir}/FindARM.cmake")
# OptimizeForArchitecture()

set(ENABLE_TIMING FALSE CACHE BOOL "Set to TRUE to enable timing")
message(STATUS "Timers enabled? ${ENABLE_TIMING}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_TIMING=${ENABLE_TIMING}")

set(ENABLE_STATISTICS FALSE CACHE BOOL "Set to TRUE to enable statistics")
message(STATUS "Statistics enabled? ${ENABLE_STATISTICS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_STATISTICS=${ENABLE_STATISTICS}")
