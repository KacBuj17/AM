cmake_minimum_required (VERSION 3.5)

project (mniam_player)

add_executable(mniam_player main.c amcom.c
        objects.h
        objects.c
        logic.h
        logic.c
        utils.c
        utils.h)
target_link_libraries(mniam_player Ws2_32.lib)