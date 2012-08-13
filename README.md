HLSL to GLSL shader language translator
========

DX9 style HLSL in, GLSL / GLSL ES out.

A continued development from [ATI's HLSL2GLSL](http://sourceforge.net/projects/hlsl2glsl). I'm changing it to make it work for [Unity's](http://unity3d.com) use cases; might totally not work for yours!

For an opposite tool (GLSL ES to HLSL translator), look at [Google's ANGLE](http://code.google.com/p/angleproject/).

Changes from original HLSL2GLSL 0.9
--------
* Removed all constant folding functionality as it was completely broken.
* Added support for emission of const initializers (including struct and array initializers using GLSL 1.20 array syntax).
* A myriad of various bug fixes.
* Support DX10 SV_VertexID, SV_PrimitiveID and SV_InstanceID semantics.
* Support for shadow sampler types (samplerRECTShadow/sampler2DShadow etc.) which generate appropriate shadow2DRect/shadow2D etc. calls.
* Fixed matrix swizzle access & assignments (view._m01_m02_m33 = value)
* Made it build with VS2008 on Windows and XCode 3.2 on Mac. Build as static library.
* Feature to produce OpenGL ES-like precision specifiers (fixed/half/float -> lowp/mediump/highp)
* Fixes to ternary vector selection (float4 ? float4 : float4)
* Fixes to bool->float promotion in arithmetic ops
* Fixes to matrix constructors & indexing (GLSL is transposed in regards to HLSL)
* Support clip()
* Support Cg-like samplerRECT, texRECT, texRECTproj
* Support VPOS and VFACE semantics
* Fix various crashes & infinite loops, mostly on shaders with errors
* Cleaner and more deterministic generated GLSL output
* Unit testing suite
* Simplified interface, code cleanup, unused code removal, merge copy-n-pasted code, simplify implementation etc.


Notes
--------

* Only Direct3D 9 style HLSL is supported. No Direct3D 10/11 "template like" syntax, no geometry/tesselation/compute shaders, no abstract interfaces.
* I bumped into some issues of HLSL2GLSL's preprocessor that I am not fixing. Most issues were with token pasting operator. So I preprocess source using [mojoshader's](http://icculus.org/mojoshader/) preprocessor. Grab latest from [mojoshader hg](http://hg.icculus.org/icculus/mojoshader/), it's awesome!
* On Windows, the library is built with `_HAS_ITERATOR_DEBUGGING=0,_SECURE_SCL=0` defines, which affect MSVC's STL behavior. If this does not match defines in your application, _totally strange_ things can start to happen!
* The library is not currently thread-safe.


Status
--------

The library is used in the bitsquid engine and seems to work quite well. Support for more DX10/11 features might be added in the future.

Seems to work quite well for Unity's shaders. When/if we bump into any issues; we'll just fix them.

No optimizations are performed on the generated GLSL, so it is expected that your platform will have a decent GLSL compiler. Or, use [GLSL Optimizer](http://github.com/aras-p/glsl-optimizer), we use it to optimize shaders produced by HLSL2GLSL; gives a substantial performance boost on mobile platforms.
