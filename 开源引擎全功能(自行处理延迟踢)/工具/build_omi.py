# 编译 Omi Release x64 v3 —— 用 32 位 MSBuild (amd64 版在本环境 segfault)
#   坑位记录:
#   ① WorkBuddy shell 环境块超 64KB → MSB4175, 必须干净 env
#   ② FileTracker 需要全套路径变量 → 缺了 MSB4018
#   ③ amd64 MSBuild.exe 在 WorkBuddy 沙箱下任何参数都 segfault
#      (0xC0000005, 连 -version 都崩) → 换 x86 版 MSBuild 正常
#   ④ capture_output 大输出也可能崩 → 输出重定向到文件
import subprocess, sys, os, re

MSBUILD = r"C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
PROJ = r"C:\Users\Administrator\Desktop\主\Omi.vcxproj"

sdk = r"C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A"
env = {
    "SystemRoot": r"C:\Windows", "windir": r"C:\Windows", "SystemDrive": "C:",
    "TEMP": r"C:\Windows\Temp", "TMP": r"C:\Windows\Temp",
    "PATH": r"C:\Windows\System32;C:\Windows",
    "ProgramData": r"C:\ProgramData", "ProgramFiles": r"C:\Program Files",
    "ProgramFiles(x86)": r"C:\Program Files (x86)",
    "ProgramW6432": r"C:\Program Files",
    "CommonProgramFiles": r"C:\Program Files\Common Files",
    "CommonProgramFiles(x86)": r"C:\Program Files (x86)\Common Files",
    "ComSpec": r"C:\Windows\System32\cmd.exe",
    "LOCALAPPDATA": r"C:\Windows\Temp", "APPDATA": r"C:\Windows\Temp",
    "USERPROFILE": r"C:\Windows\Temp", "HOMEDRIVE": "C:", "HOMEPATH": r"\Windows\Temp",
    "USERDOMAIN": "WORKGROUP", "USERNAME": "Administrator",
    "ALLUSERSPROFILE": r"C:\ProgramData",
    "FrameworkSDKRoot": sdk + r"\bin\NETFX 4.8 Tools",
    "FrameworkSDKRoot(x86)": sdk + r"\bin\NETFX 4.8 Tools",
    "FrameworkDir64": r"C:\Windows\Microsoft.NET\Framework64",
    "FrameworkVersion64": "v4.0.30319",
    "FrameworkDir": r"C:\Windows\Microsoft.NET\Framework",
    "FrameworkVersion": "v4.0.30319",
    "Framework40Version": "v4.0",
    "DevEnvDir": "C:\\Program Files (x86)\\Microsoft Visual Studio\\18\\BuildTools\\Common7\\IDE\\",
    "MSBUILDDISABLENODEREUSE": "1",
    "DisableFastUpToDateCheck": "true",
}

_logf = open(r"C:\Users\Administrator\Desktop\主\工具\build_out.txt", "w", encoding="utf-8", errors="replace")
r = subprocess.run(
    [MSBUILD, PROJ, "-p:Configuration=Release", "-p:Platform=x64",
     "-v:m", "-nologo", "-t:Rebuild", "-p:TrackFileAccess=false"],
    text=True, cwd=os.path.dirname(PROJ), env=env,
    stdout=_logf, stderr=subprocess.STDOUT,
)
_logf.close()

r2 = open(r"C:\Users\Administrator\Desktop\主\工具\build_out.txt", encoding="utf-8", errors="replace").read()
keep = [l for l in r2.splitlines() if re.search(
    r"error|-> |裁剪|清零|正在创建库|warning C4\d+|warning LNK", l)]
print("\n".join(keep[-30:]))
print("RC =", r.returncode)
sys.exit(r.returncode)
