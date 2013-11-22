HLSL to GLSL shader language translator
========

DX9 style HLSL in, GLSL / GLSL ES out.

A continued development from [ATI's HLSL2GLSL](http://sourceforge.net/projects/hlsl2glsl). I'm changing it to make it work for [Unity's](http://unity3d.com) use cases; might totally not work for yours!

For an opposite tool (GLSL ES to HLSL translator), look at [Google's ANGLE](http://code.google.com/p/angleproject/).

See badly maintained [change log](Changelog.md).


Notes
--------

* Only Direct3D 9 style HLSL is supported. No Direct3D 10/11 "template like" syntax, no geometry/tesselation/compute shaders, no abstract interfaces.
* I bumped into some issues of HLSL2GLSL's preprocessor that I am not fixing. Most issues were with token pasting operator. So I preprocess source using [mojoshader's](http://icculus.org/mojoshader/) preprocessor. Grab latest from [mojoshader hg](http://hg.icculus.org/icculus/mojoshader/), it's awesome!
* On Windows, the library is built with `_HAS_ITERATOR_DEBUGGING=0,_SECURE_SCL=0` defines, which affect MSVC's STL behavior. If this does not match defines in your application, _totally strange_ things can start to happen!
* The library is not currently thread-safe.


Status
--------

Used in Unity and bitsquid engine, and seems to work quite ok.

Support for DX11 features will probably not be added due to the bad condition the original code is in (very obscure and inefficient), instead maybe a new cross-compiler will be made.

No optimizations are performed on the generated GLSL, so it is expected that your platform will have a decent GLSL compiler. Or, use [GLSL Optimizer](http://github.com/aras-p/glsl-optimizer), at Unity we use it to optimize shaders produced by HLSL2GLSL; gives a substantial performance boost on mobile platforms.
