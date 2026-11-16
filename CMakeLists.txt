cmake_minimum_required(VERSION 3.16)
project(Computer-Graphics CXX)

# Padrão C++17 exigido pelo projeto
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Lista de arquivos-fonte: ponto de entrada e módulos práticos em src/
set(SOURCES
    main.cpp
    src/ui_widgets.cpp
    src/grid_canvas.cpp
    src/aula03_cores.cpp
    src/aula04_pixels.cpp
    src/aula05_retas_pixels.cpp
    src/aula06_bresenham_reta.cpp
    src/aula07_circ_pixels.cpp
    src/aula08_bresenham_circ.cpp
    src/aula12_projecoes.cpp
    src/aula15_clipping.cpp
    src/app.cpp
)

if (EMSCRIPTEN OR PLATFORM STREQUAL "Web")
    # Configuração para compilação WebAssembly (Emscripten + raylib Web)
    set(PLATFORM "Web" CACHE STRING "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/raylib/CMakeLists.txt")
        add_subdirectory(raylib)
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../maze/raylib/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../maze/raylib" raylib_build)
    else()
        include(FetchContent)
        FetchContent_Declare(
            raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
        )
        FetchContent_MakeAvailable(raylib)
    endif()

    # Gera index.html (+ index.js, index.wasm, index.data) prontos para GitHub Pages
    add_executable(index ${SOURCES})
    target_include_directories(index PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)
    target_link_libraries(index PRIVATE raylib)
    target_compile_definitions(index PRIVATE PLATFORM_WEB)
    target_compile_options(index PRIVATE -Wall -Wextra -Wpedantic)

    set_target_properties(index PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "-s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )

    # Copia os artefatos estáticos para docs/ (hospedagem direta via GitHub Pages)
    add_custom_command(TARGET index POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:index>/index.html"
            "$<TARGET_FILE_DIR:index>/index.js"
            "$<TARGET_FILE_DIR:index>/index.wasm"
            "$<TARGET_FILE_DIR:index>/index.data"
            "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        COMMENT "Copiando arquivos WebAssembly para docs/ (GitHub Pages)"
    )
else()
    # Configuração local (Linux / Desktop)
    find_package(raylib REQUIRED)

    add_executable(app ${SOURCES})
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)
    target_link_libraries(app PRIVATE raylib)
    target_compile_options(app PRIVATE -Wall -Wextra -Wpedantic)
endif()
