#Activer le support c++11 
if(CMAKE_VERSION VERSION_LESS "3.1")
  set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()

include_directories(../src)
include_directories(MongooseCpp)
include_directories(.)
 
#Génération de la liste des fichiers sources
file(
  GLOB_RECURSE
  LightTSDB_files
  ../src/*
)

file(
  GLOB_RECURSE
  Mongoose_files
  mongoose/*
)

add_definitions( -DMG_ENABLE_CALLBACK_USERDATA )

#Déclaration de l'exécutable
set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
add_executable(
  LTSDBServer
  ${LightTSDB_files}
  ${Mongoose_files}
  MongooseCpp/Request.cpp
  MongooseCpp/Response.cpp
  MongooseCpp/WebServer.cpp
  LastController.cpp
  LtsdbFactory.cpp
  ReadController.cpp
  TimeHelper.cpp
  WelcomeController.cpp
  WriteController.cpp
  main.cpp
)

#Spécification WIN32
if(WIN32)
  target_link_libraries(LTSDBServer ws2_32)
endif()
