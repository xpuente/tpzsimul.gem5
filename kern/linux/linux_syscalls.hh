/*
 * Copyright (c) 2003 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LINUX_SYSCALLS_HH__
#define __LINUX_SYSCALLS_HH__

#include "targetarch/syscalls.hh"
#include "kern/linux/linux.hh"

struct SystemCalls<Linux>
{
    enum {
        syscall = 0,
    llseek = 1,
    newselect = 2,
    sysctl = 3,
    access = 4,
    acct = 5,
    adjtimex = 6,
    afs_syscall = 7,
    alarm = 8,
    bdflush = 9,
    _break = 10, /*renamed from break*/
    brk = 11,
    capget = 12,
    capset = 13,
    chdir = 14,
    chmod = 15,
    chown = 16,
    chown32 = 17,
    chroot = 18,
    clock_getres = 19,
    clock_gettime = 20,
    clock_nanosleep = 21,
    clock_settime = 22,
    clone = 23,
    close = 24,
    creat = 25,
    create_module = 26,
    delete_module = 27,
    dup = 28,
    dup2 = 29,
    epoll_create = 30,
    epoll_ctl = 31,
    epoll_wait = 32,
    execve = 33,
    exit = 34,
    exit_group = 35,
    fadvise64 = 36,
    fadvise64_64 = 37,
    fchdir = 38,
    fchmod = 39,
    fchown = 40,
    fchown32 = 41,
    fcntl = 42,
    fcntl64 = 43,
    fdatasync = 44,
    fgetxattr = 45,
    flistxattr = 46,
    flock = 47,
    fork = 48,
    fremovexattr = 49,
    fsetxattr = 50,
    fstat = 51,
    fstat64 = 52,
    fstatfs = 53,
    fstatfs64 = 54,
    fsync = 55,
    ftime = 56,
    ftruncate = 57,
    ftruncate64 = 58,
    futex = 59,
    get_kernel_syms = 60,
    get_thread_area = 61,
    getcwd = 62,
    getdents = 63,
    getdents64 = 64,
    getegid = 65,
    getegid32 = 66,
    geteuid = 67,
    geteuid32 = 68,
    getgid = 69,
    getgid32 = 70,
    getgroups = 71,
    getgroups32 = 72,
    getitimer = 73,
    getpgid = 74,
    getpgrp = 75,
    getpid = 76,
    getpmsg = 77,
    getppid = 78,
    getpriority = 79,
    getresgid = 80,
    getresgid32 = 81,
    getresuid = 82,
    getresuid32 = 83,
    getrlimit = 84,
    getrusage = 85,
    getsid = 86,
    gettid = 87,
    gettimeofday = 88,
    getuid = 89,
    getuid32 = 90,
    getxattr = 91,
    gtty = 92,
    idle = 93,
    init_module = 94,
    io_cancel = 95,
    io_destroy = 96,
    io_getevents = 97,
    io_setup = 98,
    io_submit = 99,
    ioctl = 100,
    ioperm = 101,
    iopl = 102,
    ipc = 103,
    kill = 104,
    lchown = 105,
    lchown32 = 106,
    lgetxattr = 107,
    link = 108,
    listxattr = 109,
    llistxattr = 110,
    lock = 111,
    lookup_dcookie = 112,
    lremovexattr = 113,
    lseek = 114,
    lsetxattr = 115,
    lstat = 116,
    lstat64 = 117,
    madvise = 118,
    madvise1 = 119,
    mincore = 120,
    mkdir = 121,
    mknod = 122,
    mlock = 123,
    mlockall = 124,
    mmap = 125,
    mmap2 = 126,
    modify_ldt = 127,
    mount = 128,
    mprotect = 129,
    mpx = 130,
    mremap = 131,
    msync = 132,
    munlock = 133,
    munlockall = 134,
    munmap = 135,
    nanosleep = 136,
    nfsservctl = 137,
    nice = 138,
    oldfstat = 139,
    oldlstat = 140,
    oldolduname = 141,
    oldstat = 142,
    olduname = 143,
    open = 144,
    pause = 145,
    personality = 146,
    pipe = 147,
    pivot_root = 148,
    poll = 149,
    prctl = 150,
    pread64 = 151,
    prof = 152,
    profil = 153,
    ptrace = 154,
    putpmsg = 155,
    pwrite64 = 156,
    query_module = 157,
    quotactl = 158,
    read = 159,
    readahead = 160,
    readdir = 161,
    readlink = 162,
    readv = 163,
    reboot = 164,
    remap_file_pages = 165,
    removexattr = 166,
    rename = 167,
    restart_syscall = 168,
    rmdir = 169,
    rt_sigaction = 170,
    rt_sigpending = 171,
    rt_sigprocmask = 172,
    rt_sigqueueinfo = 173,
    rt_sigreturn = 174,
    rt_sigsuspend = 175,
    rt_sigtimedwait = 176,
    sched_get_priority_max = 177,
    sched_get_priority_min = 178,
    sched_getaffinity = 179,
    sched_getparam = 180,
    sched_getscheduler = 181,
    sched_rr_get_interval = 182,
    sched_setaffinity = 183,
    sched_setparam = 184,
    sched_setscheduler = 185,
    sched_yield = 186,
    select = 187,
    sendfile = 188,
    sendfile64 = 189,
    set_thread_area = 190,
    set_tid_address = 191,
    setdomainname = 192,
    setfsgid = 193,
    setfsgid32 = 194,
    setfsuid = 195,
    setfsuid32 = 196,
    setgid = 197,
    setgid32 = 198,
    setgroups = 199,
    setgroups32 = 200,
    sethostname = 201,
    setitimer = 202,
    setpgid = 203,
    setpriority = 204,
    setregid = 205,
    setregid32 = 206,
    setresgid = 207,
    setresgid32 = 208,
    setresuid = 209,
    setresuid32 = 210,
    setreuid = 211,
    setreuid32 = 212,
    setrlimit = 213,
    setsid = 214,
    settimeofday = 215,
    setuid = 216,
    setuid32 = 217,
    setxattr = 218,
    sgetmask = 219,
    sigaction = 220,
    sigaltstack = 221,
    signal = 222,
    sigpending = 223,
    sigprocmask = 224,
    sigreturn = 225,
    sigsuspend = 226,
    socketcall = 227,
    ssetmask = 228,
    stat = 229,
    stat64 = 230,
    statfs = 231,
    statfs64 = 232,
    stime = 233,
    stty = 234,
    swapoff = 235,
    swapon = 236,
    symlink = 237,
    sync = 238,
    sysfs = 239,
    sysinfo = 240,
    syslog = 241,
    tgkill = 242,
    time = 243,
    timer_create = 244,
    timer_delete = 245,
    timer_getoverrun = 246,
    timer_gettime = 247,
    timer_settime = 248,
    times = 249,
    tkill = 250,
    truncate = 251,
    truncate64 = 252,
    ugetrlimit = 253,
    ulimit = 254,
    umask = 255,
    umount = 256,
    umount2 = 257,
    uname = 258,
    unlink = 259,
    uselib = 260,
    ustat = 261,
    utime = 262,
    utimes = 263,
    vfork = 264,
    vhangup = 265,
    vm86 = 266,
    vm86old = 267,
    vserver = 268,
    wait4 = 269,
    waitpid = 270,
    write = 271,
    writev = 272,
        StandardNumber
    };

    static const int Number = StandardNumber;

    static const char *name(int num);

    static bool validSyscallNumber(int num) {
        return num < StandardNumber;
    }

    /* why does this exist, I don't think it is needed for linux */
    static int convert(int syscall_num) {
        if (!validSyscallNumber(syscall_num))
            return -1;

        return syscall_num ;
    }
};

#endif // __LINUX_SYSCALLS_HH__
