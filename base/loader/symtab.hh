/*
 * Copyright (c) 2002-2004 The Regents of The University of Michigan
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

#ifndef __SYMTAB_HH__
#define __SYMTAB_HH__

#include <map>
#include "targetarch/isa_traits.hh"	// for Addr

class SymbolTable
{
  private:
    typedef std::map<Addr, std::string> ATable;
    typedef std::map<std::string, Addr> STable;

    ATable addrTable;
    STable symbolTable;

  public:
    SymbolTable() {}
    SymbolTable(const std::string &file) { load(file); }
    ~SymbolTable() {}

    bool insert(Addr address, std::string symbol);
    bool load(const std::string &file);

    /// Find the nearest symbol equal to or less than the supplied
    /// address (e.g., the label for the enclosing function).
    /// @param address The address to look up.
    /// @param symbol  Return reference for symbol string.
    /// @param sym_address Return reference for symbol address.
    /// @param next_sym_address Address of following symbol (for
    /// determining valid range of symbol).
    /// @retval True if a symbol was found.
    bool findNearestSymbol(Addr address, std::string &symbol,
                           Addr &sym_address, Addr &next_sym_address) const;

    /// Overload for findNearestSymbol() for callers who don't care
    /// about next_sym_address.
    bool findNearestSymbol(Addr address, std::string &symbol,
                           Addr &sym_address) const
    {
        Addr dummy;
        return findNearestSymbol(address, symbol, sym_address, dummy);
    }


    bool findSymbol(Addr address, std::string &symbol) const;
    bool findAddress(const std::string &symbol, Addr &address) const;
};

/// Global unified debugging symbol table (for target).  Conceptually
/// there should be one of these per System object for full system,
/// and per Process object for non-full-system, but so far one big
/// global one has worked well enough.
extern SymbolTable *debugSymbolTable;

#endif // __SYMTAB_HH__
