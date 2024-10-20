HLSL to GLSL shader language translator
========

> :warning: As of mid-2016, the project will not get any significant developments. Unity has moved to a different
> shader compilation pipeline. So from my side there won't be significant work done on it.
> You might want to look into [glslang](https://github.com/KhronosGroup/glslang),
> [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) + [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross),
> [HLSLParser](https://github.com/Thekla/hlslparser) or
> [HLSLcc](https://github.com/strandborg/HLSLcc
> instead. :warning:

DX9 style HLSL in, GLSL / GLSL ES out.

A continued development from [ATI's HLSL2GLSL](https://sourceforge.net/projects/hlsl2glsl/), with preprocessor code based on
[mojoshader](https://icculus.org/mojoshader/). I'm changing it to make it work for [Unity's](https://unity.com/) use cases;
might totally not work for yours!

For an opposite tool (GLSL ES to HLSL translator), look at [Google's ANGLE](https://github.com/google/angle).

See badly maintained [change log](Changelog.md).


Notes
--------

* Only Direct3D 9 style HLSL is supported. No Direct3D 10/11 "template like" syntax, no geometry/tesselation/compute shaders, no abstract interfaces.
* Platform support:
	* Windows via Visual Studio 2022 (`hlslang.sln`).
	* Mac via Xcode 15 (`hlslang.xcodeproj`).
	* Other platforms may or might not work. Some people have contributed CMake build scripts, but I am not maintaining them.
* On Windows, the library is built with `_HAS_ITERATOR_DEBUGGING=0,_SECURE_SCL=0` defines, which affect MSVC's STL behavior. If this does not match defines in your application, _totally strange_ things can start to happen!
* The library is not currently thread-safe.


Status
--------

Used in Unity and bitsquid engines, and some other studios -- seems to work quite ok.

Support for DX11 features might or might not get added due to the bad condition the original code is in (very obscure and inefficient),
instead maybe a new cross-compiler will be made. Someday. Maybe.

No optimizations are performed on the generated GLSL, so it is expected that your platform will have a decent GLSL compiler.
Or, use [GLSL Optimizer](https://github.com/aras-p/glsl-optimizer), at Unity we use it to optimize shaders produced by HLSL2GLSL;
gives a substantial performance boost on mobile platforms.
