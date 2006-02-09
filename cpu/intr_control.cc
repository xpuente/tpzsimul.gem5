/*
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
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
#include <vector>

#include "cpu/base.hh"
#include "cpu/intr_control.hh"
#include "sim/builder.hh"
#include "sim/sim_object.hh"

using namespace std;

IntrControl::IntrControl(const string &name, BaseCPU *c)
    : SimObject(name), cpu(c)
{}

/* @todo
 *Fix the cpu sim object parameter to be a system pointer
 *instead, to avoid some extra dereferencing
 */
void
IntrControl::post(int int_num, int index)
{
    std::vector<ExecContext *> &xcvec = cpu->system->execContexts;
    BaseCPU *temp = xcvec[0]->cpu;
    temp->post_interrupt(int_num, index);
}

void
IntrControl::post(int cpu_id, int int_num, int index)
{
    std::vector<ExecContext *> &xcvec = cpu->system->execContexts;
    BaseCPU *temp = xcvec[cpu_id]->cpu;
    temp->post_interrupt(int_num, index);
}

void
IntrControl::clear(int int_num, int index)
{
    std::vector<ExecContext *> &xcvec = cpu->system->execContexts;
    BaseCPU *temp = xcvec[0]->cpu;
    temp->clear_interrupt(int_num, index);
}

void
IntrControl::clear(int cpu_id, int int_num, int index)
{
    std::vector<ExecContext *> &xcvec = cpu->system->execContexts;
    BaseCPU *temp = xcvec[cpu_id]->cpu;
    temp->clear_interrupt(int_num, index);
}

BEGIN_DECLARE_SIM_OBJECT_PARAMS(IntrControl)

    SimObjectParam<BaseCPU *> cpu;

END_DECLARE_SIM_OBJECT_PARAMS(IntrControl)

BEGIN_INIT_SIM_OBJECT_PARAMS(IntrControl)

    INIT_PARAM(cpu, "the cpu")

END_INIT_SIM_OBJECT_PARAMS(IntrControl)

CREATE_SIM_OBJECT(IntrControl)
{
    return new IntrControl(getInstanceName(), cpu);
}

REGISTER_SIM_OBJECT("IntrControl", IntrControl)
