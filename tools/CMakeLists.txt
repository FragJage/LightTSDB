#Activer le support c++11 
if(CMAKE_VERSION VERSION_LESS "3.1")
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual") # various warning flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
 
include_directories(../dependency)
include_directories(../src)
 
#Génération de la liste des fichiers sources
file(
  GLOB_RECURSE
  LightTSDB_files
  ../src/*
)

#Déclaration de l'exécutable
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
add_executable(
  RebuildIndex
  ${LightTSDB_files}
  RebuildIndex.cpp
)

