/* $Id$ */

/* @file
 * User Console Interface
 */

#ifndef __CONSOLE_HH__
#define __CONSOLE_HH__

#include <iostream>

#include "base/circlebuf.hh"
#include "cpu/intr_control.hh"
#include "base/pollevent.hh"
#include "base/socket.hh"
#include "sim/sim_object.hh"

class ConsoleListener;
class SimConsole : public SimObject
{
  protected:
    class Event : public PollEvent
    {
      protected:
        SimConsole *cons;

      public:
        Event(SimConsole *c, int fd, int e);
        void process(int revent);
    };

    friend class Event;
    Event *event;

  protected:
    int number;
    int in_fd;
    int out_fd;

  protected:
    ConsoleListener *listener;

  public:
    SimConsole(const std::string &name, const std::string &file, int num);
    ~SimConsole();

  protected:
    CircleBuf txbuf;
    CircleBuf rxbuf;
    std::ostream *outfile;
#if TRACING_ON == 1
    CircleBuf linebuf;
#endif

  public:
    ///////////////////////
    // Terminal Interface

    void attach(int fd, ConsoleListener *l = NULL) { attach(fd, fd, l); }
    void attach(int in, int out, ConsoleListener *l = NULL);
    void detach();

    void data();

    void close();
    void read(uint8_t &c) { read(&c, 1); }
    size_t read(uint8_t *buf, size_t len);
    void write(uint8_t c) { write(&c, 1); }
    size_t write(const uint8_t *buf, size_t len);

    void configTerm();

  protected:
    // interrupt status/enable
    int _status;
    int _enable;

    // interrupt handle
    IntrControl *intr;
    // Platform so we can post interrupts
    Platform    *platform;

  public:
    /////////////////
    // OS interface

    // Get a character from the console.
    bool in(uint8_t &value);

    // get a character from the console in the console specific format
    // corresponds to GETC:
    // retval<63:61>
    //     000: success: character received
    //     001: success: character received, more pending
    //     100: failure: no character ready
    //     110: failure: character received with error
    //     111: failure: character received with error, more pending
    // retval<31:0>
    //     character read from console
    //
    // Interrupts are cleared when the buffer is empty.
    uint64_t console_in();

    // Send a character to the console
    void out(char c, bool raise_int = true);

    enum {
        TransmitInterrupt = 1,
        ReceiveInterrupt = 2
    };

    // Read the current interrupt status of this console.
    int intStatus() { return _status; }

    // Set the interrupt enable bits.
    int clearInt(int i);
    void raiseInt(int i);

    void initInt(IntrControl *i);
    void setInt(int bits);

    void setPlatform(Platform *p);

    virtual void serialize(std::ostream &os);
    virtual void unserialize(Checkpoint *cp, const std::string &section);
};

class ConsoleListener : public SimObject
{
  protected:
    class Event : public PollEvent
    {
      protected:
        ConsoleListener *listener;

      public:
        Event(ConsoleListener *l, int fd, int e)
            : PollEvent(fd, e), listener(l) {}
        void process(int revent);
    };

    friend class Event;
    Event *event;

    typedef std::list<SimConsole *> list_t;
    typedef list_t::iterator iter_t;
    list_t ConsoleList;

  protected:
    ListenSocket listener;

  public:
    ConsoleListener(const std::string &name);
    ~ConsoleListener();

    void add(SimConsole *cons);

    void accept();
    void listen(int port);
};

#endif // __CONSOLE_HH__
