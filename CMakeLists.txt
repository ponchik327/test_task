CMAKE_MINIMUM_REQUIRED(VERSION 3.21)
PROJECT(ClienServerEcn)

FIND_PACKAGE(Boost 1.74 REQUIRED system)
FIND_PACKAGE(Threads REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

SET(CMAKE_CXX_STANDARD 17)

SET(SRC_SERVER_FILES 
    main_server.cpp
    Server.cpp Server.h 
    Core.cpp Core.h 
    Common.hpp
    json.hpp)

SET(SRC_CLIENT_FILES 
    main_client.cpp
    Client.cpp Client.h
    Common.hpp
    json.hpp)

ADD_EXECUTABLE(Server ${SRC_SERVER_FILES})
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads ${Boost_LIBRARIES})

ADD_EXECUTABLE(Client ${SRC_CLIENT_FILES})
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads ${Boost_LIBRARIES})