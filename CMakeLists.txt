cmake_minimum_required(VERSION 2.8.12)

project(hlsl2glsl)

# derived from msvs2010 project

set(CMAKE_PROGRAM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools ${CMAKE_CURRENT_SOURCE_DIR}/tools/bin)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
set(ENV{BISON_SIMPLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/bison.simple)
set(ENV{BISON_HAIRY} ${CMAKE_CURRENT_SOURCE_DIR}/tools/bison.simple)
execute_process(COMMAND ${BISON_EXECUTABLE} -d -t -v -o ${PROJECT_BINARY_DIR}/gen_hlslang_tab.cpp hlslang.y WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hlslang/MachineIndependent)
execute_process(COMMAND ${FLEX_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/hlslang/MachineIndependent/hlslang.l WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
file(WRITE ${PROJECT_BINARY_DIR}/hlslang_tab.h "#include <gen_hlslang_tab.hpp>")

set(HLSL2GLSL_HEADER
	hlslang/MachineIndependent/Initialize.h
	hlslang/MachineIndependent/localintermediate.h
	hlslang/MachineIndependent/ParseHelper.h
	hlslang/MachineIndependent/preprocessor/mojoshader.h
	hlslang/MachineIndependent/preprocessor/mojoshader_internal.h
	hlslang/MachineIndependent/RemoveTree.h
	${PROJECT_BINARY_DIR}/gen_hlslang_tab.hpp
	hlslang/MachineIndependent/preprocessor/sourceloc.h
	hlslang/Include/BaseTypes.h
	hlslang/Include/Common.h
	hlslang/Include/InfoSink.h
	hlslang/Include/InitializeGlobals.h
	hlslang/Include/InitializeParseContext.h
	hlslang/Include/intermediate.h
	hlslang/Include/PoolAlloc.h
	hlslang/MachineIndependent/SymbolTable.h
	hlslang/Include/Types.h
	include/hlsl2glsl.h
	hlslang/GLSLCodeGen/glslCommon.h
	hlslang/GLSLCodeGen/glslFunction.h
	hlslang/GLSLCodeGen/glslOutput.h
	hlslang/GLSLCodeGen/glslStruct.h
	hlslang/GLSLCodeGen/glslSymbol.h
	hlslang/GLSLCodeGen/hlslCrossCompiler.h
	hlslang/GLSLCodeGen/hlslLinker.h
	hlslang/GLSLCodeGen/hlslSupportLib.h
	hlslang/GLSLCodeGen/propagateMutable.h
	hlslang/GLSLCodeGen/typeSamplers.h
)

set(HLSL2GLSL_SRCS
	hlslang/MachineIndependent/ConstantFolding.cpp
	hlslang/MachineIndependent/HLSL2GLSL.cpp
	hlslang/MachineIndependent/InfoSink.cpp
	hlslang/MachineIndependent/Initialize.cpp
	hlslang/MachineIndependent/Intermediate.cpp
	hlslang/MachineIndependent/intermOut.cpp
	hlslang/MachineIndependent/IntermTraverse.cpp
	hlslang/MachineIndependent/ParseHelper.cpp
	hlslang/MachineIndependent/PoolAlloc.cpp
	hlslang/MachineIndependent/preprocessor/mojoshader.cpp
	hlslang/MachineIndependent/preprocessor/mojoshader_common.cpp
	hlslang/MachineIndependent/preprocessor/mojoshader_lexer.cpp
	hlslang/MachineIndependent/preprocessor/mojoshader_preprocessor.cpp
	hlslang/MachineIndependent/preprocessor/sourceloc.cpp
	hlslang/MachineIndependent/RemoveTree.cpp
	hlslang/MachineIndependent/SymbolTable.cpp
	${PROJECT_BINARY_DIR}/Gen_hlslang.cpp
	${PROJECT_BINARY_DIR}/gen_hlslang_tab.cpp
	hlslang/GLSLCodeGen/glslCommon.cpp
	hlslang/GLSLCodeGen/glslFunction.cpp
	hlslang/GLSLCodeGen/glslOutput.cpp
	hlslang/GLSLCodeGen/glslStruct.cpp
	hlslang/GLSLCodeGen/glslSymbol.cpp
	hlslang/GLSLCodeGen/hlslCrossCompiler.cpp
	hlslang/GLSLCodeGen/hlslLinker.cpp
	hlslang/GLSLCodeGen/hlslSupportLib.cpp
	hlslang/GLSLCodeGen/propagateMutable.cpp
	hlslang/GLSLCodeGen/typeSamplers.cpp
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/hlslang
  ${CMAKE_CURRENT_SOURCE_DIR}/hlslang/MachineIndependent
  ${PROJECT_BINARY_DIR}
)

# Add system specific settings
if (WIN32)
	set(HLSL2GLSL_HEADER
		${HLSL2GLSL_HEADER}
		hlslang/OSDependent/Windows/osinclude.h
		hlslang/OSDependent/Windows/unistd.h
	)
	set(HLSL2GLSL_SRCS
		${HLSL2GLSL_SRCS}
		hlslang/OSDependent/Windows/ossource.cpp
	)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hlslang/OSDependent/Windows)
	add_definitions(/D "_HAS_ITERATOR_DEBUGGING=0" /D "_SECURE_SCL=0" /D "_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_DEBUG_POSTFIX "_d")
elseif(APPLE)
	set(HLSL2GLSL_HEADER
		${HLSL2GLSL_HEADER}
		hlslang/OSDependent/Mac/osinclude.h
	)
	set(HLSL2GLSL_SRCS
		${HLSL2GLSL_SRCS}
		hlslang/OSDependent/Mac/ossource.cpp
	)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hlslang/OSDependent/Mac)
else()
	set(HLSL2GLSL_HEADER
		${HLSL2GLSL_HEADER}
		hlslang/OSDependent/Linux/osinclude.h
	)
	set(HLSL2GLSL_SRCS
		${HLSL2GLSL_SRCS}
		hlslang/OSDependent/Linux/ossource.cpp
	)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hlslang/OSDependent/Linux)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
endif()

add_library(hlsl2glsl ${HLSL2GLSL_HEADER} ${HLSL2GLSL_SRCS})

find_package(OpenGL REQUIRED)
add_executable(hlsl2glsltest tests/hlsl2glsltest/hlsl2glsltest.cpp)
target_link_libraries(hlsl2glsltest hlsl2glsl)
include_directories(hlsl2glsltest ${OPENGL_INCLUDE_DIRS})
target_link_libraries(hlsl2glsltest ${OPENGL_LIBRARIES})

if(MSVC)
	set_target_properties(hlsl2glsl PROPERTIES COMPILE_FLAGS /Fd"$(IntDir)$(TargetName).pdb")
	
	INSTALL(TARGETS hlsl2glsl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug CONFIGURATIONS Debug)
	INSTALL(FILES ${PROJECT_BINARY_DIR}/Debug/hlsl2glsl_d.pdb DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Debug CONFIGURATIONS Debug)
	INSTALL(TARGETS hlsl2glsl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/MinSizeRel CONFIGURATIONS MinSizeRel)
	INSTALL(TARGETS hlsl2glsl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/Release CONFIGURATIONS Release)
	INSTALL(TARGETS hlsl2glsl DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
	#INSTALL(FILES ${PROJECT_BINARY_DIR}/RelWithDebInfo/hlsl2glsl.pdb DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/RelWithDebInfo CONFIGURATIONS RelWithDebInfo)
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/include/hlsl2glsl.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif(MSVC)
