cmake_minimum_required(VERSION 4.0.0)

project(roblox C CXX ASM)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  enable_language(OBJC OBJCXX)
  if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(APPLE_IOS TRUE)
  endif()
endif()

add_compile_options(-w)
add_compile_options(-Wnonportable-include-path)
add_compile_options(-Wundefined-var-template)
add_compile_options(-Wdelete-abstract-non-virtual-dtor)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Boost COMPONENTS random format accumulators bimap filesystem system iostreams thread exception program_options pool circular_buffer math crc numeric_conversion flyweight numeric_ublas foreach REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# HACK
include_directories(${Boost_INCLUDE_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/ThirdParty/fmod/include")
if(ANDROID)
  link_directories("${CMAKE_SOURCE_DIR}/ThirdParty/fmod/lib/Android/${ANDROID_ABI}")
elseif(APPLE)
  link_directories("${CMAKE_SOURCE_DIR}/ThirdParty/fmod/lib/Mac")
else()
  link_directories("${CMAKE_SOURCE_DIR}/ThirdParty/fmod/lib/Win32")
endif()

add_subdirectory(Base)
add_subdirectory(Log)
add_subdirectory(Rendering)
add_subdirectory(Network)
add_subdirectory(App)
add_subdirectory(BulletPhysics)

set(DEPENDENCIES
   Boost::filesystem
   Boost::iostreams
   Boost::program_options
   Boost::system
   Boost::thread
   Boost::exception
   Boost::pool

   CURL::libcurl
   OpenSSL::Crypto
   OpenSSL::SSL
   ZLIB::ZLIB
   PNG::PNG
   JPEG::JPEG
   Freetype::Freetype

   fmod
   OpenGL::GL
   SDL2::SDL2
)

add_subdirectory(${CMAKE_SOURCE_DIR}/Client)