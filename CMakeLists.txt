if(DEFINED IS_ROOT)
    # since this is really a demo, we don't want to register it if we're not the root project
    add_executable(regurgitate
        regurgitate.cpp
    )
endif(DEFINED IS_ROOT)
