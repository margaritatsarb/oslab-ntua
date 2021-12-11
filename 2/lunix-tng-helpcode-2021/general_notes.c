-----------------------------------------------------------------------------------------------------------
strace insmod ./lunix.ko
-----------------------------------------------------------------------------------------------------------
execve("/sbin/insmod", ["insmod", "./lunix.ko"], 0x7fff69de3e38 /* 30 vars */) = 0
brk(NULL)                               = 0x5577af6bb000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=37738, ...}) = 0
mmap(NULL, 37738, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f14dee2f000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/liblzma.so.5", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0205\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=158400, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f14dee2d000
mmap(NULL, 160400, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f14dee05000
mmap(0x7f14dee08000, 98304, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x3000) = 0x7f14dee08000
mmap(0x7f14dee20000, 45056, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b000) = 0x7f14dee20000
mmap(0x7f14dee2b000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x25000) = 0x7f14dee2b000
close(3)                                = 0
openat(AT_FDCWD, "/usr/lib/x86_64-linux-gnu/libcrypto.so.1.1", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0\360\10\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=3031904, ...}) = 0
mmap(NULL, 3051424, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f14deb1c000
mprotect(0x7f14deba1000, 2285568, PROT_NONE) = 0
mmap(0x7f14deba1000, 1695744, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x85000) = 0x7f14deba1000
mmap(0x7f14ded3f000, 585728, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x223000) = 0x7f14ded3f000
mmap(0x7f14dedcf000, 204800, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2b2000) = 0x7f14dedcf000
mmap(0x7f14dee01000, 16288, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f14dee01000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f14de95b000
mprotect(0x7f14de97d000, 1658880, PROT_NONE) = 0
mmap(0x7f14de97d000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f14de97d000
mmap(0x7f14deac5000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f14deac5000
mmap(0x7f14deb12000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f14deb12000
mmap(0x7f14deb18000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f14deb18000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libpthread.so.0", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0@l\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=146968, ...}) = 0
mmap(NULL, 132288, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f14de93a000
mmap(0x7f14de940000, 61440, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x6000) = 0x7f14de940000
mmap(0x7f14de94f000, 24576, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x15000) = 0x7f14de94f000
mmap(0x7f14de955000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1a000) = 0x7f14de955000
mmap(0x7f14de957000, 13504, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f14de957000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libdl.so.2", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0000\21\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=14592, ...}) = 0
mmap(NULL, 16656, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f14de935000
mmap(0x7f14de936000, 4096, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1000) = 0x7f14de936000
mmap(0x7f14de937000, 4096, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2000) = 0x7f14de937000
mmap(0x7f14de938000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x2000) = 0x7f14de938000
close(3)                                = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f14de933000
arch_prctl(ARCH_SET_FS, 0x7f14de934480) = 0
mprotect(0x7f14deb12000, 16384, PROT_READ) = 0
mprotect(0x7f14de938000, 4096, PROT_READ) = 0
mprotect(0x7f14de955000, 4096, PROT_READ) = 0
mprotect(0x7f14dedcf000, 196608, PROT_READ) = 0
mprotect(0x7f14dee2b000, 4096, PROT_READ) = 0
mprotect(0x5577ad90c000, 8192, PROT_READ) = 0
mprotect(0x7f14dee60000, 4096, PROT_READ) = 0
munmap(0x7f14dee2f000, 37738)           = 0
set_tid_address(0x7f14de934750)         = 765
set_robust_list(0x7f14de934760, 24)     = 0
rt_sigaction(SIGRTMIN, {sa_handler=0x7f14de9406b0, sa_mask=[], sa_flags=SA_RESTORER|SA_SIGINFO, sa_restorer=0x7f14de94c730}, NULL, 8) = 0
rt_sigaction(SIGRT_1, {sa_handler=0x7f14de940740, sa_mask=[], sa_flags=SA_RESTORER|SA_RESTART|SA_SIGINFO, sa_restorer=0x7f14de94c730}, NULL, 8) = 0
rt_sigprocmask(SIG_UNBLOCK, [RTMIN RT_1], NULL, 8) = 0
prlimit64(0, RLIMIT_STACK, NULL, {rlim_cur=8192*1024, rlim_max=RLIM64_INFINITY}) = 0
brk(NULL)                               = 0x5577af6bb000
brk(0x5577af6dc000)                     = 0x5577af6dc000
uname({sysname="Linux", nodename="utopia", ...}) = 0
openat(AT_FDCWD, "/lib/modules/4.19.0-6-amd64/modules.softdep", O_RDONLY|O_CLOEXEC) = 3
fcntl(3, F_GETFL)                       = 0x8000 (flags O_RDONLY|O_LARGEFILE)
fstat(3, {st_mode=S_IFREG|0644, st_size=800, ...}) = 0
read(3, "# Soft dependencies extracted fr"..., 4096) = 800
read(3, "", 4096)                       = 0
close(3)                                = 0
openat(AT_FDCWD, "/proc/cmdline", O_RDONLY|O_CLOEXEC) = 3
read(3, "BOOT_IMAGE=/boot/vmlinuz-4.19.0-"..., 4095) = 96
read(3, "", 3999)                       = 0
close(3)                                = 0
stat(".", {st_mode=S_IFDIR|0755, st_size=4096, ...}) = 0
stat("/home/user/lunix-tng", {st_mode=S_IFDIR|0755, st_size=4096, ...}) = 0
stat("/home/user/lunix-tng/./lunix.ko", {st_mode=S_IFREG|0644, st_size=2809736, ...}) = 0
openat(AT_FDCWD, "/home/user/lunix-tng/./lunix.ko", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1", 6)               = 6
lseek(3, 0, SEEK_SET)                   = 0
fstat(3, {st_mode=S_IFREG|0644, st_size=2809736, ...}) = 0
mmap(NULL, 2809736, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f14de685000
finit_module(3, "", 0)                  = 0
munmap(0x7f14de685000, 2809736)         = 0
close(3)                                = 0
exit_group(0)                           = ?
+++ exited with 0 +++
-----------------------------------------------------------------------------------------------------------
strace -f ./lunix-attach /dev/ttyS0
-----------------------------------------------------------------------------------------------------------
execve("./lunix-attach", ["./lunix-attach", "/dev/ttyS0"], 0x7ffc2e57aa10 /* 30 vars */) = 0
brk(NULL)                               = 0x564788081000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=37738, ...}) = 0
mmap(NULL, 37738, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7ff237aa7000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7ff237aa5000
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7ff2378e4000
mprotect(0x7ff237906000, 1658880, PROT_NONE) = 0
mmap(0x7ff237906000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7ff237906000
mmap(0x7ff237a4e000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7ff237a4e000
mmap(0x7ff237a9b000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7ff237a9b000
mmap(0x7ff237aa1000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7ff237aa1000
close(3)                                = 0
arch_prctl(ARCH_SET_FS, 0x7ff237aa6500) = 0
mprotect(0x7ff237a9b000, 16384, PROT_READ) = 0
mprotect(0x564786f9d000, 4096, PROT_READ) = 0
mprotect(0x7ff237ad8000, 4096, PROT_READ) = 0
munmap(0x7ff237aa7000, 37738)           = 0
write(2, "tty_open: looking for lock\n", 27tty_open: looking for lock
) = 27
brk(NULL)                               = 0x564788081000
brk(0x5647880a2000)                     = 0x5647880a2000
openat(AT_FDCWD, "/var/lock/LCK..ttyS0", O_RDONLY) = -1 ENOENT (No such file or directory)
creat("/var/lock/LCK..ttyS0", 0644)     = 3
getpid()                                = 1157
write(3, "      1157\n", 11)            = 11
close(3)                                = 0
socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0) = 3
connect(3, {sa_family=AF_UNIX, sun_path="/var/run/nscd/socket"}, 110) = -1 ENOENT (No such file or directory)
close(3)                                = 0
socket(AF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0) = 3
connect(3, {sa_family=AF_UNIX, sun_path="/var/run/nscd/socket"}, 110) = -1 ENOENT (No such file or directory)
close(3)                                = 0
openat(AT_FDCWD, "/etc/nsswitch.conf", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=510, ...}) = 0
read(3, "# /etc/nsswitch.conf\n#\n# Example"..., 4096) = 510
read(3, "", 4096)                       = 0
close(3)                                = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=37738, ...}) = 0
mmap(NULL, 37738, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7ff237aa7000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libnss_files.so.2", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\0\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\0003\0\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0644, st_size=55792, ...}) = 0
mmap(NULL, 83768, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7ff2378cf000
mprotect(0x7ff2378d2000, 40960, PROT_NONE) = 0
mmap(0x7ff2378d2000, 28672, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x3000) = 0x7ff2378d2000
mmap(0x7ff2378d9000, 8192, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0xa000) = 0x7ff2378d9000
mmap(0x7ff2378dc000, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0xc000) = 0x7ff2378dc000
mmap(0x7ff2378de000, 22328, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7ff2378de000
close(3)                                = 0
mprotect(0x7ff2378dc000, 4096, PROT_READ) = 0
munmap(0x7ff237aa7000, 37738)           = 0
openat(AT_FDCWD, "/etc/passwd", O_RDONLY|O_CLOEXEC) = 3
lseek(3, 0, SEEK_CUR)                   = 0
fstat(3, {st_mode=S_IFREG|0644, st_size=1614, ...}) = 0
read(3, "root:x:0:0:root:/root:/bin/bash\n"..., 4096) = 1614
close(3)                                = 0
chown("/var/lock/LCK..ttyS0", 10, 10)   = 0
write(2, "tty_open: trying to open /dev/tt"..., 36tty_open: trying to open /dev/ttyS0
) = 36
openat(AT_FDCWD, "/dev/ttyS0", O_RDWR|O_NONBLOCK) = 3
write(2, "tty_open: /dev/ttyS0 (fd=3) ", 28tty_open: /dev/ttyS0 (fd=3) ) = 28
ioctl(3, TCGETS, {B9600 opost isig icanon echo ...}) = 0
ioctl(3, TIOCGETD, [0])                 = 0
ioctl(3, SNDCTL_TMR_START or TCSETS, {B57600 -opost -isig -icanon -echo ...}) = 0
ioctl(3, TIOCSETD, [8])                 = 0
write(2, "Line discipline set on /dev/ttyS"..., 66Line discipline set on /dev/ttyS0, press ^C to release the TTY...
) = 66
rt_sigaction(SIGHUP, {sa_handler=0x564786f9af3a, sa_mask=[HUP], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7ff23791b840}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGINT, {sa_handler=0x564786f9af3a, sa_mask=[INT], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7ff23791b840}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGQUIT, {sa_handler=0x564786f9af3a, sa_mask=[QUIT], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7ff23791b840}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
rt_sigaction(SIGTERM, {sa_handler=0x564786f9af3a, sa_mask=[TERM], sa_flags=SA_RESTORER|SA_RESTART, sa_restorer=0x7ff23791b840}, {sa_handler=SIG_DFL, sa_mask=[], sa_flags=0}, 8) = 0
pause(
-----------------------------------------------------------------------------------------------------------
strace -f  cat /dev/lunix0-batt
-----------------------------------------------------------------------------------------------------------
execve("/bin/cat", ["cat", "/dev/lunix0-batt"], 0x7ffe29175930 /* 30 vars */) = 0
brk(NULL)                               = 0x5608da3d4000
access("/etc/ld.so.preload", R_OK)      = -1 ENOENT (No such file or directory)
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=37738, ...}) = 0
mmap(NULL, 37738, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f1935a07000
close(3)                                = 0
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
read(3, "\177ELF\2\1\1\3\0\0\0\0\0\0\0\0\3\0>\0\1\0\0\0\260A\2\0\0\0\0\0"..., 832) = 832
fstat(3, {st_mode=S_IFREG|0755, st_size=1824496, ...}) = 0
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f1935a05000
mmap(NULL, 1837056, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7f1935844000
mprotect(0x7f1935866000, 1658880, PROT_NONE) = 0
mmap(0x7f1935866000, 1343488, PROT_READ|PROT_EXEC, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x22000) = 0x7f1935866000
mmap(0x7f19359ae000, 311296, PROT_READ, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x16a000) = 0x7f19359ae000
mmap(0x7f19359fb000, 24576, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_DENYWRITE, 3, 0x1b6000) = 0x7f19359fb000
mmap(0x7f1935a01000, 14336, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0) = 0x7f1935a01000
close(3)                                = 0
arch_prctl(ARCH_SET_FS, 0x7f1935a06540) = 0
mprotect(0x7f19359fb000, 16384, PROT_READ) = 0
mprotect(0x5608d884f000, 4096, PROT_READ) = 0
mprotect(0x7f1935a38000, 4096, PROT_READ) = 0
munmap(0x7f1935a07000, 37738)           = 0
brk(NULL)                               = 0x5608da3d4000
brk(0x5608da3f5000)                     = 0x5608da3f5000
openat(AT_FDCWD, "/usr/lib/locale/locale-archive", O_RDONLY|O_CLOEXEC) = 3
fstat(3, {st_mode=S_IFREG|0644, st_size=3354080, ...}) = 0
mmap(NULL, 3354080, PROT_READ, MAP_PRIVATE, 3, 0) = 0x7f1935511000
close(3)                                = 0
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0x1), ...}) = 0
openat(AT_FDCWD, "/dev/lunix0-batt", O_RDONLY) = 3
fstat(3, {st_mode=S_IFCHR|0644, st_rdev=makedev(0x3c, 0), ...}) = 0
fadvise64(3, 0, 0, POSIX_FADV_SEQUENTIAL) = 0
mmap(NULL, 139264, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f19354ef000
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3, "3.354\n", 131072)              = 6
write(1, "3.354\n", 63.354
)                  = 6
read(3,
