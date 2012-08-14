HLSL to GLSL shader language translator
========

This is a fork of [Aras Pranckevičius's](http://aras-p.info) fork of [ATI's HLSL2GLSL](http://sourceforge.net/projects/hlsl2glsl) for use in the [bitsquid](http://www.bitsquid.se) engine.

Changes from [Unity's hlsl2glsl](https://github.com/aras-p/hlsl2glslfork)
--------
* Added support for emission of const initializers (including struct and array initializers using GLSL 1.20 array syntax).
* Removed all constant folding functionality as it was completely broken.
* A myriad of smaller bug fixes.
* Support DX10 SV_VertexID, SV_PrimitiveID and SV_InstanceID semantics.
* Support for shadow sampler types (samplerRECTShadow/sampler2DShadow etc.) which generate appropriate shadow2DRect/shadow2D etc. calls.
* Fixed unaligned swizzled matrix access & assignments (view._m01_m02_m33 = value)


Status
--------
The library is used in the bitsquid engine and seems to work well for our current purposes. Support for DX11 feautres will probably not be added due to the bad condition the original code is in (very obscure and inefficient), instead we will publish our own cross compiler when it's ready.
