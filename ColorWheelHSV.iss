; -- ColorWheelHSV.iss --
; Installer of ColorWheelHSV.exe for x86 and x64 architectures using a single installer.

; definitions
#define APPVER GetFileVersion("Release\Win32\ColorWheelHSV.exe")
#define OPENCVVER "246"
#define X86DIR "Release\Win32\"
#define X64DIR "Release\x64\"
#define X86VCREDISTDIR "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x86\Microsoft.VC100.CRT\"
#define X64VCREDISTDIR "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\"

[Setup]
AppName=ColorWheelHSV
AppVersion={#APPVER}
AppVerName=helo
OutputBaseFilename=ColorWheelHSV_setup_{#APPVER}
DefaultDirName={pf}\ColorWheelHSV
DefaultGroupName=ColorWheelHSV
UninstallDisplayIcon={app}\ColorWheelHSV.ico
Compression=lzma2
SolidCompression=yes
OutputDir=bin\
AppReadmeFile=usage.txt
SetupIconFile=res\ColorWheelHSV.ico

; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; On all other architectures it will install in "32-bit mode".
;ArchitecturesInstallIn64BitMode=x64
; Note: We don't set ProcessorsAllowed because we want this
; installation to run on all architectures (including Itanium,
; since it's capable of running 32-bit code too).

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon";
Name: quicklaunchicon; Description: "Create a &Quick Launch icon"; Flags: unchecked

[Files]
; x64 mode
;Source: "{#X64DIR}ColorWheelHSV.exe"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}ColorWheelHSV.chm"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}opencv_ffmpeg_64.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}opencv_core{#OPENCVVER}.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}opencv_highgui{#OPENCVVER}.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}opencv_imgproc{#OPENCVVER}.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64DIR}tbb.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64VCREDISTDIR}msvcp100.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
;Source: "{#X64VCREDISTDIR}msvcr100.dll"; DestDir: "{app}"; Check: Is64BitInstallMode
; Win32 mode
Source: "{#X86DIR}ColorWheelHSV.exe"; DestDir: "{app}"; Check: not Is64BitInstallMode
;Source: "{#X86DIR}ColorWheelHSV.chm"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86DIR}opencv_ffmpeg{#OPENCVVER}.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86DIR}opencv_core{#OPENCVVER}.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86DIR}opencv_highgui{#OPENCVVER}.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86DIR}opencv_imgproc{#OPENCVVER}.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86DIR}tbb.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
;Source: "{#X86DIR}lapack_win32_MT.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
;Source: "{#X86DIR}blas_win32_MT.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86VCREDISTDIR}msvcp100.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
Source: "{#X86VCREDISTDIR}msvcr100.dll"; DestDir: "{app}"; Check: not Is64BitInstallMode
; common files
Source: "usage.txt"; DestDir: "{app}"; Flags: isreadme;
Source: "res\ColorWheelHSV.ico"; DestDir: "{app}";


[Icons]
Name: "{group}\ColorWheelHSV"; Filename: "{app}\ColorWheelHSV.exe"
