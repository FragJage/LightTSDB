﻿#Activer le support c++11 
if(CMAKE_VERSION VERSION_LESS "3.1")
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Zi -Od") # debug, no optimisation
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual") # various warning flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
endif ()
 
include_directories(../dependency)
include_directories(../src)
 
#Génération de la liste des fichiers sources
file(
  GLOB_RECURSE
  LightTSDB_files
  ../src/*
)

file(
  GLOB_RECURSE
  UnitTest_files
  ../dependency/UnitTest/*
)

add_definitions( -DUSE_MOCKS )

#Déclaration de l'exécutable
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
add_executable(
  unittest
  ${LightTSDB_files}
  ${UnitTest_files}
  ../tools/RebuildIndex.cpp
  TimeMock.cpp
  TestLtsdbFile.cpp
  TestHourlyTimestamp.cpp
  TestLightTSDB.cpp
  TestOtherTypes.cpp
  TestTools.cpp
  main.cpp
)

