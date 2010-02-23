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

//=================================================================================================================================
//   ATI Research, Inc.
//
//   This file contains the Hlsl2GlslUIDlg definition
//=================================================================================================================================

#pragma once
#include "afxwin.h"
#include "Hlsl2GlslUIUserAttributesDlg.h"
#include "SimpleDropTarget.h"

//=========================================================================================================
/// CHlsl2GlslUIDlg dialog
//=========================================================================================================

class CHlsl2GlslUIDlg : public CDialog, ISimpleDropTarget
{
// Construction
public:
	CHlsl2GlslUIDlg(CWnd* pParent = NULL);	// standard constructor

   // Dialog Data
	enum { IDD = IDD_HLSL2GLSLUI_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

   HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()   

public:
   afx_msg void OnBnClickedTranslate();
   afx_msg void OnBnClickedBrowse();
   afx_msg void OnBnClickedUserAttributesButton();
   afx_msg void OnBnClickedUserAttributes();
   afx_msg void OnBnClickedSave();
   afx_msg void OnOK();   
   afx_msg void OnDestroy();
   afx_msg void OnDropFiles(HDROP hDropInfo);	

protected:

   // Edit box for translated GLSL source
   CEdit m_glslSource;
   // HLSL input source edit box
   CEdit m_hlslSource;
   // Shader type combox box
   CComboBox m_shaderType;
   // Shader function edit box
   CEdit m_shaderFunction;
   // Dialog box for configuring user attributes
   Hlsl2GlslUIUserAttributesDlg m_userAttributesDlg;
   // Check box of whether to use user varyings
   CButton m_useUserVaryings;
   // Check box for use user attributes
   CButton m_useUserAttributes;
   // Button for user attributes dialog
   CButton m_userAttributesButton;
   // Check box for default precision qualifier
   CButton m_defaultPrecision;

   // ISimpleDropTarget - drop target handling
   BOOL SupportsDrop(COleDataObject* pDataObject);

   // Handle drag-and drop
   BOOL HandleDrop(IStream* pStream, const char* pszFileName);   

   // Drop target manager
   CSimpleDropTargetManager* m_pDropTargetManager;

   // Load HLSL source file
   void LoadFile ( const char* strFileName );

};
