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

#include <iostream>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.hh"
#include "cprintf.hh"
#include "misc.hh"
#include "smt.hh"

#include "annotation.hh"
#include "exec_context.hh"
#include "base_cpu.hh"
#include "debug.hh"
#include "simple_cpu.hh"
#include "inifile.hh"
#include "mem_interface.hh"
#include "base_mem.hh"
#include "static_inst.hh"

#ifdef FULL_SYSTEM
#include "memory_control.hh"
#include "physical_memory.hh"
#include "alpha_memory.hh"
#include "system.hh"
#else // !FULL_SYSTEM
#include "functional_memory.hh"
#include "prog.hh"
#include "eio.hh"
#endif // FULL_SYSTEM

#include "exetrace.hh"
#include "trace.hh"
#include "sim_events.hh"
#include "pollevent.hh"
#include "sim_object.hh"
#include "sim_stats.hh"

#include "range.hh"
#include "symtab.hh"

#ifdef FULL_SYSTEM
#include "vtophys.hh"
#include "pciareg.h"
#include "remote_gdb.hh"
#include "alpha_access.h"
#endif


using namespace std;

SimpleCPU::CacheCompletionEvent::CacheCompletionEvent(SimpleCPU *_cpu)
    : Event(&mainEventQueue),
      cpu(_cpu)
{
}

void SimpleCPU::CacheCompletionEvent::process()
{
    cpu->processCacheCompletion();
}

const char *
SimpleCPU::CacheCompletionEvent::description()
{
    return "cache completion event";
}

#ifdef FULL_SYSTEM
SimpleCPU::SimpleCPU(const string &_name,
                     System *_system,
                     Counter max_insts_any_thread,
                     Counter max_insts_all_threads,
                     AlphaItb *itb, AlphaDtb *dtb,
                     FunctionalMemory *mem,
                     MemInterface *icache_interface,
                     MemInterface *dcache_interface,
                     int cpu_id, Tick freq)
    : BaseCPU(_name, /* number_of_threads */ 1,
              max_insts_any_thread, max_insts_all_threads,
              _system, cpu_id, freq),
#else
SimpleCPU::SimpleCPU(const string &_name, Process *_process,
                     Counter max_insts_any_thread,
                     Counter max_insts_all_threads,
                     MemInterface *icache_interface,
                     MemInterface *dcache_interface)
    : BaseCPU(_name, /* number_of_threads */ 1,
              max_insts_any_thread, max_insts_all_threads),
#endif
      tickEvent(this), xc(NULL), cacheCompletionEvent(this)
{
#ifdef FULL_SYSTEM
    xc = new ExecContext(this, 0, system, itb, dtb, mem, cpu_id);

    _status = Running;
    if (cpu_id != 0) {

       xc->setStatus(ExecContext::Unallocated);

       //Open a GDB debug session on port (7000 + the cpu_id)
       (new GDBListener(new RemoteGDB(system, xc), 7000 + cpu_id))->listen();

       AlphaISA::init(system->physmem, &xc->regs);

       fault = Reset_Fault;

       IntReg *ipr = xc->regs.ipr;
       ipr[TheISA::IPR_MCSR] = 0x6;

       AlphaISA::swap_palshadow(&xc->regs, true);

       xc->regs.pc =
           ipr[TheISA::IPR_PAL_BASE] + AlphaISA::fault_addr[fault];
       xc->regs.npc = xc->regs.pc + sizeof(MachInst);

       _status = Idle;
    }
    else {
      system->initBootContext(xc);

      // Reset the system
      //
      AlphaISA::init(system->physmem, &xc->regs);

      fault = Reset_Fault;

      IntReg *ipr = xc->regs.ipr;
      ipr[TheISA::IPR_MCSR] = 0x6;

      AlphaISA::swap_palshadow(&xc->regs, true);

      xc->regs.pc = ipr[TheISA::IPR_PAL_BASE] + AlphaISA::fault_addr[fault];
      xc->regs.npc = xc->regs.pc + sizeof(MachInst);

       _status = Running;
       tickEvent.schedule(0);
    }

#else
    xc = new ExecContext(this, /* thread_num */ 0, _process, /* asid */ 0);
    fault = No_Fault;
    if (xc->status() == ExecContext::Active) {
        _status = Running;
       tickEvent.schedule(0);
    } else
        _status = Idle;
#endif // !FULL_SYSTEM

    icacheInterface = icache_interface;
    dcacheInterface = dcache_interface;

    memReq = new MemReq();
    memReq->xc = xc;
    memReq->asid = 0;

    numInst = 0;
    last_idle = 0;
    lastIcacheStall = 0;
    lastDcacheStall = 0;

    contexts.push_back(xc);
}

SimpleCPU::~SimpleCPU()
{
}

void
SimpleCPU::regStats()
{
    BaseCPU::regStats();

    numInsts
        .name(name() + ".num_insts")
        .desc("Number of instructions executed")
        ;

    numMemRefs
        .name(name() + ".num_refs")
        .desc("Number of memory references")
        ;

    idleCycles
        .name(name() + ".idle_cycles")
        .desc("Number of idle cycles")
        ;

    idleFraction
        .name(name() + ".idle_fraction")
        .desc("Percentage of idle cycles")
        ;

    icacheStallCycles
        .name(name() + ".icache_stall_cycles")
        .desc("ICache total stall cycles")
        .prereq(icacheStallCycles)
        ;

    dcacheStallCycles
        .name(name() + ".dcache_stall_cycles")
        .desc("DCache total stall cycles")
        .prereq(dcacheStallCycles)
        ;

    idleFraction = idleCycles / simTicks;

    numInsts = Statistics::scalar(numInst);
    simInsts += numInsts;
}

void
SimpleCPU::serialize()
{
    nameOut();

#ifdef FULL_SYSTEM
#if 0
    // do we need this anymore?? egh
    childOut("itb", xc->itb);
    childOut("dtb", xc->dtb);
    childOut("physmem", physmem);
#endif
#endif

    for (int i = 0; i < NumIntRegs; i++) {
        stringstream buf;
        ccprintf(buf, "R%02d", i);
        paramOut(buf.str(), xc->regs.intRegFile[i]);
    }
    for (int i = 0; i < NumFloatRegs; i++) {
        stringstream buf;
        ccprintf(buf, "F%02d", i);
        paramOut(buf.str(), xc->regs.floatRegFile.d[i]);
    }
    // CPUTraitsType::serializeSpecialRegs(getProxy(), xc->regs);
}

void
SimpleCPU::unserialize(IniFile &db, const string &category, ConfigNode *node)
{
    string data;

    for (int i = 0; i < NumIntRegs; i++) {
        stringstream buf;
        ccprintf(buf, "R%02d", i);
        db.findDefault(category, buf.str(), data);
        to_number(data,xc->regs.intRegFile[i]);
    }
    for (int i = 0; i < NumFloatRegs; i++) {
        stringstream buf;
        ccprintf(buf, "F%02d", i);
        db.findDefault(category, buf.str(), data);
        xc->regs.floatRegFile.d[i] = strtod(data.c_str(),NULL);
    }

    // Read in Special registers

    // CPUTraitsType::unserializeSpecialRegs(db,category,node,xc->regs);
}

void
change_thread_state(int thread_number, int activate, int priority)
{
}

// precise architected memory state accessor macros
template <class T>
Fault
SimpleCPU::read(Addr addr, T& data, unsigned flags)
{
    memReq->reset(addr, sizeof(T), flags);

    // translate to physical address
    Fault fault = xc->translateDataReadReq(memReq);

    // do functional access
    if (fault == No_Fault)
        fault = xc->read(memReq, data);

    if (traceData) {
        traceData->setAddr(addr);
        if (fault == No_Fault)
            traceData->setData(data);
    }

    // if we have a cache, do cache access too
    if (fault == No_Fault && dcacheInterface) {
        memReq->cmd = Read;
        memReq->completionEvent = NULL;
        memReq->time = curTick;
        memReq->flags &= ~UNCACHEABLE;
        MemAccessResult result = dcacheInterface->access(memReq);

        // Ugly hack to get an event scheduled *only* if the access is
        // a miss.  We really should add first-class support for this
        // at some point.
        if (result != MA_HIT && dcacheInterface->doEvents) {
            memReq->completionEvent = &cacheCompletionEvent;
            setStatus(DcacheMissStall);
        }
    }

    return fault;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

template
Fault
SimpleCPU::read(Addr addr, uint64_t& data, unsigned flags);

template
Fault
SimpleCPU::read(Addr addr, uint32_t& data, unsigned flags);

template
Fault
SimpleCPU::read(Addr addr, uint16_t& data, unsigned flags);

template
Fault
SimpleCPU::read(Addr addr, uint8_t& data, unsigned flags);

#endif //DOXYGEN_SHOULD_SKIP_THIS

template<>
Fault
SimpleCPU::read(Addr addr, double& data, unsigned flags)
{
    return read(addr, *(uint64_t*)&data, flags);
}

template<>
Fault
SimpleCPU::read(Addr addr, float& data, unsigned flags)
{
    return read(addr, *(uint32_t*)&data, flags);
}


template<>
Fault
SimpleCPU::read(Addr addr, int32_t& data, unsigned flags)
{
    return read(addr, (uint32_t&)data, flags);
}


template <class T>
Fault
SimpleCPU::write(T data, Addr addr, unsigned flags, uint64_t *res)
{
    if (traceData) {
        traceData->setAddr(addr);
        traceData->setData(data);
    }

    memReq->reset(addr, sizeof(T), flags);

    // translate to physical address
    Fault fault = xc->translateDataWriteReq(memReq);

    // do functional access
    if (fault == No_Fault)
        fault = xc->write(memReq, data);

    if (fault == No_Fault && dcacheInterface) {
        memReq->cmd = Write;
        memReq->data = (uint8_t *)&data;
        memReq->completionEvent = NULL;
        memReq->time = curTick;
        memReq->flags &= ~UNCACHEABLE;
        MemAccessResult result = dcacheInterface->access(memReq);

        // Ugly hack to get an event scheduled *only* if the access is
        // a miss.  We really should add first-class support for this
        // at some point.
        if (result != MA_HIT && dcacheInterface->doEvents) {
            memReq->completionEvent = &cacheCompletionEvent;
            setStatus(DcacheMissStall);
        }
    }

    if (res && (fault == No_Fault))
        *res = memReq->result;

    return fault;
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS
template
Fault
SimpleCPU::write(uint64_t data, Addr addr, unsigned flags, uint64_t *res);

template
Fault
SimpleCPU::write(uint32_t data, Addr addr, unsigned flags, uint64_t *res);

template
Fault
SimpleCPU::write(uint16_t data, Addr addr, unsigned flags, uint64_t *res);

template
Fault
SimpleCPU::write(uint8_t data, Addr addr, unsigned flags, uint64_t *res);

#endif //DOXYGEN_SHOULD_SKIP_THIS

template<>
Fault
SimpleCPU::write(double data, Addr addr, unsigned flags, uint64_t *res)
{
    return write(*(uint64_t*)&data, addr, flags, res);
}

template<>
Fault
SimpleCPU::write(float data, Addr addr, unsigned flags, uint64_t *res)
{
    return write(*(uint32_t*)&data, addr, flags, res);
}


template<>
Fault
SimpleCPU::write(int32_t data, Addr addr, unsigned flags, uint64_t *res)
{
    return write((uint32_t)data, addr, flags, res);
}


#ifdef FULL_SYSTEM
Addr
SimpleCPU::dbg_vtophys(Addr addr)
{
    return vtophys(xc, addr);
}
#endif // FULL_SYSTEM

Tick save_cycle = 0;


void
SimpleCPU::processCacheCompletion()
{
    switch (status()) {
      case IcacheMissStall:
        icacheStallCycles += curTick - lastIcacheStall;
        setStatus(IcacheMissComplete);
        break;
      case DcacheMissStall:
        dcacheStallCycles += curTick - lastDcacheStall;
        setStatus(Running);
        break;
      default:
        panic("SimpleCPU::processCacheCompletion: bad state");
        break;
    }
}

#ifdef FULL_SYSTEM
void
SimpleCPU::post_interrupt(int int_num, int index)
{
    BaseCPU::post_interrupt(int_num, index);

    if (xc->status() == ExecContext::Suspended) {
                DPRINTF(IPI,"Suspended Processor awoke\n");
        xc->setStatus(ExecContext::Active);
        Annotate::Resume(xc);
    }
}
#endif // FULL_SYSTEM

/* start simulation, program loaded, processor precise state initialized */
void
SimpleCPU::tick()
{
    traceData = NULL;

#ifdef FULL_SYSTEM
    if (fault == No_Fault && AlphaISA::check_interrupts &&
        xc->cpu->check_interrupts() &&
        !PC_PAL(xc->regs.pc) &&
        status() != IcacheMissComplete) {
        int ipl = 0;
        int summary = 0;
        AlphaISA::check_interrupts = 0;
        IntReg *ipr = xc->regs.ipr;

        if (xc->regs.ipr[TheISA::IPR_SIRR]) {
            for (int i = TheISA::INTLEVEL_SOFTWARE_MIN;
                 i < TheISA::INTLEVEL_SOFTWARE_MAX; i++) {
                if (ipr[TheISA::IPR_SIRR] & (ULL(1) << i)) {
                    // See table 4-19 of 21164 hardware reference
                    ipl = (i - TheISA::INTLEVEL_SOFTWARE_MIN) + 1;
                    summary |= (ULL(1) << i);
                }
            }
        }

        uint64_t interrupts = xc->cpu->intr_status();
        for(int i = TheISA::INTLEVEL_EXTERNAL_MIN;
            i < TheISA::INTLEVEL_EXTERNAL_MAX; i++) {
            if (interrupts & (ULL(1) << i)) {
                // See table 4-19 of 21164 hardware reference
                ipl = i;
                summary |= (ULL(1) << i);
            }
        }

        if (ipr[TheISA::IPR_ASTRR])
            panic("asynchronous traps not implemented\n");

        if (ipl && ipl > xc->regs.ipr[TheISA::IPR_IPLR]) {
            ipr[TheISA::IPR_ISR] = summary;
            ipr[TheISA::IPR_INTID] = ipl;
            xc->ev5_trap(Interrupt_Fault);

            DPRINTF(Flow, "Interrupt! IPLR=%d ipl=%d summary=%x\n",
                    ipr[TheISA::IPR_IPLR], ipl, summary);
        }
    }
#endif

    // maintain $r0 semantics
    xc->regs.intRegFile[ZeroReg] = 0;
#ifdef TARGET_ALPHA
    xc->regs.floatRegFile.d[ZeroReg] = 0.0;
#endif // TARGET_ALPHA

    if (status() == IcacheMissComplete) {
        // We've already fetched an instruction and were stalled on an
        // I-cache miss.  No need to fetch it again.

        setStatus(Running);
    }
    else {
        // Try to fetch an instruction

        // set up memory request for instruction fetch
#ifdef FULL_SYSTEM
#define IFETCH_FLAGS(pc)	((pc) & 1) ? PHYSICAL : 0
#else
#define IFETCH_FLAGS(pc)	0
#endif

        memReq->cmd = Read;
        memReq->reset(xc->regs.pc & ~3, sizeof(uint32_t),
                     IFETCH_FLAGS(xc->regs.pc));

        fault = xc->translateInstReq(memReq);

        if (fault == No_Fault)
            fault = xc->mem->read(memReq, inst);

        if (icacheInterface && fault == No_Fault) {
            memReq->completionEvent = NULL;

            memReq->time = curTick;
            memReq->flags &= ~UNCACHEABLE;
            MemAccessResult result = icacheInterface->access(memReq);

            // Ugly hack to get an event scheduled *only* if the access is
            // a miss.  We really should add first-class support for this
            // at some point.
            if (result != MA_HIT && icacheInterface->doEvents) {
                memReq->completionEvent = &cacheCompletionEvent;
                setStatus(IcacheMissStall);
                return;
            }
        }
    }

    // If we've got a valid instruction (i.e., no fault on instruction
    // fetch), then execute it.
    if (fault == No_Fault) {

        // keep an instruction count
        numInst++;

        // check for instruction-count-based events
        comInsnEventQueue[0]->serviceEvents(numInst);

        // decode the instruction
        StaticInstPtr<TheISA> si(inst);

        traceData = Trace::getInstRecord(curTick, xc, this, si,
                                         xc->regs.pc);

#ifdef FULL_SYSTEM
        xc->regs.opcode = (inst >> 26) & 0x3f;
        xc->regs.ra = (inst >> 21) & 0x1f;
#endif // FULL_SYSTEM

        xc->func_exe_insn++;

        fault = si->execute(this, xc, traceData);

        if (si->isMemRef()) {
            numMemRefs++;
        }

        if (traceData)
            traceData->finalize();

    }	// if (fault == No_Fault)

    if (fault != No_Fault) {
#ifdef FULL_SYSTEM
        xc->ev5_trap(fault);
#else // !FULL_SYSTEM
        fatal("fault (%d) detected @ PC 0x%08p", fault, xc->regs.pc);
#endif // FULL_SYSTEM
    }
    else {
        // go to the next instruction
        xc->regs.pc = xc->regs.npc;
        xc->regs.npc += sizeof(MachInst);
    }

#ifdef FULL_SYSTEM
    Addr oldpc;
    do {
        oldpc = xc->regs.pc;
        system->pcEventQueue.service(xc);
    } while (oldpc != xc->regs.pc);
#endif

    assert(status() == Running ||
           status() == Idle ||
           status() == DcacheMissStall);

    if (status() == Running && !tickEvent.scheduled())
        tickEvent.schedule(curTick + 1);
}


////////////////////////////////////////////////////////////////////////
//
//  SimpleCPU Simulation Object
//
BEGIN_DECLARE_SIM_OBJECT_PARAMS(SimpleCPU)

    Param<Counter> max_insts_any_thread;
    Param<Counter> max_insts_all_threads;

#ifdef FULL_SYSTEM
    SimObjectParam<AlphaItb *> itb;
    SimObjectParam<AlphaDtb *> dtb;
    SimObjectParam<FunctionalMemory *> mem;
    SimObjectParam<System *> system;
    Param<int> cpu_id;
    Param<int> mult;
#else
    SimObjectParam<Process *> workload;
#endif // FULL_SYSTEM

    SimObjectParam<BaseMem *> icache;
    SimObjectParam<BaseMem *> dcache;

END_DECLARE_SIM_OBJECT_PARAMS(SimpleCPU)

BEGIN_INIT_SIM_OBJECT_PARAMS(SimpleCPU)

    INIT_PARAM_DFLT(max_insts_any_thread,
                    "terminate when any thread reaches this insn count",
                    0),
    INIT_PARAM_DFLT(max_insts_all_threads,
                    "terminate when all threads have reached this insn count",
                    0),

#ifdef FULL_SYSTEM
    INIT_PARAM(itb, "Instruction TLB"),
    INIT_PARAM(dtb, "Data TLB"),
    INIT_PARAM(mem, "memory"),
    INIT_PARAM(system, "system object"),
    INIT_PARAM_DFLT(cpu_id, "CPU identification number", 0),
    INIT_PARAM_DFLT(mult, "system clock multiplier", 1),
#else
    INIT_PARAM(workload, "processes to run"),
#endif // FULL_SYSTEM

    INIT_PARAM_DFLT(icache, "L1 instruction cache object", NULL),
    INIT_PARAM_DFLT(dcache, "L1 data cache object", NULL)

END_INIT_SIM_OBJECT_PARAMS(SimpleCPU)


CREATE_SIM_OBJECT(SimpleCPU)
{
#ifdef FULL_SYSTEM
    if (mult != 1)
        panic("processor clock multiplier must be 1\n");

    return new SimpleCPU(getInstanceName(), system,
                         max_insts_any_thread, max_insts_all_threads,
                         itb, dtb, mem,
                         (icache) ? icache->getInterface() : NULL,
                         (dcache) ? dcache->getInterface() : NULL,
                         cpu_id, ticksPerSecond * mult);
#else

    return new SimpleCPU(getInstanceName(), workload,
                         max_insts_any_thread, max_insts_all_threads,
                         icache->getInterface(), dcache->getInterface());

#endif // FULL_SYSTEM
}

REGISTER_SIM_OBJECT("SimpleCPU", SimpleCPU)
