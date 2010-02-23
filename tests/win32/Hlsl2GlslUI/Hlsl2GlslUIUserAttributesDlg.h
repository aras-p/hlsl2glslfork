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
//   Header for Hlsl2GlslUIUserAttributesDlg
//=================================================================================================================================


#pragma once
#include "afxwin.h"
#include <HLSL2GLSL.h>

//=========================================================================================================
// Hlsl2GlslUIUserAttributesDlg dialog
//=========================================================================================================
class Hlsl2GlslUIUserAttributesDlg : public CDialog
{
	DECLARE_DYNAMIC(Hlsl2GlslUIUserAttributesDlg)

public:
	Hlsl2GlslUIUserAttributesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~Hlsl2GlslUIUserAttributesDlg();

   // Dialog Data
	enum { IDD = IDD_USERATTRIBUTES_DIALOG };

protected:
   virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnBnClickedUserAttributeLoad();

   // Get the user attribute list and names
   int GetUserAttributes ( EAttribSemantic pSemanticEnums[], char *pSemanticNames[] );
   
   afx_msg void OnEnChangeAttribPosition();
   afx_msg void OnEnChangeAttribNormal();
   afx_msg void OnEnChangeAttribColor();
   afx_msg void OnEnChangeAttribTexcoord0();
   afx_msg void OnEnChangeAttribTexcoord1();
   afx_msg void OnEnChangeAttribTexcoord2();
   afx_msg void OnEnChangeAttribTexcoord3();
   afx_msg void OnEnChangeAttribTexcoord4();
   afx_msg void OnEnChangeAttribTexcoord5();
   afx_msg void OnEnChangeAttribTexcoord6();
   afx_msg void OnEnChangeAttribTexcoord7();
   afx_msg void OnEnChangeAttribTexcoord8();
   afx_msg void OnEnChangeAttribTexcoord9();
   afx_msg void OnEnChangeAttribTangent();
   afx_msg void OnEnChangeAttribBinormal();
   afx_msg void OnEnChangeAttribBlendweights();
   afx_msg void OnEnChangeAttribBlendindices();
   
protected:
   // Set the edit windows based on the stored text
   void SetAttributesFromStoredState();

   //   Load a configuration file specifying which user attribute names to use
   bool LoadAttribConfigFile ( CString& rConfigFileName );
   
   // User attrib edit controls
   CEdit m_attribPosition;
   CEdit m_attribNormal;
   CEdit m_attribColor;
   CEdit m_attribTexCoord0;
   CEdit m_attribTexCoord1;
   CEdit m_attribTexCoord2;
   CEdit m_attribTexCoord3;
   CEdit m_attribTexCoord4;
   CEdit m_attribTexCoord5;
   CEdit m_attribTexCoord6;
   CEdit m_attribTexCoord7;
   CEdit m_attribTexCoord8;
   CEdit m_attribTexCoord9;
   CEdit m_attribTangent;
   CEdit m_attribBinormal;
   CEdit m_attribBlendWeights;
   CEdit m_attribBlendIndices;

   CString m_attributes[EAttrSemCount];

};
