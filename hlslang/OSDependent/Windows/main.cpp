// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.


#include "InitializeDll.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:

            if (!InitProcess())
                return FALSE;
            break;
		case DLL_THREAD_ATTACH:

            if (!InitThread())
                return FALSE;
            break;

		case DLL_THREAD_DETACH:

			if (!DetachThread())
				return FALSE;
			break;

		case DLL_PROCESS_DETACH:

			DetachProcess();
			break;

		default:
			assert(0 && "DllMain(): Reason for calling DLL Main is unknown");
			return FALSE;
	}

	return TRUE;
}
