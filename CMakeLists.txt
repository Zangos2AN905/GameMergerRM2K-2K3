cmake_minimum_required(VERSION 3.20)
project(RM2KGameMergerBeta VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies from vcpkg (Global/Classic mode)
find_package(fmt REQUIRED)
find_package(EXPAT REQUIRED)

# Find inih
find_package(unofficial-inih CONFIG) 
if (NOT unofficial-inih_FOUND)
    find_package(inih CONFIG)
endif()

# Find ICU with all necessary components
# liblcf needs uc (common), i18n (in), and data (dt)
find_package(ICU REQUIRED COMPONENTS uc i18n data)

# Fetch liblcf
include(FetchContent)
FetchContent_Declare(
  liblcf
  GIT_REPOSITORY https://github.com/EasyRPG/liblcf.git
  GIT_TAG        master
)
set(LIBLCF_ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
set(LIBLCF_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(LIBLCF_INSTALL      OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS   OFF CACHE BOOL "" FORCE) # Force static library

# Match Vcpkg's static CRT (assuming x64-windows-static usage)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

FetchContent_MakeAvailable(liblcf)

include_directories(src)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

add_executable(RM2KGameMergerBeta ${SOURCES})

# Explicitly link ICU dependencies in correct dependency order (i18n depends on uc)
# Link ole32 for Windows COM (IFileOpenDialog)
target_link_libraries(RM2KGameMergerBeta PRIVATE lcf fmt::fmt EXPAT::EXPAT ICU::i18n ICU::uc ICU::data ole32)
