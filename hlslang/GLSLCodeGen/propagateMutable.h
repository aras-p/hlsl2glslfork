// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#ifndef PROPAGATE_MUTABLE_H
#define PROPAGATE_MUTABLE_H

class TIntermNode;
class TInfoSink;

// Iterates over the intermediate tree and propagates mutable uniform qualifiers as necessary
// to the symbols.
void PropagateMutableUniforms (TIntermNode* root, TInfoSink &info);


#endif //PROPAGATE_MUTABLE_H
