# Read-only audit probe: measure real x64 ntdll semantics. No project files touched.
$ErrorActionPreference = 'Stop'

$src = @'
using System;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct CLIENT_ID { public IntPtr UniqueProcess; public IntPtr UniqueThread; }

[StructLayout(LayoutKind.Sequential)]
public struct MEMORY_BASIC_INFORMATION {
    public IntPtr BaseAddress;
    public IntPtr AllocationBase;
    public uint   AllocationProtect;
    public ushort PartitionId;
    public IntPtr RegionSize;
    public uint   State;
    public uint   Protect;
    public uint   Type;
}

[StructLayout(LayoutKind.Sequential)]
public struct UNICODE_STRING { public ushort Length; public ushort MaximumLength; public IntPtr Buffer; }

public static class N {
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr GetModuleHandleA(string n);
    [DllImport("kernel32.dll", CharSet=CharSet.Ansi, SetLastError=true)] public static extern IntPtr GetProcAddress(IntPtr h, string n);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern IntPtr VirtualAlloc(IntPtr a, UIntPtr s, uint t, uint p);
    [DllImport("kernel32.dll", SetLastError=true)] public static extern bool VirtualFree(IntPtr a, UIntPtr s, uint t);

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtQueryVirtualMemory_t(IntPtr h, IntPtr addr, uint cls, IntPtr buf, UIntPtr len, out UIntPtr ret);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtReadVirtualMemory_t(IntPtr h, IntPtr addr, IntPtr buf, UIntPtr len, out UIntPtr read);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtProtectVirtualMemory_t(IntPtr h, ref IntPtr addr, ref UIntPtr size, uint prot, out uint old);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtQueryInformationThread_t(IntPtr h, uint cls, IntPtr buf, uint len, out uint ret);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtGetNextThread_t(IntPtr ph, IntPtr th, uint access, uint attrs, uint flags, out IntPtr next);
    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    public delegate int NtClose_t(IntPtr h);
}
'@

Add-Type -TypeDefinition $src -Language CSharp

$nt = [N]::GetModuleHandleA("ntdll.dll")
function P($n) { [N]::GetProcAddress($nt, $n) }

$qvm = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtQueryVirtualMemory"), [N+NtQueryVirtualMemory_t])
$rvm = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtReadVirtualMemory"), [N+NtReadVirtualMemory_t])
$pvm = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtProtectVirtualMemory"), [N+NtProtectVirtualMemory_t])
$qit = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtQueryInformationThread"), [N+NtQueryInformationThread_t])
$gnt = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtGetNextThread"), [N+NtGetNextThread_t])
$ncl = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer((P "NtClose"), [N+NtClose_t])

$cur  = [IntPtr](-1)
$curT = [IntPtr](-2)

function QVMB($addr, $cls) {
    $sz = 48
    $buf = [Runtime.InteropServices.Marshal]::AllocHGlobal($sz)
    for ($i=0; $i -lt $sz; $i++) { [Runtime.InteropServices.Marshal]::WriteByte($buf, $i, 0xCC) }
    $ret = [UIntPtr]::Zero
    $st = $qvm.Invoke($cur, [IntPtr]$addr, $cls, $buf, [UIntPtr][uint64]$sz, [ref]$ret)
    $m = [Runtime.InteropServices.Marshal]::PtrToStructure($buf, [type][MEMORY_BASIC_INFORMATION])
    [Runtime.InteropServices.Marshal]::FreeHGlobal($buf)
    "  status=0x{0:X8} RetLen={1} Base=0x{2:X} AllocBase=0x{3:X} AllocProt=0x{4:X} RegionSize=0x{5:X} State={6} Protect=0x{7:X} Type={8}" -f `
        $st, $ret.ToUInt64(), $m.BaseAddress.ToInt64(), $m.AllocationBase.ToInt64(), $m.AllocationProtect, `
        $m.RegionSize.ToInt64(), $m.State, $m.Protect, $m.Type
}

"=== sizeof(MEMORY_BASIC_INFORMATION) = 48 (hardcoded, matches .NET layout)"
"=== sizeof(IntPtr) = $([IntPtr]::Size)"

$p = [N]::VirtualAlloc([IntPtr]::Zero, [UIntPtr][uint64]0x800000, 0x2000, 0x04)
if ($p -eq [IntPtr]::Zero) { "VirtualAlloc RESERVE FAILED err=$([Runtime.InteropServices.Marshal]::GetLastWin32Error())"; exit 1 }
"=== reserved 8MB base = 0x{0:X}" -f $p.ToInt64()
$ret = [N]::VirtualFree($p, [UIntPtr][uint64]0, 0x8000)
"=== VirtualFree(MEM_RELEASE) = $ret err=$([Runtime.InteropServices.Marshal]::GetLastWin32Error())"

"[1] FREE region, query at region start:"
QVMB $p 0
$mid = [IntPtr]::Add($p, 0x300000)
"[2] FREE region, query at base+0x300000 (0x{0:X}):" -f $mid.ToInt64()
QVMB $mid 0

"[3] FREE region, MemoryMappedFilenameInformation(2):"
$h = [Runtime.InteropServices.Marshal]::AllocHGlobal(1032)
for ($i=0; $i -lt 1032; $i++) { [Runtime.InteropServices.Marshal]::WriteByte($h, $i, 0xCC) }
$ret2 = [UIntPtr]::Zero
$st2 = $qvm.Invoke($cur, $mid, 2, $h, [UIntPtr][uint64]1032, [ref]$ret2)
$head = ((0..15 | ForEach-Object { ([Runtime.InteropServices.Marshal]::ReadByte($h,$_)).ToString('x2') }) -join '')
"  status=0x{0:X8} RetLen={1} first16={2}" -f $st2, $ret2.ToUInt64(), $head
[Runtime.InteropServices.Marshal]::FreeHGlobal($h)

"[3b] mapped page (ntdll base), MemoryMappedFilenameInformation(2):"
$hf = [Runtime.InteropServices.Marshal]::AllocHGlobal(1032)
$retf = [UIntPtr]::Zero
$stf = $qvm.Invoke($cur, $nt, 2, $hf, [UIntPtr][uint64]1032, [ref]$retf)
$usf = [Runtime.InteropServices.Marshal]::PtrToStructure($hf, [type][UNICODE_STRING])
$s = [Runtime.InteropServices.Marshal]::PtrToStringUni($usf.Buffer, $usf.Length/2)
"  status=0x{0:X8} RetLen={1} Length={2} str={3}" -f $stf, $retf.ToUInt64(), $usf.Length, $s
[Runtime.InteropServices.Marshal]::FreeHGlobal($hf)

"[4] NtReadVirtualMemory on FREE address, 64 bytes:"
$rb = [Runtime.InteropServices.Marshal]::AllocHGlobal(64)
for ($i=0; $i -lt 64; $i++) { [Runtime.InteropServices.Marshal]::WriteByte($rb, $i, 0xCC) }
$nr = [uint64]::MaxValue
$str = $rvm.Invoke($cur, $mid, $rb, [UIntPtr][uint64]64, [ref]$nr)
"  status=0x{0:X8} NumberOfBytesRead={1} (sentinel was {2})" -f $str, $nr.ToUInt64(), [uint64]::MaxValue

"[4b] NtReadVirtualMemory on FREE address, 0 bytes:"
$nr2 = [uint64]::MaxValue
$str2 = $rvm.Invoke($cur, $mid, $rb, [UIntPtr][uint64]0, [ref]$nr2)
"  status=0x{0:X8} NumberOfBytesRead={1}" -f $str2, $nr2.ToUInt64()

"[4c] straddle: 1 committed page + 1 decommitted page, read 0x40 from page start:"
$pr = [N]::VirtualAlloc([IntPtr]::Zero, [UIntPtr][uint64]0x2000, 0x3000, 0x04)
[N]::VirtualFree([IntPtr]::Add($pr, 0x1000), [UIntPtr][uint64]0x1000, 0x4000) | Out-Null
$nr3 = [uint64]::MaxValue
$str3 = $rvm.Invoke($cur, $pr, $rb, [UIntPtr][uint64]0x40, [ref]$nr3)
"  status=0x{0:X8} NumberOfBytesRead={1}" -f $str3, $nr3.ToUInt64()
$nr4 = [uint64]::MaxValue
$str4 = $rvm.Invoke($cur, [IntPtr]::Add($pr, 0xF00), $rb, [UIntPtr][uint64]0x200, [ref]$nr4)
"  read 0x200 starting 0x100 before decommit boundary: status=0x{0:X8} NumberOfBytesRead={1}" -f $str4, $nr4.ToUInt64()
$nr5 = [uint64]::MaxValue
$str5 = $rvm.Invoke($cur, [IntPtr]::Add($pr, 0x1000), $rb, [UIntPtr][uint64]0x40, [ref]$nr5)
"  read from decommitted page directly: status=0x{0:X8} NumberOfBytesRead={1}" -f $str5, $nr5.ToUInt64()

"[5] NtQueryInformationThread(ThreadBasicInformation=0) length sensitivity:"
$buf = [Runtime.InteropServices.Marshal]::AllocHGlobal(64)
foreach ($len in @(48, 40, 47, 24, 16, 0, 64)) {
    for ($i=0; $i -lt 64; $i++) { [Runtime.InteropServices.Marshal]::WriteByte($buf, $i, 0xCC) }
    $r = 0
    $st = $qit.Invoke($curT, 0, $buf, [uint32]$len, [ref]$r)
    $teb = [Runtime.InteropServices.Marshal]::ReadIntPtr($buf, 8)
    $tid = [Runtime.InteropServices.Marshal]::ReadIntPtr($buf, 24)
    "  len={0,-3} status=0x{1:X8} RetLen={2} TEB=0x{3:X} ClientId.UniqueThread={4} ExitStatus@0=0x{5:X8}" -f `
        $len, $st, $r, $teb.ToInt64(), $tid.ToInt64(), [Runtime.InteropServices.Marshal]::ReadInt32($buf,0)
}
[Runtime.InteropServices.Marshal]::FreeHGlobal($buf)

"[6] NtGetNextThread semantics:"
$ph = [IntPtr](-1)
$nx = [IntPtr]::Zero
$stn = $gnt.Invoke($ph, [IntPtr]::Zero, 0x0040, 0, 0, [ref]$nx)
"  cur=NULL access=THREAD_QUERY_INFORMATION(0x40): status=0x{0:X8} handle=0x{1:X}" -f $stn, $nx.ToInt64()
if ($nx -ne [IntPtr]::Zero) {
    for ($i=0; $i -lt 4; $i++) {
        $nx2 = [IntPtr]::Zero
        $stn2 = $gnt.Invoke($ph, $nx, 0x0040, 0, 0, [ref]$nx2)
        if ($stn2 -lt 0) { "  iter{0}: status=0x{1:X8} -> enumeration end (cursor handle leaked? no, closed below)" -f ($i+1), $stn2; break }
        "  iter{0}: status=0x{1:X8} handle=0x{2:X}" -f ($i+1), $stn2, $nx2.ToInt64()
        [void]$ncl.Invoke($nx)
        $nx = $nx2
    }
    [void]$ncl.Invoke($nx)
}
$nx4 = [IntPtr]::Zero
$stn4 = $gnt.Invoke($ph, [IntPtr]::Zero, 0, 0, 0, [ref]$nx4)
"  access=0: status=0x{0:X8} handle=0x{1:X}" -f $stn4, $nx4.ToInt64()
if ($nx4 -ne [IntPtr]::Zero) {
    $b48 = [Runtime.InteropServices.Marshal]::AllocHGlobal(48)
    $r4 = 0
    $stq = $qit.Invoke($nx4, 0, $b48, 48, [ref]$r4)
    "    access=0 handle -> QueryInformationThread(TBI): status=0x{0:X8} RetLen={1}" -f $stq, $r4
    [Runtime.InteropServices.Marshal]::FreeHGlobal($b48)
    [void]$ncl.Invoke($nx4)
}
$nx5 = [IntPtr]::Zero
$stn5 = $gnt.Invoke($ph, [IntPtr](-2), 0x0040, 0, 0, [ref]$nx5)
"  cur=pseudo GetCurrentThread(-2) access=0x40: status=0x{0:X8} handle=0x{1:X}" -f $stn5, $nx5.ToInt64()
if ($nx5 -ne [IntPtr]::Zero) { [void]$ncl.Invoke($nx5) }

"[7] NtProtectVirtualMemory guard behavior:"
$gp = [N]::VirtualAlloc([IntPtr]::Zero, [UIntPtr][uint64]0x2000, 0x3000, 0x04)
$ra = $gp; $rs = [UIntPtr][uint64]0x1000; $oldp = 0
$stp1 = $pvm.Invoke($cur, [ref]$ra, [ref]$rs, (0x04 -bor 0x100), [ref]$oldp)
"  set RW|GUARD (0x104): status=0x{0:X8} old=0x{1:X} appliedLen=0x{2:X}" -f $stp1, $oldp, $rs.ToUInt64()
$ra = $gp; $rs = [UIntPtr][uint64]0x1000; $oldp2 = 0
$stp2 = $pvm.Invoke($cur, [ref]$ra, [ref]$rs, (0x04 -bor 0x100), [ref]$oldp2)
"  set RW|GUARD again:  status=0x{0:X8} old=0x{1:X}" -f $stp2, $oldp2
$ra = $gp; $rs = [UIntPtr][uint64]0x1000; $oldp3 = 0
$stp3 = $pvm.Invoke($cur, [ref]$ra, [ref]$rs, 0x04, [ref]$oldp3)
"  clear guard (RW):     status=0x{0:X8} old=0x{1:X}" -f $stp3, $oldp3

"=== probe done ==="
