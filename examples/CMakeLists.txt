#Activer le support c++11 
if(CMAKE_VERSION VERSION_LESS "3.1")
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()

include_directories("../src")
include_directories("../dependency")
 
add_executable(example1 example1.cpp ../src/LightTSDB.cpp)
add_executable(example2 example2.cpp ../src/LightTSDB.cpp)
add_executable(example3 example3.cpp ../src/LightTSDB.cpp)

