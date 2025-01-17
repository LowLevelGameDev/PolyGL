cmake_minimum_required(VERSION 3.10)
project(libpolygl VERSION 1.0.0)

# Determine window src files
if(WIN32)
  set(WINDOW_SRC src/window/win32.c)
elseif(APPLE)
  set(WINDOW_SRC src/window/cocoa.c)
elseif(UNIX AND NOT APPLE)
  if(WAYLAND_FOUND)
    set(WINDOW_SRC src/window/wayland.c)
  else()
    set(WINDOW_SRC src/window/x11.c)
  endif()
endif()

# Find and determine graphic src files
if (VULKAN)
  find_package(Vulkan REQUIRED)
  list(APPEND GRAPHIC_SRC src/graphics/vulkan.c)
endif()
if(OPENGL)
  list(APPEND GRAPHIC_SRC src/graphics/opengl.c)
endif()

# Collect all source files
set(SRCS 
  src/poly.c
  ${WINDOW_SRC} 
  ${GRAPHIC_SRC}
)

# Add the library
add_library(polygl SHARED ${SRCS})
add_library(polygl_static STATIC ${SRCS})
if(VULKAN)
  target_link_libraries(polygl PRIVATE ${Vulkan_LIBRARIES})
endif()
if(OPENGL)
  message(FATAL_ERROR "opengl not implemented")
endif()

target_include_directories(polygl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(polygl_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Examples
set(EXAMPLE_SRCS examples/window.c)
foreach(example ${EXAMPLE_SRCS})
  get_filename_component(example_name ${example} NAME_WE)
  add_executable(${example_name} ${example})
  target_link_libraries(${example_name} PRIVATE polygl_static)
endforeach()

# Testing
# NotImplementedYet...