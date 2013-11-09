#pragma once
#include <map>

template <class item>
class Iterator
{
public:
    virtual void Begin() = 0;
    virtual void Next() = 0;
    virtual BOOL IsEnd() = 0;
    virtual item* GetCurItem() = 0;
};

struct VSITEM
{
    CString sVer;
    CString sInstallDir;
};

enum BUILD_PLATFORM
{
    PLATFORM_X86,
    PLATFORM_X64
};
enum BUILD_CONFIG
{
    CONFIG_DEBUG,
    CONFIG_RELEASE
};

class CVSUtility : public Iterator<VSITEM>
{
public:

    CVSUtility()
    {
    
    }
    
    virtual ~CVSUtility()
    {
        Clear();
    }
    
    void Begin()
    {
        if ( m_vsItemMap.size() > 0 )
        {
            m_cit = m_vsItemMap.begin();
        }
    }
    
    void Next()
    {
        if ( m_vsItemMap.size() > 0 )
        {
            m_cit++;
        }
    }
    
    BOOL IsEnd()
    {
        if ( m_vsItemMap.size() > 0 )
        {
            if ( m_cit != m_vsItemMap.end() )
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    
    VSITEM* GetCurItem()
    {
        return m_cit->second;
    }
    
    void FindVSItem()
    {
        Clear();
        CRegKey regKey;
        CString sKeyPath = _T( "SOFTWARE\\Microsoft\\VisualStudio\\" );
        if ( ERROR_SUCCESS == regKey.Open( HKEY_LOCAL_MACHINE, sKeyPath ) )
        {
            int iIndex = 0;
            TCHAR sVer[MAX_PATH] = {0};
            DWORD dwVerLen = MAX_PATH;
            while ( ERROR_NO_MORE_ITEMS != regKey.EnumKey( iIndex, sVer, &dwVerLen ) )
            {
                CString sFullPath = sKeyPath + sVer;
                sFullPath.Append( _T( "\\Setup\\VC" ) );
                CRegKey regKey1;
                if ( ERROR_SUCCESS == regKey1.Open( HKEY_LOCAL_MACHINE, sFullPath ) )
                {
                    TCHAR sInstallDir[MAX_PATH] = {0};
                    ULONG lInstallDirLen = MAX_PATH;
                    if ( ERROR_SUCCESS == regKey1.QueryStringValue( _T( "ProductDir" ), sInstallDir, &lInstallDirLen ) )
                    {
                        VSITEM* pVsitem = new VSITEM;
                        pVsitem->sVer = sVer;
                        pVsitem->sInstallDir = sInstallDir;
                        m_vsItemMap.insert( std::make_pair( sVer, pVsitem ) );
                    }
                    regKey1.Close();
                }
                ++iIndex;
                dwVerLen = MAX_PATH;	// 必须还原长度
            }
            regKey.Close();
        }
    }
    
    void Clear()
    {
        std::map<CString, VSITEM*>::const_iterator cit;
        for ( cit = m_vsItemMap.begin(); cit != m_vsItemMap.end(); cit++ )
        {
            VSITEM* pVsItem = cit->second;
            if ( NULL != pVsItem )
            {
                delete pVsItem;
                pVsItem = NULL;
            }
        }
        m_vsItemMap.clear();
    }
    
    void Build()
    {
        CString sBatPath = CreateBatFile();
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        
        ZeroMemory( &si, sizeof( si ) );
        si.cb = sizeof( si );
        ZeroMemory( &pi, sizeof( pi ) );
        
        // Start the child process.
        if ( !CreateProcess( NULL,  // No module name (use command line)
                             sBatPath.GetBuffer(),        // Command line
                             NULL,           // Process handle not inheritable
                             NULL,           // Thread handle not inheritable
                             FALSE,          // Set handle inheritance to FALSE
                             0,              // No creation flags
                             NULL,           // Use parent's environment block
                             NULL,           // Use parent's starting directory
                             &si,            // Pointer to STARTUPINFO structure
                             &pi )           // Pointer to PROCESS_INFORMATION structure
           )
        {
            return;
        }
        
        // Wait until child process exits.
        WaitForSingleObject( pi.hProcess, INFINITE );
        
        // Close process and thread handles.
        CloseHandle( pi.hProcess );
        CloseHandle( pi.hThread );
    }
    
    void SetBuildPlatForm( BUILD_PLATFORM buildPlatForm )
    {
        m_buildPlatForm = buildPlatForm;
    }
    
    void SetBuildConfig( BUILD_CONFIG buildConfig )
    {
        m_buildConfig = buildConfig;
    }
    
    void SetVsVer( CString sVer )
    {
        m_sVSVer = sVer;
    }
    
    void SetProjectDir( CString sProDir )
    {
        int pos = sProDir.ReverseFind( _T( '\\' ) );
        if ( -1 != pos )
        {
            m_sProjectName = sProDir.Mid( pos + 1 );
            m_sProjectDir = sProDir;
            m_sProjectDir.Truncate( pos + 1 );
        }
    }
    
protected:
    CString GetVsInstallPath( CString sVer )
    {
        if ( m_vsItemMap.size() > 0 )
        {
            std::map<CString, VSITEM*>::const_iterator cit = m_vsItemMap.find( sVer );
            if ( cit != m_vsItemMap.end() )
            {
                VSITEM* pVsItem = cit->second;
                return pVsItem->sInstallDir;
            }
        }
        return _T( "" );
    }
    
    CString CreateBatFile()
    {
        CString sVsBatPath = GetVsInstallPath( m_sVSVer ) + _T( "vcvarsall.bat" );
        CString sBatContent;
        sBatContent.Format( _T( "%s" ), _T( "CALL" ) );
        sBatContent.AppendFormat( _T( " \"%s\"\r\n" ), sVsBatPath );
        sBatContent.AppendFormat( _T( "%s\r\n" ), _T( "CD /d %~dp0" ) );
        sBatContent.AppendFormat( _T( "MSBUILD \"%s\"" ), m_sProjectName );
        if ( m_buildConfig == CONFIG_DEBUG )
        {
            sBatContent.AppendFormat( _T( " %s" ), _T( "/p:Configuration=Debug" ) );
        }
        else if ( CONFIG_RELEASE == m_buildConfig )
        {
            sBatContent.AppendFormat( _T( " %s" ), _T( "/p:Configuration=Release" ) );
        }
        sBatContent.Append( _T( " /m" ) );
        
        USES_CONVERSION;
        CString sBatFileName;
        sBatFileName.Format( _T( "%s%s" ), m_sProjectDir, _T( "BUILD.bat" ) );
        FILE* pFile = NULL;
        _tfopen_s( &pFile, sBatFileName, _T( "wb" ) );
        fwrite( W2A( sBatContent ), sBatContent.GetLength(), 1, pFile );
        fflush( pFile );
        fclose( pFile );
        return sBatFileName;
    }
private:
    std::map<CString, VSITEM*> m_vsItemMap;
    std::map<CString, VSITEM*>::const_iterator m_cit;
    
    BUILD_PLATFORM m_buildPlatForm;
    BUILD_CONFIG m_buildConfig;
    CString m_sVSVer;
    CString m_sProjectDir;
    CString m_sProjectName;
};