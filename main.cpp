#define APP_CURRENT_VERSION "14.1"
#define APP_VERSION_URL L"https://raw.githubusercontent.com/glaciercold101/Salvation-Prayer-Table/main/version.json"
#include <windows.h>
#include <shellapi.h>
#include <wininet.h>
#include <shlwapi.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <filesystem>
#include "native_bridge.h"

static std::wstring GetExePath(){ wchar_t p[MAX_PATH]={}; GetModuleFileNameW(nullptr,p,MAX_PATH); return std::wstring(p); }
static std::wstring GetExeFolder(){ std::wstring p=GetExePath(); size_t pos=p.find_last_of(L"\\/"); if(pos!=std::wstring::npos) return p.substr(0,pos); return L"."; }
static bool FileExistsW(const std::wstring& path){ DWORD a=GetFileAttributesW(path.c_str()); return a!=INVALID_FILE_ATTRIBUTES; }
static std::wstring UTF8ToWString(const std::string& s){ if(s.empty()) return {}; int sz=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0); if(sz<=0) return {}; std::wstring out(sz-1,L'\0'); MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,out.data(),sz); return out; }
static std::string HttpGet(const std::wstring& urlW){
    std::string r; HINTERNET hI=InternetOpenW(L"SalvationBeads/13.9",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,0); if(!hI) return r;
    DWORD to=4000; InternetSetOptionW(hI,INTERNET_OPTION_CONNECT_TIMEOUT,&to,sizeof(to)); InternetSetOptionW(hI,INTERNET_OPTION_RECEIVE_TIMEOUT,&to,sizeof(to));
    HINTERNET hU=InternetOpenUrlW(hI,urlW.c_str(),NULL,0,INTERNET_FLAG_RELOAD|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_NO_COOKIES,0);
    if(!hU){ InternetCloseHandle(hI); return r; }
    char b[8192]; DWORD br=0; while(InternetReadFile(hU,b,sizeof(b),&br)&&br>0) r.append(b,br);
    InternetCloseHandle(hU); InternetCloseHandle(hI); return r;
}
static std::string ExtractJsonValue(const std::string& json,const std::string& key){
    std::string pat="\""+key+"\""; size_t pos=json.find(pat); if(pos==std::string::npos) return {}; pos=json.find(":",pos+pat.size()); if(pos==std::string::npos) return {}; pos++; while(pos<json.size()&&isspace((unsigned char)json[pos])) pos++; if(pos>=json.size()) return {}; if(json[pos]=='"'){ pos++; size_t e=pos; while(e<json.size()){ if(json[e]=='"'&&json[e-1]!='\\') break; e++; } if(e>=json.size()) return {}; return json.substr(pos,e-pos); } else { size_t e=pos; while(e<json.size()&&json[e]!=','&&json[e]!='}'&&json[e]!='\n'&&json[e]!='\r') e++; std::string v=json.substr(pos,e-pos); size_t s=0; while(s<v.size()&&isspace((unsigned char)v[s])) s++; size_t ee=v.size(); while(ee>s&&isspace((unsigned char)v[ee-1])) ee--; return v.substr(s,ee-s); }
}
static std::vector<int> ParseVersion(const std::string& v){ std::string s=v; if(!s.empty()&&(s[0]=='v'||s[0]=='V')) s=s.substr(1); std::vector<int> parts; std::string cur; for(char c:s){ if(c=='.'||c=='-'||c=='_'){ if(!cur.empty()){ parts.push_back(atoi(cur.c_str())); cur.clear(); } } else if(isdigit((unsigned char)c)) cur+=c; else { if(!cur.empty()){ parts.push_back(atoi(cur.c_str())); cur.clear(); } } } if(!cur.empty()) parts.push_back(atoi(cur.c_str())); return parts; }
static bool IsNewerVersion(const std::string& cur,const std::string& lat){ if(lat.empty()||cur==lat) return false; auto a=ParseVersion(cur); auto b=ParseVersion(lat); size_t n=std::max(a.size(),b.size()); for(size_t i=0;i<n;i++){ int av=i<a.size()?a[i]:0; int bv=i<b.size()?b[i]:0; if(bv>av) return true; if(bv<av) return false; } return cur!=lat; }
static std::string LoadHtmlResource(){ HRSRC hr=FindResourceW(nullptr,MAKEINTRESOURCE(101),RT_RCDATA); if(!hr) return {}; HGLOBAL hg=LoadResource(nullptr,hr); if(!hg) return {}; DWORD sz=SizeofResource(nullptr,hr); const char* d=static_cast<const char*>(LockResource(hg)); if(!d||sz==0) return {}; return std::string(d,d+sz); }
static std::wstring FindPortableBrowser(){
    std::vector<std::wstring> cands; const std::wstring ef=GetExeFolder(); auto add=[&](const std::wstring& p){ cands.push_back(p); };
    add(ef+L"\\chrome.exe"); add(ef+L"\\msedge.exe"); add(ef+L"\\PortableChrome\\chrome.exe"); add(ef+L"\\PortableEdge\\msedge.exe"); add(ef+L"\\browser.exe");
    for(auto& c:cands) if(FileExistsW(c)) return c; cands.clear();
    add(L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"); add(L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe");
    add(L"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe"); add(L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe");
    for(auto& c:cands) if(FileExistsW(c)) return c; return L"";
}
static DWORD WINAPI VersionCheckThread(LPVOID){
    try{
        std::string json=HttpGet(APP_VERSION_URL);
        if(json.empty()) return 0; // no internet -> silent bypass
        std::string latest=ExtractJsonValue(json,"version");
        std::string website=ExtractJsonValue(json,"website");
        std::string dl=ExtractJsonValue(json,"download_url");
        std::string notes=ExtractJsonValue(json,"notes");
        std::string urlToOpen=!website.empty()?website:dl;
        if(!latest.empty()&&IsNewerVersion(APP_CURRENT_VERSION,latest)){
            std::string msg="Update available!\n\nCurrent: "+std::string(APP_CURRENT_VERSION)+"\nLatest: "+latest+"\n\n";
            if(!notes.empty()) msg+=notes+"\n\n";
            if(!urlToOpen.empty()) msg+="Download at:\n"+urlToOpen+"\n\n";
            msg+="Do you want to open the website now?";
            std::wstring wMsg=UTF8ToWString(msg);
            int res=MessageBoxW(nullptr,wMsg.c_str(),L"Salvation Beads - Update Available",MB_YESNO|MB_ICONINFORMATION|MB_TOPMOST);
            if(res==IDYES&&!urlToOpen.empty()){ std::wstring wUrl=UTF8ToWString(urlToOpen); ShellExecuteW(nullptr,L"open",wUrl.c_str(),nullptr,nullptr,SW_SHOWNORMAL); }
        }
    }catch(...){}
    return 0;
}

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int){
    std::wstring exeFolder=GetExeFolder();
    std::wstring externalHtml=exeFolder+L"\\IndexApp.html";
    // Background version check - silent bypass if no internet
    HANDLE hThread=CreateThread(nullptr,0,VersionCheckThread,nullptr,0,nullptr); if(hThread) CloseHandle(hThread);

    // Load HTML - prefers external for dev, else embedded resource 101
    std::string html;
    if(FileExistsW(externalHtml)){
        std::ifstream in(std::filesystem::path(externalHtml),std::ios::binary);
        html.assign(std::istreambuf_iterator<char>(in),{});
    }
    if(html.empty()) html=LoadHtmlResource();
    if(html.empty()){ MessageBoxW(nullptr,L"No HTML found. Place IndexApp.html next to exe or embed as resource 101.",L"Salvation Beads",MB_ICONERROR|MB_OK); return 1; }

    NativeBridge bridge;
    if(!bridge.Start(exeFolder,html)){
        MessageBoxW(nullptr,L"Could not start local app service.",L"Salvation Beads",MB_ICONERROR|MB_OK); return 1;
    }

    std::wstring browser=FindPortableBrowser();
    if(browser.empty()){ MessageBoxW(nullptr,L"No browser found. Place portable chrome.exe / msedge.exe next to exe, or install Chrome/Edge.",L"Salvation Beads",MB_ICONERROR|MB_OK); bridge.Stop(); return 1; }

    std::wstring url=L"http://127.0.0.1:"+std::to_wstring(bridge.Port())+L"/index.html?token="+UTF8ToWString(bridge.Token());
    // Use temp profile so app mode is isolated
    wchar_t tmpPath[MAX_PATH]={}; GetTempPathW(MAX_PATH,tmpPath);
    std::wstring profile=std::wstring(tmpPath)+L"SalvationBeads-"+UTF8ToWString(bridge.Token());
    CreateDirectoryW(profile.c_str(),nullptr);
    std::wstring params=L"--app=\""+url+L"\" --window-size=1280,800 --user-data-dir=\""+profile+L"\"";
    std::wstring cmdLine=L"\""+browser+L"\" "+params;
    std::vector<wchar_t> buf(cmdLine.begin(),cmdLine.end()); buf.push_back(0);
    STARTUPINFOW si{}; si.cb=sizeof(si); PROCESS_INFORMATION pi{}; BOOL ok=CreateProcessW(browser.c_str(),buf.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&si,&pi);
    if(!ok){ MessageBoxW(nullptr,L"Failed to launch browser.",L"Salvation Beads",MB_ICONERROR|MB_OK); bridge.Stop(); return 1; }
    WaitForSingleObject(pi.hProcess,INFINITE); CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    bridge.Stop();
    std::error_code ec; std::filesystem::remove_all(profile,ec);
    return 0;
}
