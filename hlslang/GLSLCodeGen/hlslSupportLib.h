// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef HLSL_SUPPORT_LIB_H
#define HLSL_SUPPORT_LIB_H

#include <string>
#include "../Include/intermediate.h"

void initializeHLSLSupportLibrary();

void finalizeHLSLSupportLibrary();

const std::string& getHLSLSupportCode( TOperator op);

#endif //HLSL_SUPPORT_LIB_H
