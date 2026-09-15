cmake_minimum_required(VERSION 3.16)
project(QtCalculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml)

add_subdirectory(external_lib)

qt_add_executable(QtCalculator
    src/main.cpp
    src/calc_backend.hpp
    src/calc_backend.cpp
    src/calc_engine.hpp
    src/calc_types.hpp
    src/internal_engine.hpp
    src/external_engine.hpp
    src/external_engine.cpp
    src/calc_worker.hpp
    src/calc_worker.cpp
    src/thread_queue.hpp
)

qt_add_qml_module(QtCalculator
    URI QtCalculatorApp
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(QtCalculator PRIVATE Qt6::Quick Qt6::Qml)

if (WIN32)
    set(EXTERNAL_LIB_NAME "doit.dll")
elseif (APPLE)
    set(EXTERNAL_LIB_NAME "libdoit.dylib")
else()
    set(EXTERNAL_LIB_NAME "libdoit.so")
endif()

add_custom_command(TARGET QtCalculator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:doit>
            $<TARGET_FILE_DIR:QtCalculator>/${EXTERNAL_LIB_NAME}
    COMMENT "Copying external calculator library next to the executable"
)

add_custom_command(TARGET QtCalculator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo
            "External library copied to $<TARGET_FILE_DIR:QtCalculator>/${EXTERNAL_LIB_NAME}"
)
