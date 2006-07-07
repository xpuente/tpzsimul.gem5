/*
 * Copyright (c) 2001-2005 The Regents of The University of Michigan
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
 * Authors: Steve Reinhardt
 *          Nathan Binkert
 */

/* @file
 * User Console Definitions
 */

#ifndef __SIM_OBJECT_HH__
#define __SIM_OBJECT_HH__

#include <map>
#include <list>
#include <vector>
#include <iostream>

#include "sim/serialize.hh"
#include "sim/startup.hh"

class BaseCPU;
class Event;

/*
 * Abstract superclass for simulation objects.  Represents things that
 * correspond to physical components and can be specified via the
 * config file (CPUs, caches, etc.).
 */
class SimObject : public Serializable, protected StartupCallback
{
  public:
    struct Params {
        std::string name;
    };

    enum State {
        Atomic,
        Timing,
        Draining,
        DrainedAtomic,
        DrainedTiming
    };

  protected:
    Params *_params;
    State state;

    void changeState(State new_state) { state = new_state; }

  public:
    const Params *params() const { return _params; }

    State getState() { return state; }

  private:
    typedef std::vector<SimObject *> SimObjectList;

    // list of all instantiated simulation objects
    static SimObjectList simObjectList;

  public:
    SimObject(Params *_params);
    SimObject(const std::string &_name);

    virtual ~SimObject() {}

    virtual const std::string name() const { return params()->name; }

    // initialization pass of all objects.
    // Gets invoked after construction, before unserialize.
    virtual void init();
    virtual void connect();
    static void initAll();
    static void connectAll();

    // register statistics for this object
    virtual void regStats();
    virtual void regFormulas();
    virtual void resetStats();

    // static: call reg_stats on all SimObjects
    static void regAllStats();

    // static: call resetStats on all SimObjects
    static void resetAllStats();

    // static: call nameOut() & serialize() on all SimObjects
    static void serializeAll(std::ostream &);
    static void unserializeAll(Checkpoint *cp);

    // Methods to drain objects in order to take checkpoints
    // Or switch from timing -> atomic memory model
    // Drain returns false if the SimObject cannot drain immediately.
    virtual bool drain(Event *drain_event);
    virtual void resume();
    virtual void setMemoryMode(State new_mode);
    virtual void switchOut();
    virtual void takeOverFrom(BaseCPU *cpu);

#ifdef DEBUG
  public:
    bool doDebugBreak;
    static void debugObjectBreak(const std::string &objs);
#endif

  public:
    bool doRecordEvent;
    void recordEvent(const std::string &stat);
};

#endif // __SIM_OBJECT_HH__
