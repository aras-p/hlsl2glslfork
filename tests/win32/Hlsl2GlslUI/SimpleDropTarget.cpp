//
//Copyright (C) 2005-2006  ATI Research, Inc.
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
//    Neither the name of ATI Research, Inc. nor the names of its
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

#include "StdAfx.h"
#include "SimpleDropTarget.h"

CSimpleDropTarget::CSimpleDropTarget(CWnd* pWnd, ISimpleDropTarget* pDropTarget)
{
	ASSERT(pDropTarget);
	m_pDropTarget = pDropTarget;

	ASSERT(pWnd);
	if(pWnd && pWnd->m_hWnd)
	{
		m_hWnd = pWnd->m_hWnd;
		RevokeDragDrop(pWnd->m_hWnd);
		Register(pWnd);
	}
	else
		m_hWnd = NULL;
}

CSimpleDropTarget::~CSimpleDropTarget(void)
{
	Revoke();
}

DROPEFFECT CSimpleDropTarget::OnDragOver(CWnd* /*pWnd*/, COleDataObject* pDataObject, DWORD /*dwKeyState*/, CPoint /*point*/)
{
	if(m_pDropTarget->SupportsDrop(pDataObject))
		return DROPEFFECT_COPY; // data fits
	else
		return DROPEFFECT_NONE; // data won’t fit
}

// Important: these strings need to be non-Unicode (don't compile UNICODE)
CLIPFORMAT g_nFormatFileDescriptor = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT g_nFormatFileContents = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_FILECONTENTS);

BOOL CSimpleDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point )
{
	ASSERT(pDataObject);
	if(!pDataObject)
		return FALSE;

	// Handle dropped file(s)
	if(pDataObject->IsDataAvailable(g_nFormatFileDescriptor) && pDataObject->IsDataAvailable(g_nFormatFileContents))
		return HandleDroppedFiles(pDataObject);
	else if(pDataObject->IsDataAvailable(CF_TEXT))
		return HandleDroppedText(pDataObject);
	else
		return COleDropTarget::OnDrop(pWnd, pDataObject, dropEffect, point);
}

BOOL CSimpleDropTarget::HandleDroppedFiles(COleDataObject* pDataObject)
{
	BOOL bRetVal = FALSE;

	//Set up format structure for the descriptor and contents
	FORMATETC formatDescriptor = {g_nFormatFileDescriptor, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	FORMATETC formatContents = {g_nFormatFileContents, NULL, DVASPECT_LINK, -1, TYMED_ISTREAM};

	if(pDataObject->IsDataAvailable(g_nFormatFileDescriptor, &formatDescriptor))
	{ 
		if(pDataObject->IsDataAvailable(g_nFormatFileContents, &formatContents))
		{ 
			// Get the descriptor information
			STGMEDIUM storageDescriptor = {0,0,0};

			HRESULT hr = pDataObject->GetData(g_nFormatFileDescriptor, &storageDescriptor, &formatDescriptor);
			FILEGROUPDESCRIPTOR* fileGroupDescriptor = (FILEGROUPDESCRIPTOR *) GlobalLock(storageDescriptor.hGlobal);

			// We're only interested in the first file
			for(UINT i = 0; i < fileGroupDescriptor->cItems; i++)
			{
				formatContents.lindex = i;
				STGMEDIUM storageContents = {0,0,0};
				hr = pDataObject->GetData(g_nFormatFileContents, &storageContents, &formatContents);
				//Ignore hr	
				if(storageContents.pstm)
					bRetVal |= m_pDropTarget->HandleDrop(storageContents.pstm, fileGroupDescriptor->fgd[i].cFileName);
			}
			GlobalUnlock(storageDescriptor.hGlobal);
			GlobalFree(storageDescriptor.hGlobal);
		}
	}

	return bRetVal;
}

BOOL CSimpleDropTarget::HandleDroppedText(COleDataObject* pDataObject)
{
	BOOL bRetVal = FALSE;

	//Set up format structure for the descriptor and contents
	FORMATETC formatText = {CF_TEXT, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM};

	if(pDataObject->IsDataAvailable(CF_TEXT, &formatText))
	{ 
		// Get the descriptor information
		STGMEDIUM storage = {0,0,0};

		HRESULT hr = pDataObject->GetData(CF_TEXT, &storage, &formatText);
		if(SUCCEEDED(hr))
			bRetVal = m_pDropTarget->HandleDrop(storage.pstm, NULL);
	}

	return bRetVal;
}

CSimpleDropTargetManager::CSimpleDropTargetManager(ISimpleDropTarget* pDropTarget, CWnd* pWnd, bool bRegisterChildren)
{
	Register(pDropTarget, pWnd, bRegisterChildren);
}

CSimpleDropTargetManager::~CSimpleDropTargetManager()
{
	std::list<CSimpleDropTarget*>::iterator ppDropTarget = m_DropTargets.begin();
	while(ppDropTarget != m_DropTargets.end())
		delete *ppDropTarget++;
}

BOOL CSimpleDropTargetManager::Register(ISimpleDropTarget* pDropTarget, CWnd* pWnd, bool bRegisterChildren)
{
	ASSERT(pDropTarget);
	m_pDropTarget = pDropTarget;

	ASSERT(pWnd);
	if(pWnd && pWnd->m_hWnd)
	{
		m_DropTargets.push_back(new CSimpleDropTarget(pWnd, pDropTarget));
		if(bRegisterChildren)
		{
			CWnd* pChild = pWnd->GetWindow(GW_CHILD);
			while(pChild)
			{
				Register(pDropTarget, pChild, true);
				pChild = pChild->GetNextWindow();
			}
		}
		return TRUE;
	}
	return FALSE;
}

#define ISTREAM_BUFFER_SIZE 1024
bool ReadStream(IStream* pStream, CString& strString)
{
	ASSERT(pStream);
	if(!pStream)
		return false;

	char szReadBuffer[ISTREAM_BUFFER_SIZE+1];

	ULONG ulBytesRead;
	pStream->Read(szReadBuffer, ISTREAM_BUFFER_SIZE, &ulBytesRead);

	while(ulBytesRead)
	{
		szReadBuffer[ulBytesRead] = '\0';
		strString += szReadBuffer;

		pStream->Read(szReadBuffer, ISTREAM_BUFFER_SIZE, &ulBytesRead);
	}

	return !strString.IsEmpty();
}

bool WriteStreamToFile(IStream* pStream, FILE* pFile)
{
	ASSERT(pStream);
	if(!pStream)
		return false;

	ASSERT(pFile);
	if(!pFile)
		return false;

	char szReadBuffer[ISTREAM_BUFFER_SIZE+1];

	ULONG ulBytesRead, ulBytesWritten = 0;
	pStream->Read(szReadBuffer, ISTREAM_BUFFER_SIZE, &ulBytesRead);
	while(ulBytesRead)
	{
		ulBytesWritten = (ULONG) fwrite(szReadBuffer, 1, ulBytesRead, pFile);

		pStream->Read(szReadBuffer, ISTREAM_BUFFER_SIZE, &ulBytesRead);
	}

	return(ulBytesWritten == ulBytesRead);
}