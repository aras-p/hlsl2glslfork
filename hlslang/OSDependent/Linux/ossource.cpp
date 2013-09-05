// Copyright (c) The HLSL2GLSLFork Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.txt file.

//
// This file contains the Linux specific functions
//
#include "osinclude.h"

#if !(defined(linux))
#error Trying to build a Linux specific file in a non-Linux build.
#endif


//
// Thread cleanup
//

//
// Wrapper for Linux call to DetachThread.  This is required as pthread_cleanup_push() expects 
// the cleanup routine to return void.
// 
void DetachThreadLinux(void *)
{
	DetachThread();
}


//
// Registers cleanup handler, sets cancel type and state, and excecutes the thread specific
// cleanup handler.  This function will be called in the Standalone.cpp for regression 
// testing.  When OpenGL applications are run with the driver code, Linux OS does the 
// thread cleanup.
// 
void OS_CleanupThreadData(void)
{
	int old_cancel_state, old_cancel_type;
	void *cleanupArg = NULL;

	//
	// Set thread cancel state and push cleanup handler.
	//
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old_cancel_state);
	pthread_cleanup_push(DetachThreadLinux, (void *) cleanupArg);

	//
	// Put the thread in deferred cancellation mode.
	//
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &old_cancel_type);

	//
	// Pop cleanup handler and execute it prior to unregistering the cleanup handler.
	//
	pthread_cleanup_pop(1);

	//
	// Restore the thread's previous cancellation mode.
	//
	pthread_setcanceltype(old_cancel_state, NULL);
}


//
// Thread Local Storage Operations
//
OS_TLSIndex OS_AllocTLSIndex()
{
	pthread_key_t pPoolIndex;

	//
	// Create global pool key.
	//
	if ((pthread_key_create(&pPoolIndex, NULL)) != 0) {
		assert(0 && "OS_AllocTLSIndex(): Unable to allocate Thread Local Storage");
		return false;
	}
	else
		return pPoolIndex;
}


bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	if (pthread_setspecific(nIndex, lpvValue) == 0)
		return true;
	else
		return false;
}


bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	//
	// Delete the global pool key.
	//
	if (pthread_key_delete(nIndex) == 0)
		return true;
	else
		return false;
}
