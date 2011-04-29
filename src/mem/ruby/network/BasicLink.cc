/*
 * Copyright (c) 2011 Advanced Micro Devices, Inc.
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

#include "mem/ruby/network/BasicLink.hh"

BasicLink::BasicLink(const Params *p)
    : SimObject(p)
{
    m_latency = p->latency;
    m_bw_multiplier = p->bw_multiplier;
    m_weight = p->weight;
}

void
BasicLink::init()
{
}

void
BasicLink::print(std::ostream& out) const
{
    out << name();
}

BasicLink *
BasicLinkParams::create()
{
    return new BasicLink(this);
}

BasicExtLink::BasicExtLink(const Params *p)
    : BasicLink(p)
{
    m_int_node = p->int_node;
    m_ext_node = p->ext_node;
}

BasicExtLink *
BasicExtLinkParams::create()
{
    return new BasicExtLink(this);
}

BasicIntLink::BasicIntLink(const Params *p)
    : BasicLink(p)
{
    m_node_a = p->node_a;
    m_node_b = p->node_b;
}

BasicIntLink *
BasicIntLinkParams::create()
{
    return new BasicIntLink(this);
}
