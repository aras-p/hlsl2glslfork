//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

#include "osinclude.h"

//
// string manipulation
//
static void StrToLower(char *str) 
{
   int i, length;
   
   length = strlen(str);
   
   for(i = 0;i < length;i++) 
      str[i] = tolower(str[i]);
}

int stricmp(const char *str1, const char *str2) 
{
   char *str1Copy, *str2Copy;
   int retVal;
   
   str1Copy = new char[strlen(str1) + 1];
   str2Copy = new char[strlen(str2) + 1];

   strcpy(str1Copy, str1);
   strcpy(str2Copy, str2);

   StrToLower(str1Copy);
   StrToLower(str2Copy);
   
   retVal = strcmp(str1Copy, str2Copy);
   
   delete str1Copy;
   delete str2Copy;
   
   return retVal;
}

//
// Thread Local Storage Operations
//
OS_TLSIndex OS_AllocTLSIndex()
{
    OS_TLSIndex key;
    if (pthread_key_create(&key, NULL) != 0)
    {
        assert(0 && "OS_AllocTLSIndex(): Unable to allocate Thread Local Storage");
        return 0;
    }
    
    return key;
}


bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
    if (pthread_setspecific(nIndex, lpvValue) != 0)
    {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Key");
		return false;
	}

	return true;
}


bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
    if (pthread_key_delete(nIndex) != 0)
    {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	return true;
}
