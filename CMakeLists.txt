### Fork from the great template of Jules Fouchy here: https://github.com/JulesFouchy/Simple-Cpp-Setup
cmake_minimum_required(VERSION 3.8)

# You can set the name of your project here
project(VoronoiApp)
add_executable(${PROJECT_NAME})

# Choose your C++ version
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

## SDL
if (EXISTS "${CMAKE_SOURCE_DIR}/SDL2")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 bits
        set(SDL2_DLL "${PROJECT_SOURCE_DIR}/SDL2/x86_64-w64-mingw32/bin/SDL2.dll")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 bits
        set(SDL2_DLL "${PROJECT_SOURCE_DIR}/SDL2/i686-w64-mingw32/bin/SDL2.dll")
    endif()
    
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to Cpp_SDL_Program
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
            ${SDL2_DLL}      # <--this is in-file
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )    
    set(SDL2_DIR ${CMAKE_SOURCE_DIR}/SDL2/cmake)
endif()

find_package(SDL2 REQUIRED)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to Cpp_SDL_Program
    COMMAND ${CMAKE_COMMAND} -E copy_directory # which executes "cmake - E copy_if_different..."
        ${PROJECT_SOURCE_DIR}/resources      # <--this is in-file
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)    

# Enable many good warnings.

# You can also add /WX (MSVC) and -Werror that enable "warnings as errors": This means that your code won't compile if you have any warning.
# This forces you to take warnings into account, which is a good practice because warnings are here for a reason and can save you from a lot of bugs!
# By default, these options are disabled because it can be a bit too strict in some cases.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set the folder where the executable is created (here in a "bin" folder to separated from the souces)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# Prevents compiler-specific extensions to C++ because they might allow code to compile on your machine but not on other people's machine
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Get and add all the source files founded in the src folder
file(GLOB_RECURSE SOURCES_FILES CONFIGURE_DEPENDS src/*)
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})


# Tell Cmake where to look for header files (use the same src folder)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
