/*
 * Copyright (c) 2003-2005 The Regents of The University of Michigan
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
 *
 * Authors: Gabe Black
 *          Ali Saidi
 */

#include "arch/sparc/miscregfile.hh"
#include "base/trace.hh"
#include "config/full_system.hh"
#include "cpu/base.hh"
#include "cpu/thread_context.hh"

#if FULL_SYSTEM
#include "arch/sparc/system.hh"
#endif

using namespace SparcISA;
using namespace std;

class Checkpoint;

//These functions map register indices to names
string SparcISA::getMiscRegName(RegIndex index)
{
    static::string miscRegName[NumMiscRegs] =
        {"y", "ccr", "asi", "tick", "pc", "fprs", "pcr", "pic",
         "gsr", "softint_set", "softint_clr", "softint", "tick_cmpr",
         "stick", "stick_cmpr",
         "tpc", "tnpc", "tstate", "tt", "privtick", "tba", "pstate", "tl",
         "pil", "cwp", "cansave", "canrestore", "cleanwin", "otherwin",
         "wstate", "gl",
         "hpstate", "htstate", "hintp", "htba", "hver", "strand_sts_reg",
         "hstick_cmpr",
         "fsr"};
    return miscRegName[index];
}

void MiscRegFile::reset()
{
}

MiscReg MiscRegFile::readReg(int miscReg)
{
    switch (miscReg) {
        case MISCREG_Y:
          return y;
        case MISCREG_CCR:
          return ccr;
        case MISCREG_ASI:
          return asi;
        case MISCREG_FPRS:
          return fprs;
        case MISCREG_TICK:
           return tick;
        case MISCREG_PCR:
        case MISCREG_PIC:
          panic("ASR number %d not implemented\n", miscReg - AsrStart);
        case MISCREG_GSR:
          return gsr;
        case MISCREG_SOFTINT:
           return softint;
        case MISCREG_TICK_CMPR:
           return tick_cmpr;
        case MISCREG_STICK:
           return stick;
        case MISCREG_STICK_CMPR:
           return stick_cmpr;

        /** Privilged Registers */
        case MISCREG_TPC:
          return tpc[tl-1];
        case MISCREG_TNPC:
          return tnpc[tl-1];
        case MISCREG_TSTATE:
          return tstate[tl-1];
        case MISCREG_TT:
          return tt[tl-1];
        case MISCREG_PRIVTICK:
          panic("Priviliged access to tick registers not implemented\n");
        case MISCREG_TBA:
          return tba;
        case MISCREG_PSTATE:
          return pstate;
        case MISCREG_TL:
          return tl;
        case MISCREG_PIL:
          return pil;
        case MISCREG_CWP:
          return cwp;
        case MISCREG_CANSAVE:
          return cansave;
        case MISCREG_CANRESTORE:
          return canrestore;
        case MISCREG_CLEANWIN:
          return cleanwin;
        case MISCREG_OTHERWIN:
          return otherwin;
        case MISCREG_WSTATE:
          return wstate;
        case MISCREG_GL:
          return gl;

        /** Hyper privileged registers */
        case MISCREG_HPSTATE:
          return hpstate;
        case MISCREG_HTSTATE:
          return htstate[tl-1];
        case MISCREG_HINTP:
          panic("HINTP not implemented\n");
        case MISCREG_HTBA:
          return htba;
        case MISCREG_HVER:
          return NWindows | MaxTL << 8 | MaxGL << 16;
        case MISCREG_STRAND_STS_REG:
          return strandStatusReg;
        case MISCREG_HSTICK_CMPR:
          return hstick_cmpr;

        /** Floating Point Status Register */
        case MISCREG_FSR:
          return fsr;
        default:
          panic("Miscellaneous register %d not implemented\n", miscReg);
    }
}

MiscReg MiscRegFile::readRegWithEffect(int miscReg, ThreadContext * tc)
{
    switch (miscReg) {
        case MISCREG_TICK:
        case MISCREG_PRIVTICK:
          return tc->getCpuPtr()->curCycle() - tickFields.counter |
              tickFields.npt << 63;
        case MISCREG_FPRS:
          panic("FPU not implemented\n");
        case MISCREG_PCR:
        case MISCREG_PIC:
          panic("Performance Instrumentation not impl\n");
        /** Floating Point Status Register */
        case MISCREG_FSR:
          panic("Floating Point not implemented\n");
//We'll include this only in FS so we don't need the SparcSystem type around
//in SE.
#if FULL_SYSTEM
        case MISCREG_STICK:
          SparcSystem *sys;
          sys = dynamic_cast<SparcSystem*>(tc->getSystemPtr());
          assert(sys != NULL);
          return curTick/Clock::Int::ns - sys->sysTick | stickFields.npt << 63;
#endif
        case MISCREG_HVER:
          return NWindows | MaxTL << 8 | MaxGL << 16;
    }
    return readReg(miscReg);
}

void MiscRegFile::setReg(int miscReg, const MiscReg &val)
{
    switch (miscReg) {
        case MISCREG_Y:
          y = val;
          break;
        case MISCREG_CCR:
          ccr = val;
          break;
        case MISCREG_ASI:
          asi = val;
          break;
        case MISCREG_FPRS:
          fprs = val;
          break;
        case MISCREG_TICK:
          tick = val;
          break;
        case MISCREG_PCR:
        case MISCREG_PIC:
          panic("ASR number %d not implemented\n", miscReg - AsrStart);
        case MISCREG_GSR:
          gsr = val;
          break;
        case MISCREG_SOFTINT:
          softint = val;
          break;
        case MISCREG_TICK_CMPR:
          tick_cmpr = val;
          break;
        case MISCREG_STICK:
          stick = val;
          break;
        case MISCREG_STICK_CMPR:
          stick_cmpr = val;
          break;

        /** Privilged Registers */
        case MISCREG_TPC:
          tpc[tl-1] = val;
          break;
        case MISCREG_TNPC:
          tnpc[tl-1] = val;
          break;
        case MISCREG_TSTATE:
          tstate[tl-1] = val;
          break;
        case MISCREG_TT:
          tt[tl-1] = val;
          break;
        case MISCREG_PRIVTICK:
          panic("Priviliged access to tick regesiters not implemented\n");
        case MISCREG_TBA:
          // clear lower 7 bits on writes.
          tba = val & ULL(~0x7FFF);
          break;
        case MISCREG_PSTATE:
          pstate = val;
          break;
        case MISCREG_TL:
          tl = val;
          break;
        case MISCREG_PIL:
          pil = val;
          break;
        case MISCREG_CWP:
          cwp = val;
          break;
        case MISCREG_CANSAVE:
          cansave = val;
          break;
        case MISCREG_CANRESTORE:
          canrestore = val;
          break;
        case MISCREG_CLEANWIN:
          cleanwin = val;
          break;
        case MISCREG_OTHERWIN:
          otherwin = val;
          break;
        case MISCREG_WSTATE:
          wstate = val;
          break;
        case MISCREG_GL:
          gl = val;
          break;

        /** Hyper privileged registers */
        case MISCREG_HPSTATE:
          hpstate = val;
          break;
        case MISCREG_HTSTATE:
          htstate[tl-1] = val;
          break;
        case MISCREG_HINTP:
          panic("HINTP not implemented\n");
        case MISCREG_HTBA:
          htba = val;
          break;
        case MISCREG_STRAND_STS_REG:
          strandStatusReg = val;
          break;
        case MISCREG_HSTICK_CMPR:
          hstick_cmpr = val;
          break;

        /** Floating Point Status Register */
        case MISCREG_FSR:
          fsr = val;
          break;
        default:
          panic("Miscellaneous register %d not implemented\n", miscReg);
    }
}

inline void MiscRegFile::setImplicitAsis()
{
    //The spec seems to use trap level to indicate the privilege level of the
    //processor. It's unclear whether the implicit ASIs should directly depend
    //on the trap level, or if they should really be based on the privelege
    //bits
    if(tl == 0)
    {
        implicitInstAsi = implicitDataAsi =
            pstateFields.cle ? ASI_PRIMARY_LITTLE : ASI_PRIMARY;
    }
    else if(tl <= MaxPTL)
    {
        implicitInstAsi = ASI_NUCLEUS;
        implicitDataAsi = pstateFields.cle ? ASI_NUCLEUS_LITTLE : ASI_NUCLEUS;
    }
    else
    {
        //This is supposed to force physical addresses to match the spec.
        //It might not because of context values and partition values.
        implicitInstAsi = implicitDataAsi = ASI_REAL;
    }
}

void MiscRegFile::setRegWithEffect(int miscReg,
        const MiscReg &val, ThreadContext * tc)
{
    const uint64_t Bit64 = (1ULL << 63);
#if FULL_SYSTEM
    uint64_t time;
    SparcSystem *sys;
#endif
    switch (miscReg) {
        case MISCREG_TICK:
          tickFields.counter = tc->getCpuPtr()->curCycle() - val  & ~Bit64;
          tickFields.npt = val & Bit64 ? 1 : 0;
          break;
        case MISCREG_FPRS:
          //Configure the fpu based on the fprs
          break;
        case MISCREG_PCR:
          //Set up performance counting based on pcr value
          break;
        case MISCREG_PSTATE:
          pstate = val;
          setImplicitAsis();
          return;
        case MISCREG_TL:
          tl = val;
          setImplicitAsis();
          return;
        case MISCREG_CWP:
          tc->changeRegFileContext(CONTEXT_CWP, val);
          break;
        case MISCREG_GL:
          tc->changeRegFileContext(CONTEXT_GLOBALS, val);
          break;
        case MISCREG_SOFTINT:
          //We need to inject interrupts, and or notify the interrupt
          //object that it needs to use a different interrupt level.
          //Any newly appropriate interrupts will happen when the cpu gets
          //around to checking for them. This might not be quite what we
          //want.
          break;
        case MISCREG_SOFTINT_CLR:
          //Do whatever this is supposed to do...
          break;
        case MISCREG_SOFTINT_SET:
          //Do whatever this is supposed to do...
          break;
#if FULL_SYSTEM
        case MISCREG_TICK_CMPR:
          if (tickCompare == NULL)
              tickCompare = new TickCompareEvent(this, tc);
          setReg(miscReg, val);
          if (tick_cmprFields.int_dis && tickCompare->scheduled())
                  tickCompare->deschedule();
          time = tick_cmprFields.tick_cmpr - tickFields.counter;
          if (!tick_cmprFields.int_dis && time > 0)
              tickCompare->schedule(time * tc->getCpuPtr()->cycles(1));
          break;
#endif
        case MISCREG_PIL:
          //We need to inject interrupts, and or notify the interrupt
          //object that it needs to use a different interrupt level.
          //Any newly appropriate interrupts will happen when the cpu gets
          //around to checking for them. This might not be quite what we
          //want.
          break;
//We'll include this only in FS so we don't need the SparcSystem type around
//in SE.
#if FULL_SYSTEM
        case MISCREG_STICK:
          sys = dynamic_cast<SparcSystem*>(tc->getSystemPtr());
          assert(sys != NULL);
          sys->sysTick = curTick/Clock::Int::ns - val & ~Bit64;
          stickFields.npt = val & Bit64 ? 1 : 0;
          break;
        case MISCREG_STICK_CMPR:
          if (sTickCompare == NULL)
              sTickCompare = new STickCompareEvent(this, tc);
          sys = dynamic_cast<SparcSystem*>(tc->getSystemPtr());
          assert(sys != NULL);
          if (stick_cmprFields.int_dis && sTickCompare->scheduled())
                  sTickCompare->deschedule();
          time = stick_cmprFields.tick_cmpr - sys->sysTick;
          if (!stick_cmprFields.int_dis && time > 0)
              sTickCompare->schedule(time * Clock::Int::ns);
          break;
        case MISCREG_HSTICK_CMPR:
          if (hSTickCompare == NULL)
              hSTickCompare = new HSTickCompareEvent(this, tc);
          sys = dynamic_cast<SparcSystem*>(tc->getSystemPtr());
          assert(sys != NULL);
          if (hstick_cmprFields.int_dis && hSTickCompare->scheduled())
                  hSTickCompare->deschedule();
          int64_t time = hstick_cmprFields.tick_cmpr - sys->sysTick;
          if (!hstick_cmprFields.int_dis && time > 0)
              hSTickCompare->schedule(time * Clock::Int::ns);
          break;
#endif
    }
    setReg(miscReg, val);
}

void MiscRegFile::serialize(std::ostream & os)
{
    SERIALIZE_SCALAR(pstate);
    SERIALIZE_SCALAR(tba);
    SERIALIZE_SCALAR(y);
    SERIALIZE_SCALAR(pil);
    SERIALIZE_SCALAR(gl);
    SERIALIZE_SCALAR(cwp);
    SERIALIZE_ARRAY(tt, MaxTL);
    SERIALIZE_SCALAR(ccr);
    SERIALIZE_SCALAR(asi);
    SERIALIZE_SCALAR(tl);
    SERIALIZE_ARRAY(tpc, MaxTL);
    SERIALIZE_ARRAY(tnpc, MaxTL);
    SERIALIZE_ARRAY(tstate, MaxTL);
    SERIALIZE_SCALAR(tick);
    SERIALIZE_SCALAR(cansave);
    SERIALIZE_SCALAR(canrestore);
    SERIALIZE_SCALAR(otherwin);
    SERIALIZE_SCALAR(cleanwin);
    SERIALIZE_SCALAR(wstate);
    SERIALIZE_SCALAR(fsr);
    SERIALIZE_SCALAR(fprs);
    SERIALIZE_SCALAR(hpstate);
    SERIALIZE_ARRAY(htstate, MaxTL);
    SERIALIZE_SCALAR(htba);
    SERIALIZE_SCALAR(hstick_cmpr);
    SERIALIZE_SCALAR((int)implicitInstAsi);
    SERIALIZE_SCALAR((int)implicitDataAsi);
}

void MiscRegFile::unserialize(Checkpoint * cp, const std::string & section)
{
    UNSERIALIZE_SCALAR(pstate);
    UNSERIALIZE_SCALAR(tba);
    UNSERIALIZE_SCALAR(y);
    UNSERIALIZE_SCALAR(pil);
    UNSERIALIZE_SCALAR(gl);
    UNSERIALIZE_SCALAR(cwp);
    UNSERIALIZE_ARRAY(tt, MaxTL);
    UNSERIALIZE_SCALAR(ccr);
    UNSERIALIZE_SCALAR(asi);
    UNSERIALIZE_SCALAR(tl);
    UNSERIALIZE_ARRAY(tpc, MaxTL);
    UNSERIALIZE_ARRAY(tnpc, MaxTL);
    UNSERIALIZE_ARRAY(tstate, MaxTL);
    UNSERIALIZE_SCALAR(tick);
    UNSERIALIZE_SCALAR(cansave);
    UNSERIALIZE_SCALAR(canrestore);
    UNSERIALIZE_SCALAR(otherwin);
    UNSERIALIZE_SCALAR(cleanwin);
    UNSERIALIZE_SCALAR(wstate);
    UNSERIALIZE_SCALAR(fsr);
    UNSERIALIZE_SCALAR(fprs);
    UNSERIALIZE_SCALAR(hpstate);
    UNSERIALIZE_ARRAY(htstate, MaxTL);
    UNSERIALIZE_SCALAR(htba);
    UNSERIALIZE_SCALAR(hstick_cmpr);
    int temp;
    UNSERIALIZE_SCALAR(temp);
    implicitInstAsi = (ASI)temp;
    UNSERIALIZE_SCALAR(temp);
    implicitDataAsi = (ASI)temp;
}

#if FULL_SYSTEM
void
MiscRegFile::processTickCompare(ThreadContext *tc)
{
    panic("tick compare not implemented\n");
}

void
MiscRegFile::processSTickCompare(ThreadContext *tc)
{
    panic("tick compare not implemented\n");
}

void
MiscRegFile::processHSTickCompare(ThreadContext *tc)
{
    panic("tick compare not implemented\n");
}
#endif
