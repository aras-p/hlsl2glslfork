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
//   This file contains the Hlsl2GlslUIDlg implementation
//=================================================================================================================================

//=================================================================================================================================
//
//          Includes / defines / typedefs / static member variable initialization block
//
//=================================================================================================================================
#include "stdafx.h"
#include "Hlsl2GlslUI.h"
#include "Hlsl2GlslUIUserAttributesDlg.h"
#include <vector>


//=================================================================================================================================
//
//          Hlsl2GlslUIUserAttributesDlg dialog
//
//=================================================================================================================================


IMPLEMENT_DYNAMIC(Hlsl2GlslUIUserAttributesDlg, CDialog)


//=========================================================================================================
/// Constructor
//=========================================================================================================
Hlsl2GlslUIUserAttributesDlg::Hlsl2GlslUIUserAttributesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Hlsl2GlslUIUserAttributesDlg::IDD, pParent)
{   
}

//=========================================================================================================
/// Destructor
//=========================================================================================================
Hlsl2GlslUIUserAttributesDlg::~Hlsl2GlslUIUserAttributesDlg()
{
}

//=========================================================================================================
/// OnInitDialog
//=========================================================================================================
BOOL Hlsl2GlslUIUserAttributesDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   // Initialize to past state
   SetAttributesFromStoredState();
   
   return TRUE;
}

//=========================================================================================================
/// DoDataExchange
//=========================================================================================================
void Hlsl2GlslUIUserAttributesDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_ATTRIB_POSITION, m_attribPosition);
   DDX_Control(pDX, IDC_ATTRIB_NORMAL, m_attribNormal);
   DDX_Control(pDX, IDC_ATTRIB_COLOR, m_attribColor);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD0, m_attribTexCoord0);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD1, m_attribTexCoord1);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD2, m_attribTexCoord2);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD3, m_attribTexCoord3);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD4, m_attribTexCoord4);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD5, m_attribTexCoord5);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD6, m_attribTexCoord6);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD7, m_attribTexCoord7);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD8, m_attribTexCoord8);
   DDX_Control(pDX, IDC_ATTRIB_TEXCOORD9, m_attribTexCoord9);
   DDX_Control(pDX, IDC_ATTRIB_TANGENT, m_attribTangent);
   DDX_Control(pDX, IDC_ATTRIB_BINORMAL, m_attribBinormal);
   DDX_Control(pDX, IDC_ATTRIB_BLENDWEIGHTS, m_attribBlendWeights);
   DDX_Control(pDX, IDC_ATTRIB_BLENDINDICES, m_attribBlendIndices);
}


//=========================================================================================================
/// Message map
//=========================================================================================================
BEGIN_MESSAGE_MAP(Hlsl2GlslUIUserAttributesDlg, CDialog)
   ON_BN_CLICKED(IDC_USER_ATTRIBUTE_LOAD, &Hlsl2GlslUIUserAttributesDlg::OnBnClickedUserAttributeLoad)
   ON_EN_CHANGE(IDC_ATTRIB_POSITION, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribPosition)
   ON_EN_CHANGE(IDC_ATTRIB_NORMAL, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribNormal)
   ON_EN_CHANGE(IDC_ATTRIB_COLOR, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribColor)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD0, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord0)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD1, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord1)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD2, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord2)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD3, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord3)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD4, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord4)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD5, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord5)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD6, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord6)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD7, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord7)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD8, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord8)
   ON_EN_CHANGE(IDC_ATTRIB_TEXCOORD9, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord9)
   ON_EN_CHANGE(IDC_ATTRIB_TANGENT, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTangent)
   ON_EN_CHANGE(IDC_ATTRIB_BINORMAL, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBinormal)
   ON_EN_CHANGE(IDC_ATTRIB_BLENDWEIGHTS, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBlendweights)
   ON_EN_CHANGE(IDC_ATTRIB_BLENDINDICES, &Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBlendindices)
END_MESSAGE_MAP()


//=================================================================================================================================
//
//          Hlsl2GlslUIUserAttributesDlg message handlers
//
//=================================================================================================================================


//=========================================================================================================
/// OnBnClickedUserAttributeLoad - load user attribute .cfg file
//=========================================================================================================
void Hlsl2GlslUIUserAttributesDlg::OnBnClickedUserAttributeLoad()
{
   CFileDialog browseDlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT,_T("All Files (*.*)|*.*||"));
   INT_PTR iRet = browseDlg.DoModal();
   CString strFileName;

   strFileName = browseDlg.GetPathName();

   if(iRet == IDOK)
   {
      LoadAttribConfigFile ( strFileName );
   }   
}

//=========================================================================================================
/// Message handlers to store attribute names
//=========================================================================================================
void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribPosition()
{
   m_attribPosition.GetWindowTextA ( m_attributes[EAttrSemPosition] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribNormal()
{
   m_attribNormal.GetWindowTextA ( m_attributes[EAttrSemNormal] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribColor()
{
   m_attribColor.GetWindowTextA ( m_attributes[EAttrSemColor0] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord0()
{
   m_attribTexCoord0.GetWindowTextA ( m_attributes[EAttrSemTex0] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord1()
{
   m_attribTexCoord1.GetWindowTextA ( m_attributes[EAttrSemTex1] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord2()
{
   m_attribTexCoord2.GetWindowTextA ( m_attributes[EAttrSemTex2] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord3()
{
   m_attribTexCoord3.GetWindowTextA ( m_attributes[EAttrSemTex3] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord4()
{
   m_attribTexCoord4.GetWindowTextA ( m_attributes[EAttrSemTex4] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord5()
{
   m_attribTexCoord5.GetWindowTextA ( m_attributes[EAttrSemTex5] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord6()
{
   m_attribTexCoord6.GetWindowTextA ( m_attributes[EAttrSemTex6] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord7()
{
   m_attribTexCoord7.GetWindowTextA ( m_attributes[EAttrSemTex7] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord8()
{
   m_attribTexCoord8.GetWindowTextA ( m_attributes[EAttrSemTex8] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTexcoord9()
{
   m_attribTexCoord9.GetWindowTextA ( m_attributes[EAttrSemTex9] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribTangent()
{
   m_attribTangent.GetWindowTextA ( m_attributes[EAttrSemTangent] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBinormal()
{
   m_attribBinormal.GetWindowTextA ( m_attributes[EAttrSemBinormal] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBlendweights()
{
   m_attribBlendWeights.GetWindowTextA ( m_attributes[EAttrSemBlendWeight] );   
}

void Hlsl2GlslUIUserAttributesDlg::OnEnChangeAttribBlendindices()
{
   m_attribBlendIndices.GetWindowTextA ( m_attributes[EAttrSemBlendIndices] );   
}


//=================================================================================================================================
//
//          Public Methods
//
//=================================================================================================================================

//=========================================================================================================
/// Get the user attribute list and names
//=========================================================================================================
int Hlsl2GlslUIUserAttributesDlg::GetUserAttributes ( EAttribSemantic pSemanticEnums[], char *pSemanticNames[] )
{
   std::vector<EAttribSemantic> semanticEnumList;
   std::vector<char*> semanticNameList;

   for ( int i = 0; i < (int) EAttrSemCount; i++ )
   {
      if ( m_attributes[i].GetLength() > 0 )
      {
         char *pStr = new char [ m_attributes[i].GetLength() + 1 ];

         strcpy_s ( pStr, m_attributes[i].GetLength() + 1, m_attributes[i] );
            
         semanticEnumList.push_back ( (EAttribSemantic) i );
         semanticNameList.push_back ( pStr );         
      }
   }

   for ( size_t i = 0; i < semanticEnumList.size(); i++ )
   {
      pSemanticEnums[i] = semanticEnumList[i];
      pSemanticNames[i] = semanticNameList[i];
   }

   return (int) semanticEnumList.size();
}

//=================================================================================================================================
//
//          Protected Methods
//
//=================================================================================================================================


//=========================================================================================================
/// Set the edit windows based on the stored text
//=========================================================================================================
void Hlsl2GlslUIUserAttributesDlg::SetAttributesFromStoredState()
{
   m_attribPosition.SetWindowTextA ( m_attributes[EAttrSemPosition] );      
   m_attribNormal.SetWindowTextA ( m_attributes[EAttrSemNormal] );   
   m_attribColor.SetWindowTextA ( m_attributes[EAttrSemColor0] );   
   m_attribTexCoord0.SetWindowTextA ( m_attributes[EAttrSemTex0] );   
   m_attribTexCoord1.SetWindowTextA ( m_attributes[EAttrSemTex1] );   
   m_attribTexCoord2.SetWindowTextA ( m_attributes[EAttrSemTex2] );   
   m_attribTexCoord3.SetWindowTextA ( m_attributes[EAttrSemTex3] );   
   m_attribTexCoord4.SetWindowTextA ( m_attributes[EAttrSemTex4] );   
   m_attribTexCoord5.SetWindowTextA ( m_attributes[EAttrSemTex5] );   
   m_attribTexCoord6.SetWindowTextA ( m_attributes[EAttrSemTex6] );   
   m_attribTexCoord7.SetWindowTextA ( m_attributes[EAttrSemTex7] );   
   m_attribTexCoord8.SetWindowTextA ( m_attributes[EAttrSemTex8] );   
   m_attribTexCoord9.SetWindowTextA ( m_attributes[EAttrSemTex9] );   
   m_attribTangent.SetWindowTextA ( m_attributes[EAttrSemTangent] );   
   m_attribBinormal.SetWindowTextA ( m_attributes[EAttrSemBinormal] );   
   m_attribBlendWeights.SetWindowTextA ( m_attributes[EAttrSemBlendWeight] );   
   m_attribBlendIndices.SetWindowTextA ( m_attributes[EAttrSemBlendIndices] );   
}

//=========================================================================================================
///   Load a configuration file specifying which user attribute names to use
//=========================================================================================================
bool Hlsl2GlslUIUserAttributesDlg::LoadAttribConfigFile ( CString& rConfigFileName )
{
   static char *attribString[EAttrSemCount] =
   {
      "",
      "EAttrSemPosition",
      "EAttrSemNormal",
      "EAttrSemColor0",
      "EAttrSemColor1",
      "EAttrSemColor2",
      "EAttrSemColor3",
      "EAttrSemTex0",
      "EAttrSemTex1",
      "EAttrSemTex2",
      "EAttrSemTex3",
      "EAttrSemTex4",
      "EAttrSemTex5",
      "EAttrSemTex6",
      "EAttrSemTex7",
      "EAttrSemTex8",
      "EAttrSemTex9",
      "EAttrSemTangent",
      "EAttrSemBinormal",
      "EAttrSemBlendWeight",
      "EAttrSemBlendIndices",
      "EAttrSemDepth",
      "EAttrSemUnknown",   
   };
   char *data;
   int semanticCount = 0;

   FILE *fp;
   fopen_s ( &fp, rConfigFileName, "r" );
   if (!fp)
   {
      MessageBox ( "Unabled to open configuration file.", "ERROR" );
      return false;
   }

   int count = 0;
   while (fgetc(fp) != EOF)
      count++;

   fseek(fp, 0, SEEK_SET);

   data = new char [count + 2];
   if (!data)
      return false;
   
   fread(data, 1, count, fp);
   data[count] = 0;
   fclose (fp );

   char *nextToken;
   char *semToken;
   semToken = strtok_s ( data, " \n", &nextToken );
   while ( semToken != NULL )
   {      
      char *attrToken;
      attrToken = strtok_s ( NULL, " \n", &nextToken );

      if ( attrToken == NULL )
      {
         MessageBox ( "Encountered invalid token in file.", "ERROR" );
         delete [] data;
         return false;
      }

      bool bAttribSemFound = false;
      EAttribSemantic semantic;

      for ( int i = 0; i < EAttrSemCount; i++ )
      {
         if ( !strcmp ( semToken, attribString[i] ) )
         {
            bAttribSemFound = true;
            semantic = (EAttribSemantic) i;
            break;
         }
      }

      if ( !bAttribSemFound )
      {
         CString errMsg;
         errMsg.FormatMessage ( "Attribute semantic '%s' in file '%s' not found.", semToken, rConfigFileName );
         MessageBox (errMsg, "ERROR" );
      }
      else
      {
         if ( semantic < EAttrSemCount )
         {
            m_attributes[semantic] = attrToken;            
         }
         else
         {
            CString errMsg;
            errMsg.FormatMessage( "Ignoring attribute semantic '%s' in file '%s' because there are too many semantics\n", semToken, rConfigFileName );
            MessageBox (errMsg, "WARNING" );
         }
      }

      semToken = strtok_s ( NULL, " \n", &nextToken );
   }

   delete [] data;
   SetAttributesFromStoredState();
   return true;
}
