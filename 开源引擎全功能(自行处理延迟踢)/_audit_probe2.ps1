$ErrorActionPreference = 'Stop'
$src = @'
using System;
using System.Runtime.InteropServices;
[StructLayout(LayoutKind.Sequential)]
public struct CLIENT_ID { public IntPtr UniqueProcess; public IntPtr UniqueThread; }
[StructLayout(LayoutKind.Sequential)]
public struct MEMORY_BASIC_INFORMATION {
    public IntPtr BaseAddress; public IntPtr AllocationBase; public uint AllocationProtect;
    public ushort PartitionId; public IntPtr RegionSize; public uint State; public uint Protect; public uint Type; }
[StructLayout(LayoutKind.Sequential)]
public struct UNICODE_STRING { public ushort Length; public ushort MaximumLength; public IntPtr Buffer; }
public static class N {
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr GetModuleHandleA(string n);
    [DllImport("kernel32.dll", CharSet=CharSet.Ansi, SetLastError=true)] public static extern IntPtr GetProcAddress(IntPtr h, string n);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr VirtualAlloc(IntPtr a, UIntPtr s, uint t, uint p);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern bool VirtualFree(IntPtr a, UIntPtr s, uint t);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr OpenProcess(uint a, bool i, int pid);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr GetCurrentProcess();
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int qvm_t(IntPtr h, IntPtr a, uint c, IntPtr b, UIntPtr l, out UIntPtr r);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int rvm_t(IntPtr h, IntPtr a, IntPtr b, UIntPtr l, out UIntPtr r);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int pvm_t(IntPtr h, ref IntPtr a, ref UIntPtr s, uint p, out uint o);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int qit_t(IntPtr h, uint c, IntPtr b, uint l, out uint r);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int gnt_t(IntPtr p, IntPtr t, uint a, uint at, uint f, out IntPtr n);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)] public delegate int close_t(IntPtr h);
}
'@
Add-Type -TypeDefinition $src -Language CSharp
$nt = [N]::GetModuleHandleA("ntdll.dll")
function P($n) { [N]::GetProcAddress($nt, $n) }
$qvm = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtQueryVirtualMemory"), [N+qvm_t])
$rvm = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtReadVirtualMemory"), [N+rvm_t])

"GetCurrentProcess() = 0x{0:X}" -f ([N]::GetCurrentProcess().ToInt64())
$h1 = [IntPtr](-1)
$h2 = [N]::OpenProcess(0x1F0FFF, $false, [Diagnostics.Process]::GetCurrentProcess().Id)
"OpenProcess(all)     = 0x{0:X} err={1}" -f $h2.ToInt64(), [Runtime.InteropServices.Marshal]::GetLastWin32Error()

$p = [N]::VirtualAlloc([IntPtr]::Zero, [UIntPtr][uint64]0x800000, 0x2000, 0x04)
"alloc = 0x{0:X}" -f $p.ToInt64()
[N]::VirtualFree($p, [UIntPtr][uint64]0, 0x8000) | Out-Null
$mid = [IntPtr]::Add($p, 0x300000)

foreach ($tag in @(@('-1',$h1), @('OpenProcess',$h2))) {
    $name = $tag[0]; $h = [IntPtr]$tag[1]
    $buf = [Runtime.InteropServices.Marshal]::AllocHGlobal(48)
    for ($i=0;$i -lt 48;$i++){ [Runtime.InteropServices.Marshal]::WriteByte($buf,$i,0xCC) }
    $r = [UIntPtr]::Zero
    $st = $qvm.Invoke($h, $nt, 0, $buf, [UIntPtr][uint64]48, [ref]$r)
    $m = [Runtime.InteropServices.Marshal]::PtrToStructure($buf, [type][MEMORY_BASIC_INFORMATION])
    "[$name] mapped addr 0x{0:X} class0: status=0x{1:X8} RetLen={2} Base=0x{3:X} RegionSize=0x{4:X} State={5} Protect=0x{6:X}" -f `
        $nt.ToInt64(), $st, $r.ToUInt64(), $m.BaseAddress.ToInt64(), $m.RegionSize.ToInt64(), $m.State, $m.Protect
    [Runtime.InteropServices.Marshal]::FreeHGlobal($buf)
    $rb = [Runtime.InteropServices.Marshal]::AllocHGlobal(64)
    $nr = [UIntPtr]::MaxValue
    $s2 = $rvm.Invoke($h, $nt, $rb, [UIntPtr][uint64]16, [ref]$nr)
    "[$name] NtReadVirtualMemory(ntdll,16): status=0x{0:X8} read={1}" -f $s2, $nr.ToUInt64()
    [Runtime.InteropServices.Marshal]::FreeHGlobal($rb)
}
