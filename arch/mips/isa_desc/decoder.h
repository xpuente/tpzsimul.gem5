////////////////////////////////////////////////////////////////////
//
// The actual MIPS32 ISA decoder
// -----------------------------
// The following instructions are specified in the MIPS32 ISA
// Specification. Decoding closely follows the style specified
// in the MIPS32 ISAthe specification document starting with Table
// A-2 (document available @ www.mips.com)
//
//
decode OPCODE_HI default FailUnimpl::unknown() {

    // Derived From ... Table A-2 MIPS32 ISA Manual
    0x0: decode OPCODE_LO {

        0x0: decode SPECIAL {
            0x0:;
            0x1:;
            0x2:;
            0x3:;
            0x4:;
            0x5:;
            0x6:;
        }

        0x1: decode REGIMM {
            0x0:;
            0x1:;
            0x2:;
            0x3:;
            0x4:;
            0x5:;
            0x6:;
        }

        format Jump {
            0x2: j({{ }});
            0x3: jal({{ }});
        }

        format Branch {
            0x4: beq({{ }});
            0x5: bne({{ }});
            0x6: blez({{ }});
            0x7: bgtz({{ }});
        }
    };

    0x1: decode OPCODE_LO {
        format IntImmediate {
            0x0: addi({{ }});
            0x1: addiu({{ }});
            0x2: slti({{ }});
            0x3: sltiu({{ }});
            0x4: andi({{ }});
            0x5: ori({{ }});
            0x6: xori({{ }});
            0x7: lui({{ }});
        };
    };

    0x2: decode OPCODE_LO {
        format FailUnimpl{
            0x0: coprocessor_op({{ }});
            0x1: coprocessor_op({{ }});
            0x2: coprocessor_op({{ }});
            0x3: coprocessor_op({{ }});
        };

        //MIPS obsolete instructions
        0x4: beql({{ }});
        0x5: bnel({{ }});
        0x6: blezl({{ }});
        0x7: bgtzl({{ }});
    };

    0x3: decode OPCODE_LO {
        format FailUnimpl{
            0x0: reserved({{ }})
            0x1: reserved({{ }})
            0x2: reserved({{ }})
            0x3: reserved({{ }})
            0x5: reserved({{ }})
            0x6: reserved({{ }})
        };

        4: decode SPECIAL2 {
            0x0:;
            0x1:;
            0x2:;
            0x3:;
            0x4:;
            0x5:;
            0x6:;
        }

        7: decode SPECIAL3 {
            0x0:;
            0x1:;
            0x2:;
            0x3:;
            0x4:;
            0x5:;
            0x6:;
        }
    };

    0x4: decode OPCODE_LO {
        format LoadMemory{
            0x0: lb({{ }});
            0x1: lh({{ }});
            0x2: lwl({{ }});
            0x3: lw({{ }});
            0x4: lbu({{ }});
            0x5: lhu({{ }});
            0x6: lhu({{ }});
        };

        0x7: FailUnimpl::reserved({{ }});
    };

    0x5: decode OPCODE_LO {
        format StoreMemory{
            0x0: sb({{ }});
            0x1: sh({{ }});
            0x2: swl({{ }});
            0x3: sw({{ }});
            0x6: swr({{ }});
        };

        format FailUnimpl{
            0x4: reserved({{ }});
            0x5: reserved({{ }});
            0x2: cache({{ }});
        };

    };

    0x6: decode OPCODE_LO {
        format LoadMemory{
            0x0: ll({{ }});
            0x1: lwc1({{ }});
            0x5: ldc1({{ }});
        };

        format FailUnimpl{
            0x2: lwc2({{ }});
            0x3: pref({{ }});
            0x4: reserved({{ }});
            0x6: ldc2({{ }});
            0x7: reserved({{ }});
        };

    };

    0x7: decode OPCODE_LO {
        format StoreMemory{
            0x0: sc({{ }});
            0x1: swc1({{ }});
            0x5: sdc1({{ }});
        };

        format FailUnimpl{
            0x2: swc2({{ }});
            0x3: reserved({{ }});
            0x4: reserved({{ }});
            0x6: sdc2({{ }});
            0x7: reserved({{ }});
        };

    };


    //Table 3-1 CPU Arithmetic Instructions ( )
    format IntegerOperate {

        0x10: decode INTFUNC {	// integer arithmetic operations

           //ADD Add Word

           //ADDI Add Immediate Word

           //ADDIU Add Immediate Unsigned Word

           //ADDU Add Unsigned Word

            0x00: addl({{ Rc.sl = Ra.sl + Rb_or_imm.sl; }});
            0x40: addlv({{
                uint32_t tmp  = Ra.sl + Rb_or_imm.sl;
                // signed overflow occurs when operands have same sign
                // and sign of result does not match.
                if (Ra.sl<31:> == Rb_or_imm.sl<31:> && tmp<31:> != Ra.sl<31:>)
                    fault = Integer_Overflow_Fault;
                Rc.sl = tmp;
            }});
            0x02: s4addl({{ Rc.sl = (Ra.sl << 2) + Rb_or_imm.sl; }});
            0x12: s8addl({{ Rc.sl = (Ra.sl << 3) + Rb_or_imm.sl; }});

            0x20: addq({{ Rc = Ra + Rb_or_imm; }});
            0x60: addqv({{
                uint64_t tmp = Ra + Rb_or_imm;
                // signed overflow occurs when operands have same sign
                // and sign of result does not match.
                if (Ra<63:> == Rb_or_imm<63:> && tmp<63:> != Ra<63:>)
                    fault = Integer_Overflow_Fault;
                Rc = tmp;
            }});
            0x22: s4addq({{ Rc = (Ra << 2) + Rb_or_imm; }});
            0x32: s8addq({{ Rc = (Ra << 3) + Rb_or_imm; }});

            0x09: subl({{ Rc.sl = Ra.sl - Rb_or_imm.sl; }});
            0x49: sublv({{
                uint32_t tmp  = Ra.sl - Rb_or_imm.sl;
                // signed overflow detection is same as for add,
                // except we need to look at the *complemented*
                // sign bit of the subtrahend (Rb), i.e., if the initial
                // signs are the *same* then no overflow can occur
                if (Ra.sl<31:> != Rb_or_imm.sl<31:> && tmp<31:> != Ra.sl<31:>)
                    fault = Integer_Overflow_Fault;
                Rc.sl = tmp;
            }});
            0x0b: s4subl({{ Rc.sl = (Ra.sl << 2) - Rb_or_imm.sl; }});
            0x1b: s8subl({{ Rc.sl = (Ra.sl << 3) - Rb_or_imm.sl; }});

            0x29: subq({{ Rc = Ra - Rb_or_imm; }});
            0x69: subqv({{
                uint64_t tmp  = Ra - Rb_or_imm;
                // signed overflow detection is same as for add,
                // except we need to look at the *complemented*
                // sign bit of the subtrahend (Rb), i.e., if the initial
                // signs are the *same* then no overflow can occur
                if (Ra<63:> != Rb_or_imm<63:> && tmp<63:> != Ra<63:>)
                    fault = Integer_Overflow_Fault;
                Rc = tmp;
            }});
            0x2b: s4subq({{ Rc = (Ra << 2) - Rb_or_imm; }});
            0x3b: s8subq({{ Rc = (Ra << 3) - Rb_or_imm; }});

            0x2d: cmpeq({{ Rc = (Ra == Rb_or_imm); }});
            0x6d: cmple({{ Rc = (Ra.sq <= Rb_or_imm.sq); }});
            0x4d: cmplt({{ Rc = (Ra.sq <  Rb_or_imm.sq); }});
            0x3d: cmpule({{ Rc = (Ra.uq <= Rb_or_imm.uq); }});
            0x1d: cmpult({{ Rc = (Ra.uq <  Rb_or_imm.uq); }});

            0x0f: cmpbge({{
                int hi = 7;
                int lo = 0;
                uint64_t tmp = 0;
                for (int i = 0; i < 8; ++i) {
                    tmp |= (Ra.uq<hi:lo> >= Rb_or_imm.uq<hi:lo>) << i;
                    hi += 8;
                    lo += 8;
                }
                Rc = tmp;
            }});
        }

        0x11: decode INTFUNC {	// integer logical operations

            0x00: and({{ Rc = Ra & Rb_or_imm; }});
            0x08: bic({{ Rc = Ra & ~Rb_or_imm; }});
            0x20: bis({{ Rc = Ra | Rb_or_imm; }});
            0x28: ornot({{ Rc = Ra | ~Rb_or_imm; }});
            0x40: xor({{ Rc = Ra ^ Rb_or_imm; }});
            0x48: eqv({{ Rc = Ra ^ ~Rb_or_imm; }});

            // conditional moves
            0x14: cmovlbs({{ Rc = ((Ra & 1) == 1) ? Rb_or_imm : Rc; }});
            0x16: cmovlbc({{ Rc = ((Ra & 1) == 0) ? Rb_or_imm : Rc; }});
            0x24: cmoveq({{ Rc = (Ra == 0) ? Rb_or_imm : Rc; }});
            0x26: cmovne({{ Rc = (Ra != 0) ? Rb_or_imm : Rc; }});
            0x44: cmovlt({{ Rc = (Ra.sq <  0) ? Rb_or_imm : Rc; }});
            0x46: cmovge({{ Rc = (Ra.sq >= 0) ? Rb_or_imm : Rc; }});
            0x64: cmovle({{ Rc = (Ra.sq <= 0) ? Rb_or_imm : Rc; }});
            0x66: cmovgt({{ Rc = (Ra.sq >  0) ? Rb_or_imm : Rc; }});

            // For AMASK, RA must be R31.
            0x61: decode RA {
                31: amask({{ Rc = Rb_or_imm & ~ULL(0x17); }});
            }

            // For IMPLVER, RA must be R31 and the B operand
            // must be the immediate value 1.
            0x6c: decode RA {
                31: decode IMM {
                    1: decode INTIMM {
                        // return EV5 for FULL_SYSTEM and EV6 otherwise
                        1: implver({{
#if FULL_SYSTEM
                             Rc = 1;
#else
                             Rc = 2;
#endif
                        }});
                    }
                }
            }

#if FULL_SYSTEM
            // The mysterious 11.25...
            0x25: WarnUnimpl::eleven25();
#endif
        }

        0x12: decode INTFUNC {
            0x39: sll({{ Rc = Ra << Rb_or_imm<5:0>; }});
            0x34: srl({{ Rc = Ra.uq >> Rb_or_imm<5:0>; }});
            0x3c: sra({{ Rc = Ra.sq >> Rb_or_imm<5:0>; }});

            0x02: mskbl({{ Rc = Ra & ~(mask( 8) << (Rb_or_imm<2:0> * 8)); }});
            0x12: mskwl({{ Rc = Ra & ~(mask(16) << (Rb_or_imm<2:0> * 8)); }});
            0x22: mskll({{ Rc = Ra & ~(mask(32) << (Rb_or_imm<2:0> * 8)); }});
            0x32: mskql({{ Rc = Ra & ~(mask(64) << (Rb_or_imm<2:0> * 8)); }});

            0x52: mskwh({{
                int bv = Rb_or_imm<2:0>;
                Rc =  bv ? (Ra & ~(mask(16) >> (64 - 8 * bv))) : Ra;
            }});
            0x62: msklh({{
                int bv = Rb_or_imm<2:0>;
                Rc =  bv ? (Ra & ~(mask(32) >> (64 - 8 * bv))) : Ra;
            }});
            0x72: mskqh({{
                int bv = Rb_or_imm<2:0>;
                Rc =  bv ? (Ra & ~(mask(64) >> (64 - 8 * bv))) : Ra;
            }});

            0x06: extbl({{ Rc = (Ra.uq >> (Rb_or_imm<2:0> * 8))< 7:0>; }});
            0x16: extwl({{ Rc = (Ra.uq >> (Rb_or_imm<2:0> * 8))<15:0>; }});
            0x26: extll({{ Rc = (Ra.uq >> (Rb_or_imm<2:0> * 8))<31:0>; }});
            0x36: extql({{ Rc = (Ra.uq >> (Rb_or_imm<2:0> * 8)); }});

            0x5a: extwh({{
                Rc = (Ra << (64 - (Rb_or_imm<2:0> * 8))<5:0>)<15:0>; }});
            0x6a: extlh({{
                Rc = (Ra << (64 - (Rb_or_imm<2:0> * 8))<5:0>)<31:0>; }});
            0x7a: extqh({{
                Rc = (Ra << (64 - (Rb_or_imm<2:0> * 8))<5:0>); }});

            0x0b: insbl({{ Rc = Ra< 7:0> << (Rb_or_imm<2:0> * 8); }});
            0x1b: inswl({{ Rc = Ra<15:0> << (Rb_or_imm<2:0> * 8); }});
            0x2b: insll({{ Rc = Ra<31:0> << (Rb_or_imm<2:0> * 8); }});
            0x3b: insql({{ Rc = Ra       << (Rb_or_imm<2:0> * 8); }});

            0x57: inswh({{
                int bv = Rb_or_imm<2:0>;
                Rc = bv ? (Ra.uq<15:0> >> (64 - 8 * bv)) : 0;
            }});
            0x67: inslh({{
                int bv = Rb_or_imm<2:0>;
                Rc = bv ? (Ra.uq<31:0> >> (64 - 8 * bv)) : 0;
            }});
            0x77: insqh({{
                int bv = Rb_or_imm<2:0>;
                Rc = bv ? (Ra.uq       >> (64 - 8 * bv)) : 0;
            }});

            0x30: zap({{
                uint64_t zapmask = 0;
                for (int i = 0; i < 8; ++i) {
                    if (Rb_or_imm<i:>)
                        zapmask |= (mask(8) << (i * 8));
                }
                Rc = Ra & ~zapmask;
            }});
            0x31: zapnot({{
                uint64_t zapmask = 0;
                for (int i = 0; i < 8; ++i) {
                    if (!Rb_or_imm<i:>)
                        zapmask |= (mask(8) << (i * 8));
                }
                Rc = Ra & ~zapmask;
            }});
        }

        0x13: decode INTFUNC {	// integer multiplies
            0x00: mull({{ Rc.sl = Ra.sl * Rb_or_imm.sl; }}, IntMultOp);
            0x20: mulq({{ Rc    = Ra    * Rb_or_imm;    }}, IntMultOp);
            0x30: umulh({{
                uint64_t hi, lo;
                mul128(Ra, Rb_or_imm, hi, lo);
                Rc = hi;
            }}, IntMultOp);
            0x40: mullv({{
                // 32-bit multiply with trap on overflow
                int64_t Rax = Ra.sl;	// sign extended version of Ra.sl
                int64_t Rbx = Rb_or_imm.sl;
                int64_t tmp = Rax * Rbx;
                // To avoid overflow, all the upper 32 bits must match
                // the sign bit of the lower 32.  We code this as
                // checking the upper 33 bits for all 0s or all 1s.
                uint64_t sign_bits = tmp<63:31>;
                if (sign_bits != 0 && sign_bits != mask(33))
                    fault = Integer_Overflow_Fault;
                Rc.sl = tmp<31:0>;
            }}, IntMultOp);
            0x60: mulqv({{
                // 64-bit multiply with trap on overflow
                uint64_t hi, lo;
                mul128(Ra, Rb_or_imm, hi, lo);
                // all the upper 64 bits must match the sign bit of
                // the lower 64
                if (!((hi == 0 && lo<63:> == 0) ||
                      (hi == mask(64) && lo<63:> == 1)))
                    fault = Integer_Overflow_Fault;
                Rc = lo;
            }}, IntMultOp);
        }

        0x1c: decode INTFUNC {
            0x00: decode RA { 31: sextb({{ Rc.sb = Rb_or_imm< 7:0>; }}); }
            0x01: decode RA { 31: sextw({{ Rc.sw = Rb_or_imm<15:0>; }}); }
            0x32: ctlz({{
                             uint64_t count = 0;
                             uint64_t temp = Rb;
                             if (temp<63:32>) temp >>= 32; else count += 32;
                             if (temp<31:16>) temp >>= 16; else count += 16;
                             if (temp<15:8>) temp >>= 8; else count += 8;
                             if (temp<7:4>) temp >>= 4; else count += 4;
                             if (temp<3:2>) temp >>= 2; else count += 2;
                             if (temp<1:1>) temp >>= 1; else count += 1;
                             if ((temp<0:0>) != 0x1) count += 1;
                             Rc = count;
                           }}, IntAluOp);

            0x33: cttz({{
                             uint64_t count = 0;
                             uint64_t temp = Rb;
                             if (!(temp<31:0>)) { temp >>= 32; count += 32; }
                             if (!(temp<15:0>)) { temp >>= 16; count += 16; }
                             if (!(temp<7:0>)) { temp >>= 8; count += 8; }
                             if (!(temp<3:0>)) { temp >>= 4; count += 4; }
                             if (!(temp<1:0>)) { temp >>= 2; count += 2; }
                             if (!(temp<0:0> & ULL(0x1))) count += 1;
                             Rc = count;
                           }}, IntAluOp);

            format FailUnimpl {
                0x30: ctpop();
                0x31: perr();
                0x34: unpkbw();
                0x35: unpkbl();
                0x36: pkwb();
                0x37: pklb();
                0x38: minsb8();
                0x39: minsw4();
                0x3a: minub8();
                0x3b: minuw4();
                0x3c: maxub8();
                0x3d: maxuw4();
                0x3e: maxsb8();
                0x3f: maxsw4();
            }

            format BasicOperateWithNopCheck {
                0x70: decode RB {
                    31: ftoit({{ Rc = Fa.uq; }}, FloatCvtOp);
                }
                0x78: decode RB {
                    31: ftois({{ Rc.sl = t_to_s(Fa.uq); }},
                              FloatCvtOp);
                }
            }
        }
    }

    //Table 3-2 CPU Branch and Jump Instructions ( )
    //Table 3-10 Obsolete CPU Branch Instructions ( )

    //Table 3-3 CPU Instruction Control Instructions ( )

    //Table 3-4 CPU Load, Store, and Memory Control Instructions ( )

    //Table 3-5 CPU Logical Instructions ( )

    //Table 3-6 CPU Insert/Extract Instructions ( )

    //Table 3-7 CPU Move Instructions ( )

    //Table 3-9 CPU Trap Instructions ( )

    //Table 3-11 FPU Arithmetic Instructions ( )

    //Table 3-12 FPU Branch Instructions ( )
    //Table 3-17 Obsolete  FPU Branch Instructions ()

    //Table 3-13 FPU Compare Instructions ( )

    //Table 3-14 FPU Convert Instructions ( )

    //Table 3-15 FPU Load, Store, and Memory Control Instructions ( )

    //Table 3-16 FPU Move Instructions ( )

    //Tables 3-18 thru 3-22 are Co-Processor Instructions ( )

    //Table 3-23 Privileged Instructions ( )

    //Table 3-24 EJTAG Instructions ( )




    format LoadAddress {
        0x08: lda({{ Ra = Rb + disp; }});
        0x09: ldah({{ Ra = Rb + (disp << 16); }});
    }

    format LoadOrNop {
        0x0a: ldbu({{ EA = Rb + disp; }}, {{ Ra.uq = Mem.ub; }});
        0x0c: ldwu({{ EA = Rb + disp; }}, {{ Ra.uq = Mem.uw; }});
        0x0b: ldq_u({{ EA = (Rb + disp) & ~7; }}, {{ Ra = Mem.uq; }});
        0x23: ldt({{ EA = Rb + disp; }}, {{ Fa = Mem.df; }});
        0x2a: ldl_l({{ EA = Rb + disp; }}, {{ Ra.sl = Mem.sl; }}, LOCKED);
        0x2b: ldq_l({{ EA = Rb + disp; }}, {{ Ra.uq = Mem.uq; }}, LOCKED);
        0x20: copy_load({{EA = Ra;}},
                        {{fault = xc->copySrcTranslate(EA);}},
                        IsMemRef, IsLoad, IsCopy);
    }

    format LoadOrPrefetch {
        0x28: ldl({{ EA = Rb + disp; }}, {{ Ra.sl = Mem.sl; }});
        0x29: ldq({{ EA = Rb + disp; }}, {{ Ra.uq = Mem.uq; }}, EVICT_NEXT);
        // IsFloating flag on lds gets the prefetch to disassemble
        // using f31 instead of r31... funcitonally it's unnecessary
        0x22: lds({{ EA = Rb + disp; }}, {{ Fa.uq = s_to_t(Mem.ul); }},
                  PF_EXCLUSIVE, IsFloating);
    }

    format Store {
        0x0e: stb({{ EA = Rb + disp; }}, {{ Mem.ub = Ra<7:0>; }});
        0x0d: stw({{ EA = Rb + disp; }}, {{ Mem.uw = Ra<15:0>; }});
        0x2c: stl({{ EA = Rb + disp; }}, {{ Mem.ul = Ra<31:0>; }});
        0x2d: stq({{ EA = Rb + disp; }}, {{ Mem.uq = Ra.uq; }});
        0x0f: stq_u({{ EA = (Rb + disp) & ~7; }}, {{ Mem.uq = Ra.uq; }});
        0x26: sts({{ EA = Rb + disp; }}, {{ Mem.ul = t_to_s(Fa.uq); }});
        0x27: stt({{ EA = Rb + disp; }}, {{ Mem.df = Fa; }});
        0x24: copy_store({{EA = Rb;}},
                         {{fault = xc->copy(EA);}},
                         IsMemRef, IsStore, IsCopy);
    }

    format StoreCond {
        0x2e: stl_c({{ EA = Rb + disp; }}, {{ Mem.ul = Ra<31:0>; }},
                    {{
                        uint64_t tmp = Mem_write_result;
                        // see stq_c
                        Ra = (tmp == 0 || tmp == 1) ? tmp : Ra;
                    }}, LOCKED);
        0x2f: stq_c({{ EA = Rb + disp; }}, {{ Mem.uq = Ra; }},
                    {{
                        uint64_t tmp = Mem_write_result;
                        // If the write operation returns 0 or 1, then
                        // this was a conventional store conditional,
                        // and the value indicates the success/failure
                        // of the operation.  If another value is
                        // returned, then this was a Turbolaser
                        // mailbox access, and we don't update the
                        // result register at all.
                        Ra = (tmp == 0 || tmp == 1) ? tmp : Ra;
                    }}, LOCKED);
    }



    // Conditional branches.
    format CondBranch {
        0x39: beq({{ cond = (Ra == 0); }});
        0x3d: bne({{ cond = (Ra != 0); }});
        0x3e: bge({{ cond = (Ra.sq >= 0); }});
        0x3f: bgt({{ cond = (Ra.sq >  0); }});
        0x3b: ble({{ cond = (Ra.sq <= 0); }});
        0x3a: blt({{ cond = (Ra.sq < 0); }});
        0x38: blbc({{ cond = ((Ra & 1) == 0); }});
        0x3c: blbs({{ cond = ((Ra & 1) == 1); }});

        0x31: fbeq({{ cond = (Fa == 0); }});
        0x35: fbne({{ cond = (Fa != 0); }});
        0x36: fbge({{ cond = (Fa >= 0); }});
        0x37: fbgt({{ cond = (Fa >  0); }});
        0x33: fble({{ cond = (Fa <= 0); }});
        0x32: fblt({{ cond = (Fa < 0); }});
    }

    // unconditional branches
    format UncondBranch {
        0x30: br();
        0x34: bsr(IsCall);
    }

    // indirect branches
    0x1a: decode JMPFUNC {
        format Jump {
            0: jmp();
            1: jsr(IsCall);
            2: ret(IsReturn);
            3: jsr_coroutine(IsCall, IsReturn);
        }
    }

    // Square root and integer-to-FP moves
    0x14: decode FP_SHORTFUNC {
        // Integer to FP register moves must have RB == 31
        0x4: decode RB {
            31: decode FP_FULLFUNC {
                format BasicOperateWithNopCheck {
                    0x004: itofs({{ Fc.uq = s_to_t(Ra.ul); }}, FloatCvtOp);
                    0x024: itoft({{ Fc.uq = Ra.uq; }}, FloatCvtOp);
                    0x014: FailUnimpl::itoff();	// VAX-format conversion
                }
            }
        }

        // Square root instructions must have FA == 31
        0xb: decode FA {
            31: decode FP_TYPEFUNC {
                format FloatingPointOperate {
#if SS_COMPATIBLE_FP
                    0x0b: sqrts({{
                        if (Fb < 0.0)
                            fault = Arithmetic_Fault;
                        Fc = sqrt(Fb);
                    }}, FloatSqrtOp);
#else
                    0x0b: sqrts({{
                        if (Fb.sf < 0.0)
                            fault = Arithmetic_Fault;
                        Fc.sf = sqrt(Fb.sf);
                    }}, FloatSqrtOp);
#endif
                    0x2b: sqrtt({{
                        if (Fb < 0.0)
                            fault = Arithmetic_Fault;
                        Fc = sqrt(Fb);
                    }}, FloatSqrtOp);
                }
            }
        }

        // VAX-format sqrtf and sqrtg are not implemented
        0xa: FailUnimpl::sqrtfg();
    }

    // IEEE floating point
    0x16: decode FP_SHORTFUNC_TOP2 {
        // The top two bits of the short function code break this
        // space into four groups: binary ops, compares, reserved, and
        // conversions.  See Table 4-12 of AHB.  There are different
        // special cases in these different groups, so we decode on
        // these top two bits first just to select a decode strategy.
        // Most of these instructions may have various trapping and
        // rounding mode flags set; these are decoded in the
        // FloatingPointDecode template used by the
        // FloatingPointOperate format.

        // add/sub/mul/div: just decode on the short function code
        // and source type.  All valid trapping and rounding modes apply.
        0: decode FP_TRAPMODE {
            // check for valid trapping modes here
            0,1,5,7: decode FP_TYPEFUNC {
                   format FloatingPointOperate {
#if SS_COMPATIBLE_FP
                       0x00: adds({{ Fc = Fa + Fb; }});
                       0x01: subs({{ Fc = Fa - Fb; }});
                       0x02: muls({{ Fc = Fa * Fb; }}, FloatMultOp);
                       0x03: divs({{ Fc = Fa / Fb; }}, FloatDivOp);
#else
                       0x00: adds({{ Fc.sf = Fa.sf + Fb.sf; }});
                       0x01: subs({{ Fc.sf = Fa.sf - Fb.sf; }});
                       0x02: muls({{ Fc.sf = Fa.sf * Fb.sf; }}, FloatMultOp);
                       0x03: divs({{ Fc.sf = Fa.sf / Fb.sf; }}, FloatDivOp);
#endif

                       0x20: addt({{ Fc = Fa + Fb; }});
                       0x21: subt({{ Fc = Fa - Fb; }});
                       0x22: mult({{ Fc = Fa * Fb; }}, FloatMultOp);
                       0x23: divt({{ Fc = Fa / Fb; }}, FloatDivOp);
                   }
             }
        }

        // Floating-point compare instructions must have the default
        // rounding mode, and may use the default trapping mode or
        // /SU.  Both trapping modes are treated the same by M5; the
        // only difference on the real hardware (as far a I can tell)
        // is that without /SU you'd get an imprecise trap if you
        // tried to compare a NaN with something else (instead of an
        // "unordered" result).
        1: decode FP_FULLFUNC {
            format BasicOperateWithNopCheck {
                0x0a5, 0x5a5: cmpteq({{ Fc = (Fa == Fb) ? 2.0 : 0.0; }},
                                     FloatCmpOp);
                0x0a7, 0x5a7: cmptle({{ Fc = (Fa <= Fb) ? 2.0 : 0.0; }},
                                     FloatCmpOp);
                0x0a6, 0x5a6: cmptlt({{ Fc = (Fa <  Fb) ? 2.0 : 0.0; }},
                                     FloatCmpOp);
                0x0a4, 0x5a4: cmptun({{ // unordered
                    Fc = (!(Fa < Fb) && !(Fa == Fb) && !(Fa > Fb)) ? 2.0 : 0.0;
                }}, FloatCmpOp);
            }
        }

        // The FP-to-integer and integer-to-FP conversion insts
        // require that FA be 31.
        3: decode FA {
            31: decode FP_TYPEFUNC {
                format FloatingPointOperate {
                    0x2f: decode FP_ROUNDMODE {
                        format FPFixedRounding {
                            // "chopped" i.e. round toward zero
                            0: cvttq({{ Fc.sq = (int64_t)trunc(Fb); }},
                                     Chopped);
                            // round to minus infinity
                            1: cvttq({{ Fc.sq = (int64_t)floor(Fb); }},
                                     MinusInfinity);
                        }
                      default: cvttq({{ Fc.sq = (int64_t)nearbyint(Fb); }});
                    }

                    // The cvtts opcode is overloaded to be cvtst if the trap
                    // mode is 2 or 6 (which are not valid otherwise)
                    0x2c: decode FP_FULLFUNC {
                        format BasicOperateWithNopCheck {
                            // trap on denorm version "cvtst/s" is
                            // simulated same as cvtst
                            0x2ac, 0x6ac: cvtst({{ Fc = Fb.sf; }});
                        }
                      default: cvtts({{ Fc.sf = Fb; }});
                    }

                    // The trapping mode for integer-to-FP conversions
                    // must be /SUI or nothing; /U and /SU are not
                    // allowed.  The full set of rounding modes are
                    // supported though.
                    0x3c: decode FP_TRAPMODE {
                        0,7: cvtqs({{ Fc.sf = Fb.sq; }});
                    }
                    0x3e: decode FP_TRAPMODE {
                        0,7: cvtqt({{ Fc    = Fb.sq; }});
                    }
                }
            }
        }
    }

    // misc FP operate
    0x17: decode FP_FULLFUNC {
        format BasicOperateWithNopCheck {
            0x010: cvtlq({{
                Fc.sl = (Fb.uq<63:62> << 30) | Fb.uq<58:29>;
            }});
            0x030: cvtql({{
                Fc.uq = (Fb.uq<31:30> << 62) | (Fb.uq<29:0> << 29);
            }});

            // We treat the precise & imprecise trapping versions of
            // cvtql identically.
            0x130, 0x530: cvtqlv({{
                // To avoid overflow, all the upper 32 bits must match
                // the sign bit of the lower 32.  We code this as
                // checking the upper 33 bits for all 0s or all 1s.
                uint64_t sign_bits = Fb.uq<63:31>;
                if (sign_bits != 0 && sign_bits != mask(33))
                    fault = Integer_Overflow_Fault;
                Fc.uq = (Fb.uq<31:30> << 62) | (Fb.uq<29:0> << 29);
            }});

            0x020: cpys({{  // copy sign
                Fc.uq = (Fa.uq<63:> << 63) | Fb.uq<62:0>;
            }});
            0x021: cpysn({{ // copy sign negated
                Fc.uq = (~Fa.uq<63:> << 63) | Fb.uq<62:0>;
            }});
            0x022: cpyse({{ // copy sign and exponent
                Fc.uq = (Fa.uq<63:52> << 52) | Fb.uq<51:0>;
            }});

            0x02a: fcmoveq({{ Fc = (Fa == 0) ? Fb : Fc; }});
            0x02b: fcmovne({{ Fc = (Fa != 0) ? Fb : Fc; }});
            0x02c: fcmovlt({{ Fc = (Fa <  0) ? Fb : Fc; }});
            0x02d: fcmovge({{ Fc = (Fa >= 0) ? Fb : Fc; }});
            0x02e: fcmovle({{ Fc = (Fa <= 0) ? Fb : Fc; }});
            0x02f: fcmovgt({{ Fc = (Fa >  0) ? Fb : Fc; }});

            0x024: mt_fpcr({{ FPCR = Fa.uq; }});
            0x025: mf_fpcr({{ Fa.uq = FPCR; }});
        }
    }

    // miscellaneous mem-format ops
    0x18: decode MEMFUNC {
        format WarnUnimpl {
            0x8000: fetch();
            0xa000: fetch_m();
            0xe800: ecb();
        }

        format MiscPrefetch {
            0xf800: wh64({{ EA = Rb & ~ULL(63); }},
                         {{ xc->writeHint(EA, 64, memAccessFlags); }},
                         IsMemRef, IsDataPrefetch, IsStore, MemWriteOp,
                         NO_FAULT);
        }

        format BasicOperate {
            0xc000: rpcc({{
#if FULL_SYSTEM
        /* Rb is a fake dependency so here is a fun way to get
         * the parser to understand that.
         */
                Ra = xc->readIpr(MipsISA::IPR_CC, fault) + (Rb & 0);

#else
                Ra = curTick;
#endif
            }});

            // All of the barrier instructions below do nothing in
            // their execute() methods (hence the empty code blocks).
            // All of their functionality is hard-coded in the
            // pipeline based on the flags IsSerializing,
            // IsMemBarrier, and IsWriteBarrier.  In the current
            // detailed CPU model, the execute() function only gets
            // called at fetch, so there's no way to generate pipeline
            // behavior at any other stage.  Once we go to an
            // exec-in-exec CPU model we should be able to get rid of
            // these flags and implement this behavior via the
            // execute() methods.

            // trapb is just a barrier on integer traps, where excb is
            // a barrier on integer and FP traps.  "EXCB is thus a
            // superset of TRAPB." (Mips ARM, Sec 4.11.4) We treat
            // them the same though.
            0x0000: trapb({{ }}, IsSerializing, IsSerializeBefore, No_OpClass);
            0x0400: excb({{ }}, IsSerializing, IsSerializeBefore, No_OpClass);
            0x4000: mb({{ }}, IsMemBarrier, MemReadOp);
            0x4400: wmb({{ }}, IsWriteBarrier, MemWriteOp);
        }

#if FULL_SYSTEM
        format BasicOperate {
            0xe000: rc({{
                Ra = xc->readIntrFlag();
                xc->setIntrFlag(0);
            }}, IsNonSpeculative);
            0xf000: rs({{
                Ra = xc->readIntrFlag();
                xc->setIntrFlag(1);
            }}, IsNonSpeculative);
        }
#else
        format FailUnimpl {
            0xe000: rc();
            0xf000: rs();
        }
#endif
    }

#if FULL_SYSTEM
    0x00: CallPal::call_pal({{
        if (!palValid ||
            (palPriv
             && xc->readIpr(MipsISA::IPR_ICM, fault) != MipsISA::mode_kernel)) {
            // invalid pal function code, or attempt to do privileged
            // PAL call in non-kernel mode
            fault = Unimplemented_Opcode_Fault;
        }
        else {
            // check to see if simulator wants to do something special
            // on this PAL call (including maybe suppress it)
            bool dopal = xc->simPalCheck(palFunc);

            if (dopal) {
                MipsISA::swap_palshadow(&xc->xcBase()->regs, true);
                xc->setIpr(MipsISA::IPR_EXC_ADDR, NPC);
                NPC = xc->readIpr(MipsISA::IPR_PAL_BASE, fault) + palOffset;
            }
        }
    }}, IsNonSpeculative);
#else
    0x00: decode PALFUNC {
        format EmulatedCallPal {
            0x00: halt ({{
                SimExit(curTick, "halt instruction encountered");
            }}, IsNonSpeculative);
            0x83: callsys({{
                xc->syscall();
            }}, IsNonSpeculative, IsSerializeAfter);
            // Read uniq reg into ABI return value register (r0)
            0x9e: rduniq({{ R0 = Runiq; }});
            // Write uniq reg with value from ABI arg register (r16)
            0x9f: wruniq({{ Runiq = R16; }});
        }
    }
#endif

#if FULL_SYSTEM
    format HwLoadStore {
        0x1b: decode HW_LDST_QUAD {
            0: hw_ld({{ EA = (Rb + disp) & ~3; }}, {{ Ra = Mem.ul; }}, L);
            1: hw_ld({{ EA = (Rb + disp) & ~7; }}, {{ Ra = Mem.uq; }}, Q);
        }

        0x1f: decode HW_LDST_COND {
            0: decode HW_LDST_QUAD {
                0: hw_st({{ EA = (Rb + disp) & ~3; }},
                         {{ Mem.ul = Ra<31:0>; }}, L);
                1: hw_st({{ EA = (Rb + disp) & ~7; }},
                         {{ Mem.uq = Ra.uq; }}, Q);
            }

            1: FailUnimpl::hw_st_cond();
        }
    }

    format HwMoveIPR {
        0x19: hw_mfpr({{
            // this instruction is only valid in PAL mode
            if (!xc->inPalMode()) {
                fault = Unimplemented_Opcode_Fault;
            }
            else {
                Ra = xc->readIpr(ipr_index, fault);
            }
        }});
        0x1d: hw_mtpr({{
            // this instruction is only valid in PAL mode
            if (!xc->inPalMode()) {
                fault = Unimplemented_Opcode_Fault;
            }
            else {
                xc->setIpr(ipr_index, Ra);
                if (traceData) { traceData->setData(Ra); }
            }
        }});
    }

    format BasicOperate {
        0x1e: hw_rei({{ xc->hwrei(); }}, IsSerializing, IsSerializeBefore);

        // M5 special opcodes use the reserved 0x01 opcode space
        0x01: decode M5FUNC {
            0x00: arm({{
                MipsPseudo::arm(xc->xcBase());
            }}, IsNonSpeculative);
            0x01: quiesce({{
                MipsPseudo::quiesce(xc->xcBase());
            }}, IsNonSpeculative);
            0x10: ivlb({{
                MipsPseudo::ivlb(xc->xcBase());
            }}, No_OpClass, IsNonSpeculative);
            0x11: ivle({{
                MipsPseudo::ivle(xc->xcBase());
            }}, No_OpClass, IsNonSpeculative);
            0x20: m5exit_old({{
                MipsPseudo::m5exit_old(xc->xcBase());
            }}, No_OpClass, IsNonSpeculative);
            0x21: m5exit({{
                MipsPseudo::m5exit(xc->xcBase());
            }}, No_OpClass, IsNonSpeculative);
            0x30: initparam({{ Ra = xc->xcBase()->cpu->system->init_param; }});
            0x40: resetstats({{
                MipsPseudo::resetstats(xc->xcBase());
            }}, IsNonSpeculative);
            0x41: dumpstats({{
                MipsPseudo::dumpstats(xc->xcBase());
            }}, IsNonSpeculative);
            0x42: dumpresetstats({{
                MipsPseudo::dumpresetstats(xc->xcBase());
            }}, IsNonSpeculative);
            0x43: m5checkpoint({{
                MipsPseudo::m5checkpoint(xc->xcBase());
            }}, IsNonSpeculative);
            0x50: m5readfile({{
                MipsPseudo::readfile(xc->xcBase());
            }}, IsNonSpeculative);
            0x51: m5break({{
                MipsPseudo::debugbreak(xc->xcBase());
            }}, IsNonSpeculative);
            0x52: m5switchcpu({{
                MipsPseudo::switchcpu(xc->xcBase());
            }}, IsNonSpeculative);
            0x53: m5addsymbol({{
                MipsPseudo::addsymbol(xc->xcBase());
            }}, IsNonSpeculative);

        }
    }
#endif
}

