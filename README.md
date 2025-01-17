# PolyGL
My own graphics library built in C <br>

## Window API
#### Linux
Creates windows relying on wayland and X11

#### Windows
Creates windows relying on win32 api

#### OSX
Creates windows relying on cocoa

## Graphics API
#### OpenGL
Working on vulkan for now, this will be worked on later for backwards compatibility

#### Vulkan
Relies on the vulkan sdk


## Intermediate Graphics API
To work with multiple graphic library backends a universal intermediate shading language compiler should be built to compile shaders into a target format for polygl.
this can be done seperatly from polygl and doesn't have to be handled any differently, so for now this will be put on the backburner and be worked on once the library is ready for directX 11 and directX 12.