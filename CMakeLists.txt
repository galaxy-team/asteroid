if(DEFINED IS_ROOT)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    # since this is really a demo, we don't want to register it if we're not the root project
    add_executable(regurgitate
        regurgitate.cpp asteroid.cpp
    )
endif(DEFINED IS_ROOT)
