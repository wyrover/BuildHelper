// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "VSUtility.h"
#include "CAboutDlg.h"

class CMainDlg : public CDialogImpl<CMainDlg>
{
private:
    CVSUtility m_vsUtility;
public:
    enum { IDD = IDD_MAINDLG };
    
    BEGIN_MSG_MAP( CMainDlg )
    MESSAGE_HANDLER( WM_DROPFILES, OnDropFile )
    MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
    COMMAND_ID_HANDLER( IDC_BTN_BUILD, OnBuild )
    COMMAND_ID_HANDLER( ID_APP_ABOUT, OnAppAbout )
    COMMAND_ID_HANDLER( IDC_BTN_QUIT, OnCancel )
    COMMAND_ID_HANDLER( IDCANCEL, OnCancel )
    COMMAND_ID_HANDLER( IDC_BTN_REBUILD, OnRebuild )
    COMMAND_ID_HANDLER( IDC_BTN_CLEAN, OnClean )
    COMMAND_ID_HANDLER( IDC_FOLDER_BRO, OnFolderBrowser )
    END_MSG_MAP()
    
    // Handler prototypes (uncomment arguments if needed):
    //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
    
    LRESULT OnDropFile( UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
        UINT uDropFileCnt = 0;
        TCHAR sDropFilePath[MAX_PATH] = {0};
        HDROP hDrop = ( HDROP )wParam;
        uDropFileCnt = DragQueryFile( hDrop, 0xFFFFFFFF, NULL, 0 );
        if ( uDropFileCnt )
        {
            for ( UINT i = 0; i < uDropFileCnt; i++ )
            {
                DragQueryFile( hDrop, i, sDropFilePath, MAX_PATH );
                CEdit editProjectDir = GetDlgItem( IDC_EDIT_VS_FOLDER );
                editProjectDir.SetWindowText( sDropFilePath );
            }
        }
        DragFinish( hDrop );
        return 0;
    }
    
    LRESULT OnBuild( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        HandleRequest( COMMAND_BUILD );
        return 0;
    }
    
    LRESULT OnRebuild( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        HandleRequest( COMMAND_REBUILD );
        return 0;
    }
    
    LRESULT OnClean( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        HandleRequest( COMMAND_CLEAN );
        return 0;
    }
    void HandleRequest( COMMAND_TYPE cmdType )
    {
        CEdit editOutput = GetDlgItem( IDC_EDIT_OUTPUT );
        editOutput.SetWindowText( _T( "正在执行命令,请稍候..." ) );
        m_vsUtility.SetRedirectHwnd( editOutput.m_hWnd );
        
        CComboBox comPlatForm = GetDlgItem( IDC_COM_VS_PLATFORM );
        BUILD_PLATFORM buildPlatForm = ( BUILD_PLATFORM )comPlatForm.GetItemData( comPlatForm.GetCurSel() );
        m_vsUtility.SetBuildPlatForm( buildPlatForm );
        
        CComboBox comConfig = GetDlgItem( IDC_COM_CONFIG );
        BUILD_CONFIG buildConfig = ( BUILD_CONFIG )comConfig.GetItemData( comConfig.GetCurSel() );
        m_vsUtility.SetBuildConfig( buildConfig );
        
        CComboBox comboxVsVer = GetDlgItem( IDC_COM_VSVER );
        VSITEM* pVsItem = ( VSITEM* )comboxVsVer.GetItemData( comboxVsVer.GetCurSel() );
        m_vsUtility.SetVsVer( pVsItem->sVer );
        
        CEdit editProjectDir = GetDlgItem( IDC_EDIT_VS_FOLDER );
        CString sProDir;
        editProjectDir.GetWindowText( sProDir );
        if ( !m_vsUtility.SetProjectDir( sProDir ) )
        {
            editOutput.SetWindowText( _T( "工程路径错误" ) );
            return ;
        }
        
        m_vsUtility.SetCmdType( cmdType );
        m_vsUtility.HandleRequest();
    }
    
    LRESULT OnInitDialog( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/ )
    {
        // center the dialog on the screen
        CenterWindow();
        
        // set icons
        HICON hIcon = AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics( SM_CXICON ), ::GetSystemMetrics( SM_CYICON ) );
        SetIcon( hIcon, TRUE );
        HICON hIconSmall = AtlLoadIconImage( IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics( SM_CXSMICON ), ::GetSystemMetrics( SM_CYSMICON ) );
        SetIcon( hIconSmall, FALSE );
        
        Init();
        return TRUE;
    }
    
    LRESULT OnAppAbout( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        CAboutDlg aboutDlg;
        aboutDlg.DoModal();
        return 0;
    }
    
    LRESULT OnOK( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        // TODO: Add validation code
        EndDialog( wID );
        return 0;
    }
    
    LRESULT OnCancel( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        EndDialog( wID );
        return 0;
    }
    
    LRESULT OnFolderBrowser( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/ )
    {
        MessageBox( _T( "亲，直接拖动\"sln\"或者\"vcxproj\"文件到窗口,比鼠标点半天要省事的多呢" ) );
        return 0;
    }
    
    void Init()
    {
        CComboBox comboxVsVer = GetDlgItem( IDC_COM_VSVER );
        m_vsUtility.FindVSItem();
        for ( m_vsUtility.Begin(); !m_vsUtility.IsEnd(); m_vsUtility.Next() )
        {
            VSITEM* vsItem = m_vsUtility.GetCurItem();
            CString sVer = vsItem->sVer;
            CString sInstallDir = vsItem->sInstallDir;
            int index = comboxVsVer.AddString( sVer );
            comboxVsVer.SetItemData( index, ( DWORD_PTR )vsItem );
        }
        comboxVsVer.SetCurSel( 0 );
        BOOL bHandle = FALSE;
        
        CComboBox comPlatform = GetDlgItem( IDC_COM_VS_PLATFORM );
        comPlatform.InsertString( 0, _T( "x86" ) );
        comPlatform.SetItemData( 0, PLATFORM_X86 );
        comPlatform.InsertString( 1, _T( "x64" ) );
        comPlatform.SetItemData( 1, PLATFORM_X64 );
        comPlatform.SetCurSel( 0 );
        
        CComboBox comConfig = GetDlgItem( IDC_COM_CONFIG );
        comConfig.InsertString( 0, _T( "Debug" ) );
        comConfig.SetItemData( 0, CONFIG_DEBUG );
        comConfig.InsertString( 1, _T( "Release" ) );
        comConfig.SetItemData( 1, CONFIG_RELEASE );
        comConfig.SetCurSel( 0 );
    }
};
