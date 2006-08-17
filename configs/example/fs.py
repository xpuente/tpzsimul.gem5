# Copyright (c) 2006 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Ali Saidi

import optparse, os, sys

import m5
from m5.objects import *
m5.AddToPath('../common')
from FSConfig import *
from SysPaths import *
from Benchmarks import *

parser = optparse.OptionParser()

parser.add_option("-d", "--detailed", action="store_true")
parser.add_option("-t", "--timing", action="store_true")
parser.add_option("-m", "--maxtick", type="int")
parser.add_option("--maxtime", type="float")
parser.add_option("--dual", action="store_true",
                  help="Simulate two systems attached with an ethernet link")
parser.add_option("-b", "--benchmark", action="store", type="string",
                  dest="benchmark",
                  help="Specify the benchmark to run. Available benchmarks: %s"\
                          % DefinedBenchmarks)

(options, args) = parser.parse_args()

if args:
    print "Error: script doesn't take any positional arguments"
    sys.exit(1)

if options.detailed:
    cpu = DetailedO3CPU()
    cpu2 = DetailedO3CPU()
    mem_mode = 'timing'
elif options.timing:
    cpu = TimingSimpleCPU()
    cpu2 = TimingSimpleCPU()
    mem_mode = 'timing'
else:
    cpu = AtomicSimpleCPU()
    cpu2 = AtomicSimpleCPU()
    mem_mode = 'atomic'

cpu.clock = '2GHz'
cpu2.clock = '2GHz'

if options.benchmark:
    if options.benchmark not in Benchmarks:
        print "Error benchmark %s has not been defined." % options.benchmark
        print "Valid benchmarks are: %s" % DefinedBenchmarks
        sys.exit(1)

    bm = Benchmarks[options.benchmark]
else:
    if options.dual:
        bm = [Machine(), Machine()]
    else:
        bm = [Machine()]

if len(bm) == 2:
    s1 = makeLinuxAlphaSystem(mem_mode, bm[0])
    s1.cpu = cpu
    cpu.connectMemPorts(s1.membus)
    s2 = makeLinuxAlphaSystem(mem_mode, bm[1])
    s2.cpu = cpu2
    cpu2.connectMemPorts(s2.membus)
    root = makeDualRoot(s1, s2)
elif len(bm) == 1:
    root = Root(clock = '1THz',
                system = makeLinuxAlphaSystem(mem_mode, bm[0]))
    root.system.cpu = cpu
    cpu.connectMemPorts(root.system.membus)
else:
    print "Error I don't know how to create more than 2 systems."
    sys.exit(1)

m5.instantiate(root)

#exit_event = m5.simulate(2600000000000)
#if exit_event.getCause() != "user interrupt received":
#    m5.checkpoint(root, 'cpt')
#    exit_event = m5.simulate(300000000000)
#    if exit_event.getCause() != "user interrupt received":
#        m5.checkpoint(root, 'cptA')


if options.maxtick:
    exit_event = m5.simulate(options.maxtick)
elif options.maxtime:
    simtime = int(options.maxtime * root.clock.value)
    print "simulating for: ", simtime
    exit_event = m5.simulate(simtime)
else:
    exit_event = m5.simulate()

print 'Exiting @ cycle', m5.curTick(), 'because', exit_event.getCause()
