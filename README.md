# FoxEngine

FoxEngine is a small game engine created in C and C++, featuring lua as a scripting language.

This project is very early on and major changes **will** occur.

## The FoxEngine team
* [AnthoFoxo](https://anthofoxo.xyz) - Lead developer, Project director
* [Simon Bolivar](https://github.com/SNSTRUTHERS) - Code review, API design

## Dependencies
All dependencies are included in the source tree or as submodules (Make sure to clone with submodules! `--recurse-submodules`)
* glfw 3.3.8
* glm 0.9.9.8
* assimp v5.0.1
* entt v3.11.1
* spdlog v1.11.0
* OpenGL 3.3+ (optionally HKR_debug)

## Building

premake5 is used for creating project scripts.
https://premake.github.io/

* `premake5 vs2022` For visual studio
* `premake5 gmake2` For make files