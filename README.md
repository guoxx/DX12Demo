# ProtoLabNG

A DirectX 12 based renderer for prototype.

## How to setup the projects

Here are few steps to setup the project.

1. Set working directory as $(OutDir)

## Target features

* Physical based rendering
* Dynamic global illumination
* Tiled Deferred shading
* Offline path tracer integration

## Conventions

* Right hand coordinate system
* Column major matrix
* Matrix multiplication : post-multiply  (or right multiply), same rule applied on c++ and HLSL 
* Make all lightings in world space
