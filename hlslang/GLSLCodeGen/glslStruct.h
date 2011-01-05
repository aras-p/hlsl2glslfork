// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef GLSL_STRUCT_H
#define GLSL_STRUCT_H

#include "glslCommon.h"

class GlslStruct 
{
public:
   // Struct member description, presently does not handle structs of structs
   struct member    
   {
      std::string name;
      std::string semantic;
      EGlslSymbolType type;
      GlslStruct*     structType; // NULL if type != EgstStruct
      int arraySize;
	  TPrecision precision;
   };

   GlslStruct (const std::string &n) : name(n) {}
   virtual ~GlslStruct() {}

   const std::string& getName() const { return name; }

   void addMember( const member& m ) { memberList.push_back(m); }
   const member& getMember( int which ) const { return memberList[which]; }
   int memberCount() const { return int(memberList.size()); }

   std::string getDecl() const;

private:
   std::vector<member> memberList;
   std::string name;
};

#endif //GLSL_STRUCT_H
