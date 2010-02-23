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
#include "Hlsl2GlslUIDlg.h"
#include "HLSLTranslator.h"
#include <HLSL2GLSL.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//=================================================================================================================================
//
//          CAboutDlg
//
//=================================================================================================================================

//=========================================================================================================
// CAboutDlg dialog used for App About
//=========================================================================================================
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

   // Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

   // Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

//=========================================================================================================
/// Constructor
//=========================================================================================================
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

//=========================================================================================================
/// DoDataExchange
//=========================================================================================================
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

//=========================================================================================================
/// Message map for CAboutDlg
//=========================================================================================================
BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


//=================================================================================================================================
//
//          CHlsl2GlslUIDlg dialog
//
//=================================================================================================================================


//=========================================================================================================
/// Constructor
//=========================================================================================================
CHlsl2GlslUIDlg::CHlsl2GlslUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHlsl2GlslUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

//=========================================================================================================
/// DoDataExchange
//=========================================================================================================
void CHlsl2GlslUIDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_GLSLSOURCE, m_glslSource);
   DDX_Control(pDX, IDC_HLSLSOURCE, m_hlslSource);
   DDX_Control(pDX, IDC_SHADERTYPE, m_shaderType);
   DDX_Control(pDX, IDC_FUNCTION, m_shaderFunction);   
   DDX_Control(pDX, IDC_USER_VARYINGS, m_useUserVaryings);
   DDX_Control(pDX, IDC_USER_ATTRIBUTES, m_useUserAttributes);
   DDX_Control(pDX, IDC_USER_ATTRIBUTES_BUTTON, m_userAttributesButton);
   DDX_Control(pDX, IDC_DEFAULT_PRECISION, m_defaultPrecision);
}

//=========================================================================================================
/// Message map for CHlsl2GlslUIDlg
//=========================================================================================================
BEGIN_MESSAGE_MAP(CHlsl2GlslUIDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_WM_DROPFILES()
   ON_WM_DESTROY()
   ON_BN_CLICKED(IDOK, &CHlsl2GlslUIDlg::OnOK)
   ON_BN_CLICKED(IDC_TRANSLATE, &CHlsl2GlslUIDlg::OnBnClickedTranslate)
   ON_BN_CLICKED(IDC_BROWSE, &CHlsl2GlslUIDlg::OnBnClickedBrowse)
   ON_BN_CLICKED(IDC_USER_ATTRIBUTES_BUTTON, &CHlsl2GlslUIDlg::OnBnClickedUserAttributesButton)
   ON_BN_CLICKED(IDC_USER_ATTRIBUTES, &CHlsl2GlslUIDlg::OnBnClickedUserAttributes)
   ON_BN_CLICKED(IDC_SAVE, &CHlsl2GlslUIDlg::OnBnClickedSave)
END_MESSAGE_MAP()


//=================================================================================================================================
//
//          CHlsl2GlslUIDlg message handlers
//
//=================================================================================================================================

//=========================================================================================================
/// OnInitDialog
//=========================================================================================================
BOOL CHlsl2GlslUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   m_userAttributesButton.EnableWindow ( FALSE );

   // Initialize controls
   m_shaderType.SetCurSel ( 0 );

   // Drop target initialization
   DragAcceptFiles(TRUE);
   m_pDropTargetManager = new CSimpleDropTargetManager(this, this, true);
	ASSERT(m_pDropTargetManager);

   return TRUE;  // return TRUE  unless you set the focus to a control
}

//=========================================================================================================
/// OnDestroy
//=========================================================================================================
void CHlsl2GlslUIDlg::OnDestroy()
{
   if ( m_pDropTargetManager )
      delete m_pDropTargetManager;
}


//=========================================================================================================
/// OnSysCommand
//=========================================================================================================
void CHlsl2GlslUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

//=========================================================================================================
/// OnPaint
/// If you add a minimize button to your dialog, you will need the code below
///  to draw the icon.  For MFC applications using the document/view model,
///  this is automatically done for you by the framework.
//=========================================================================================================
void CHlsl2GlslUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//=========================================================================================================
/// OnQueryDragIcon - The system calls this function to obtain the cursor to display while the user drags
///  the minimized window.
//=========================================================================================================
HCURSOR CHlsl2GlslUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//=========================================================================================================
/// OnOnBnClickedTranslate - translate the HLSL shader to GLSL.  Main routine that handles doing the
///   translation
//=========================================================================================================
void CHlsl2GlslUIDlg::OnBnClickedTranslate()
{
   CString shaderFunction;
   CString shaderType;
   CString hlslSource;
   CString glslSource;
   CString errorString;

   // Check that main function is specified
   if ( m_shaderFunction.GetWindowTextLengthA() == 0 )
   {
      MessageBox ("Must specify HLSL main function name.", "ERROR" );
      return;
   }
   
   // Check that HLSL source is specified
   if ( m_hlslSource.GetWindowTextLengthA() == 0 )
   {
      MessageBox ("No HLSL source code specified.", "ERROR" );
      return;
   }

   // Get the main function name
   m_shaderFunction.GetWindowTextA ( shaderFunction );

   // Get the HLSL source from the HLSL edit box
   m_hlslSource.GetWindowTextA ( hlslSource );
   
   // Get the shader type
   m_shaderType.GetWindowTextA ( shaderType );

   // Determine whether to use user varyings from check box
   bool bUseUserVaryings = ( m_useUserVaryings.GetCheck() == 0 ) ? false : true;
   
   // Determine whether to use default precision qualifier
   bool bUseDefaultPrecision = ( m_defaultPrecision.GetCheck() == 0 ) ? false : true;

   // Determine whether to use user attributes
   bool bUseUserAttributes = ( m_useUserAttributes.GetCheck() == 0 ) ? false : true;

   EAttribSemantic attribSemantics[EAttrSemCount];
   char *pAttribNames[EAttrSemCount];
   int nNumSemantics = 0;

   // If user attributes are selected, get them from the dialog box   
   if ( bUseUserAttributes )
   {
      nNumSemantics = m_userAttributesDlg.GetUserAttributes ( attribSemantics,
                                                              pAttribNames );
   }

   // Translate the file   
   HLSLTranslator translator;
   bool bResult = translator.Translate ( hlslSource, shaderFunction, shaderType, glslSource, errorString,
                                         bUseUserVaryings, bUseDefaultPrecision, nNumSemantics,
                                         attribSemantics, (const char**) &pAttribNames[0] );

   if ( nNumSemantics > 0 )
   {
      for ( int i = 0; i < nNumSemantics; i++ )
      {
         delete [] pAttribNames[i];         
      }
   }

   // Translation failed
   if ( bResult == false )
   {
      // Display the error string
      errorString.Replace ( "\n", "\r\n" );
      m_glslSource.SetWindowTextA ( "ERROR(s):\r\n" + errorString );
   }
   else
   {  
      // Show the translated source
      glslSource.Replace ( "\n", "\r\n" );
      m_glslSource.SetWindowTextA ( glslSource );   
   }
}

//=========================================================================================================
/// OnBnClickedBrowse - Browse for HLSL source file
//=========================================================================================================
void CHlsl2GlslUIDlg::OnBnClickedBrowse()
{
   CFileDialog browseDlg(TRUE,NULL,NULL,OFN_OVERWRITEPROMPT,
                        _T("All Files (*.*)|*.*|HLSL Files (*.hlsl)|*.hlsl|Text Files (*.txt)|*.txt||"));
   INT_PTR iRet = browseDlg.DoModal();
   CString strFileName;

   strFileName = browseDlg.GetPathName();

   if(iRet == IDOK)
   {
      LoadFile ( strFileName );      
   }   
}

//=========================================================================================================
/// OnBnClickedUserAttributesButton - start the user attributes dialog box
//=========================================================================================================
void CHlsl2GlslUIDlg::OnBnClickedUserAttributesButton()
{
   m_userAttributesDlg.DoModal();   
}

//=========================================================================================================
/// OnBnClickedUserAttributes - enable/disable use of user attributes
//=========================================================================================================
void CHlsl2GlslUIDlg::OnBnClickedUserAttributes()
{
   // Enable/disable the user attributes button based on the user attributes check mark
   if ( m_useUserAttributes.GetCheck () == 0 )
   {
      m_userAttributesButton.EnableWindow ( FALSE );
   }
   else
   {
      m_userAttributesButton.EnableWindow ( TRUE );
   }   
}

//=========================================================================================================
/// OnBnClickedSave - Save GLSL output to a file
//=========================================================================================================
void CHlsl2GlslUIDlg::OnBnClickedSave()
{
   // Check that GLSL source has been generated
   if ( m_glslSource.GetWindowTextLengthA() == 0 )
   {
      MessageBox ("No GLSL source code to save.", "ERROR" );
      return;
   }

   // Create a save dialog box
   CFileDialog saveDlg(FALSE,NULL,NULL,OFN_OVERWRITEPROMPT,_T("All Files (*.*)|*.*||"));
   INT_PTR iRet = saveDlg.DoModal();
   CString strFileName;

   strFileName = saveDlg.GetPathName();
   
   if(iRet == IDOK)
   {
      CString str;
      FILE *fp;
      
      // Get source text
      m_glslSource.GetWindowTextA ( str );
      
      // open output file
      fopen_s ( &fp,  strFileName, "wb" );

      if ( fp == NULL )
      {
         MessageBox ("Open output file for writing.", "ERROR" );
         return;
      }

      fwrite ( str, 1, m_glslSource.GetWindowTextLengthA() + 1, fp );
      fclose ( fp );      
   }   

}

//=========================================================================================================
/// OnDropFiles - handle drag-and-drop of files for loading
//=========================================================================================================
void CHlsl2GlslUIDlg::OnDropFiles(HDROP hDropInfo)
{
	char szFile[MAX_PATH];
	if(DragQueryFile(hDropInfo, 0, szFile, MAX_PATH))
   {
		LoadFile(szFile);
   }

	CDialog::OnDropFiles(hDropInfo);
}

//=========================================================================================================
/// Handle OnOK so that the app doesn't exit when the user presses "ENTER"
//=========================================================================================================
void CHlsl2GlslUIDlg::OnOK()
{
}

//=========================================================================================================
/// SupportsDrop - wherether a OLE object is supported for dropping
//=========================================================================================================
BOOL CHlsl2GlslUIDlg::SupportsDrop(COleDataObject* pDataObject)
{
   return pDataObject->IsDataAvailable(CF_TEXT) || pDataObject->IsDataAvailable(g_nFormatFileDescriptor);
}

//=========================================================================================================
/// HandleDrop - handle the dropping of a stream to the dialog
//=========================================================================================================
BOOL CHlsl2GlslUIDlg::HandleDrop(IStream* pStream, const char* /*pszFileName*/)
{
   CString strShader;
   if(ReadStream(pStream, strShader))
   {
      m_hlslSource.SetWindowTextA ( strShader );
      return TRUE;
   }

   return FALSE;
}




//=================================================================================================================================
//
//          Protected Members
//
//=================================================================================================================================

//=========================================================================================================
/// Load HLSL source file for reading
//=========================================================================================================
void CHlsl2GlslUIDlg::LoadFile ( const char *strFileName )
{
   FILE *fp;

   fopen_s ( &fp,  strFileName, "rb" );

   if ( fp == NULL )
   {
      MessageBox ("Reading source code file.", "ERROR" );
      return;
   }

   fseek ( fp, 0, SEEK_END );
   unsigned int fLen = ftell ( fp );
   fseek ( fp, 0, SEEK_SET );

   if ( fLen > 0 )
   {
   
      char *pBuf = new char [ fLen + 1 ];

      fread ( pBuf, 1, fLen, fp );
      pBuf[fLen] = 0;

      m_hlslSource.SetWindowTextA ( pBuf );
      m_glslSource.SetWindowTextA ( NULL );

      delete[] pBuf;
   }
}
