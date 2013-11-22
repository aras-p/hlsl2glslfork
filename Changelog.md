hlsl2glsl Change Log
=========================

2013 11
-------

Fixes:

* Fixes to some global variable initializers.

2013 09
-------

Fixes:

* Avoid producing variable names with double underscores.
* "const static", "static const" qualifiers work now; as well as "static" and "const" on function return values.
* Fixed PSIZE semantic translation.


Earlier stuff
-------------

* Made it build with VS2010 on Windows and XCode 3.2 on Mac. Build as static library.
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
* Added support for emission of const initializers (including struct and array initializers using GLSL 1.20 array syntax).
* Removed all constant folding functionality as it was completely broken.
* A myriad of smaller bug fixes.
* Support DX10 SV_VertexID, SV_PrimitiveID and SV_InstanceID semantics.
* Support for shadow sampler types (samplerRECTShadow/sampler2DShadow etc.) which generate appropriate shadow2DRect/shadow2D etc. calls.
* Fixed unaligned swizzled matrix access & assignments (view._m01_m02_m33 = value)
