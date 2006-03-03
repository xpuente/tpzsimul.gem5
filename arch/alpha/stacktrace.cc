/*
 * Copyright (c) 2005 The Regents of The University of Michigan
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

#include <string>

#include "arch/alpha/isa_traits.hh"
#include "arch/alpha/stacktrace.hh"
#include "arch/alpha/vtophys.hh"
#include "base/bitfield.hh"
#include "base/trace.hh"
#include "cpu/base.hh"
#include "cpu/exec_context.hh"

using namespace std;
using namespace AlphaISA;

ProcessInfo::ProcessInfo(ExecContext *_xc)
    : xc(_xc)
{
    Addr addr = 0;

    if (!xc->system->kernelSymtab->findAddress("thread_info_size", addr))
        panic("thread info not compiled into kernel\n");
    thread_info_size = *(int32_t *)vtomem(xc, addr, sizeof(int32_t));

    if (!xc->system->kernelSymtab->findAddress("task_struct_size", addr))
        panic("thread info not compiled into kernel\n");
    task_struct_size = *(int32_t *)vtomem(xc, addr, sizeof(int32_t));

    if (!xc->system->kernelSymtab->findAddress("thread_info_task", addr))
        panic("thread info not compiled into kernel\n");
    task_off = *(int32_t *)vtomem(xc, addr, sizeof(int32_t));

    if (!xc->system->kernelSymtab->findAddress("task_struct_pid", addr))
        panic("thread info not compiled into kernel\n");
    pid_off = *(int32_t *)vtomem(xc, addr, sizeof(int32_t));

    if (!xc->system->kernelSymtab->findAddress("task_struct_comm", addr))
        panic("thread info not compiled into kernel\n");
    name_off = *(int32_t *)vtomem(xc, addr, sizeof(int32_t));
}

Addr
ProcessInfo::task(Addr ksp) const
{
    Addr base = ksp & ~0x3fff;
    if (base == ULL(0xfffffc0000000000))
        return 0;

    Addr task;
    CopyOut(xc, &task, base + task_off, sizeof(task));
    return task;
}

int
ProcessInfo::pid(Addr ksp) const
{
    Addr task = this->task(ksp);
    if (!task)
        return -1;

    uint16_t pid;
    CopyOut(xc, &pid, task + pid_off, sizeof(pid));
    return pid;
}

string
ProcessInfo::name(Addr ksp) const
{
    Addr task = this->task(ksp);
    if (!task)
        return "console";

    char comm[256];
    CopyString(xc, comm, task + name_off, sizeof(comm));
    if (!comm[0])
        return "startup";

    return comm;
}

StackTrace::StackTrace()
    : xc(0), stack(64)
{
}

StackTrace::StackTrace(ExecContext *_xc, StaticInstPtr inst)
    : xc(0), stack(64)
{
    trace(_xc, inst);
}

StackTrace::~StackTrace()
{
}

void
StackTrace::trace(ExecContext *_xc, bool is_call)
{
    xc = _xc;

    bool usermode = (xc->readMiscReg(AlphaISA::IPR_DTB_CM) & 0x18) != 0;

    Addr pc = xc->regs.npc;
    bool kernel = xc->system->kernelStart <= pc && pc <= xc->system->kernelEnd;

    if (usermode) {
        stack.push_back(user);
        return;
    }

    if (!kernel) {
        stack.push_back(console);
        return;
    }

    SymbolTable *symtab = xc->system->kernelSymtab;
    Addr ksp = xc->regs.intRegFile[TheISA::StackPointerReg];
    Addr bottom = ksp & ~0x3fff;
    Addr addr;

    if (is_call) {
        if (!symtab->findNearestAddr(pc, addr))
            panic("could not find address %#x", pc);

        stack.push_back(addr);
        pc = xc->regs.pc;
    }

    Addr ra;
    int size;

    while (ksp > bottom) {
        if (!symtab->findNearestAddr(pc, addr))
            panic("could not find symbol for pc=%#x", pc);
        assert(pc >= addr && "symbol botch: callpc < func");

        stack.push_back(addr);

        if (isEntry(addr))
            return;

        if (decodePrologue(ksp, pc, addr, size, ra)) {
            if (!ra)
                return;

            if (size <= 0) {
                stack.push_back(unknown);
                return;
            }

            pc = ra;
            ksp += size;
        } else {
            stack.push_back(unknown);
            return;
        }

        bool kernel = xc->system->kernelStart <= pc &&
            pc <= xc->system->kernelEnd;
        if (!kernel)
            return;

        if (stack.size() >= 1000)
            panic("unwinding too far");
    }

    panic("unwinding too far");
}

bool
StackTrace::isEntry(Addr addr)
{
    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp12))
        return true;

    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp7))
        return true;

    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp11))
        return true;

    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp21))
        return true;

    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp9))
        return true;

    if (addr == xc->readMiscReg(AlphaISA::IPR_PALtemp2))
        return true;

    return false;
}

bool
StackTrace::decodeStack(MachInst inst, int &disp)
{
    // lda $sp, -disp($sp)
    //
    // Opcode<31:26> == 0x08
    // RA<25:21> == 30
    // RB<20:16> == 30
    // Disp<15:0>
    const MachInst mem_mask = 0xffff0000;
    const MachInst lda_pattern = 0x23de0000;
    const MachInst lda_disp_mask = 0x0000ffff;

    // subq $sp, disp, $sp
    // addq $sp, disp, $sp
    //
    // Opcode<31:26> == 0x10
    // RA<25:21> == 30
    // Lit<20:13>
    // One<12> = 1
    // Func<11:5> == 0x20 (addq)
    // Func<11:5> == 0x29 (subq)
    // RC<4:0> == 30
    const MachInst intop_mask = 0xffe01fff;
    const MachInst addq_pattern = 0x43c0141e;
    const MachInst subq_pattern = 0x43c0153e;
    const MachInst intop_disp_mask = 0x001fe000;
    const int intop_disp_shift = 13;

    if ((inst & mem_mask) == lda_pattern)
        disp = -sext<16>(inst & lda_disp_mask);
    else if ((inst & intop_mask) == addq_pattern)
        disp = -int((inst & intop_disp_mask) >> intop_disp_shift);
    else if ((inst & intop_mask) == subq_pattern)
        disp = int((inst & intop_disp_mask) >> intop_disp_shift);
    else
        return false;

    return true;
}

bool
StackTrace::decodeSave(MachInst inst, int &reg, int &disp)
{
    // lda $stq, disp($sp)
    //
    // Opcode<31:26> == 0x08
    // RA<25:21> == ?
    // RB<20:16> == 30
    // Disp<15:0>
    const MachInst stq_mask = 0xfc1f0000;
    const MachInst stq_pattern = 0xb41e0000;
    const MachInst stq_disp_mask = 0x0000ffff;
    const MachInst reg_mask = 0x03e00000;
    const int reg_shift = 21;

    if ((inst & stq_mask) == stq_pattern) {
        reg = (inst & reg_mask) >> reg_shift;
        disp = sext<16>(inst & stq_disp_mask);
    } else {
        return false;
    }

    return true;
}

/*
 * Decode the function prologue for the function we're in, and note
 * which registers are stored where, and how large the stack frame is.
 */
bool
StackTrace::decodePrologue(Addr sp, Addr callpc, Addr func,
                           int &size, Addr &ra)
{
    size = 0;
    ra = 0;

    for (Addr pc = func; pc < callpc; pc += sizeof(MachInst)) {
        MachInst inst;
        CopyOut(xc, (uint8_t *)&inst, pc, sizeof(MachInst));

        int reg, disp;
        if (decodeStack(inst, disp)) {
            if (size) {
                // panic("decoding frame size again");
                return true;
            }
            size += disp;
        } else if (decodeSave(inst, reg, disp)) {
            if (!ra && reg == ReturnAddressReg) {
                CopyOut(xc, (uint8_t *)&ra, sp + disp, sizeof(Addr));
                if (!ra) {
                    // panic("no return address value pc=%#x\n", pc);
                    return false;
                }
            }
        }
    }

    return true;
}

#if TRACING_ON
void
StackTrace::dump()
{
    StringWrap name(xc->cpu->name());
    SymbolTable *symtab = xc->system->kernelSymtab;

    DPRINTFN("------ Stack ------\n");

    string symbol;
    for (int i = 0, size = stack.size(); i < size; ++i) {
        Addr addr = stack[size - i - 1];
        if (addr == user)
            symbol = "user";
        else if (addr == console)
            symbol = "console";
        else if (addr == unknown)
            symbol = "unknown";
        else
            symtab->findSymbol(addr, symbol);

        DPRINTFN("%#x: %s\n", addr, symbol);
    }
}
#endif
