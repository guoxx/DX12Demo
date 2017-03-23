# ProtoLabNG

A DX12renderer for prototype.

## How to setup the projects

ProtoLabNG.sln is for XBone, I don't maintain it anymore, will be delete later.   
ProtoLabNG_Win10.sln is working for now and will be maintained in future.  

Before you launch this project, here are few steps that you need to do.

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
