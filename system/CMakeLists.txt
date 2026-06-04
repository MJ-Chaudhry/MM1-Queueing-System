add_library(system)

target_sources(system
    PRIVATE
        system.c

    PUBLIC
        FILE_SET HEADERS
        FILES
            system.h
)

target_link_libraries(system
    PRIVATE
        m
)