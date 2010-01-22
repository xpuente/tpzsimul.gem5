
#include <sys/wait.h>
#include <algorithm>

#include "config/gems_root.hh"
#include "mem/ruby/libruby_internal.hh"
#include "mem/ruby/system/RubyPort.hh"
#include "mem/ruby/system/System.hh"
#include "mem/ruby/eventqueue/RubyEventQueue.hh"
#include "mem/ruby/system/MemoryVector.hh"
#include "mem/ruby/common/Address.hh"
#include "mem/ruby/recorder/Tracer.hh"

string RubyRequestType_to_string(const RubyRequestType& obj)
{
  switch(obj) {
  case RubyRequestType_IFETCH:
    return "IFETCH";
  case RubyRequestType_LD:
    return "LD";
  case RubyRequestType_ST:
    return "ST";
  case RubyRequestType_Locked_Read:
    return "Locked_Read";
  case RubyRequestType_Locked_Write:
    return "Locked_Write";
  case RubyRequestType_RMW_Read:
    return "RMW_Read";
  case RubyRequestType_RMW_Write:
    return "RMW_Write";
  case RubyRequestType_NULL:
  default:
    assert(0);
    return "";
  }
}

RubyRequestType string_to_RubyRequestType(std::string str)
{
  if (str == "IFETCH")
    return RubyRequestType_IFETCH;
  else if (str == "LD")
    return RubyRequestType_LD;
  else if (str == "ST")
    return RubyRequestType_ST;
  else if (str == "Locked_Read")
    return RubyRequestType_Locked_Read;
  else if (str == "Locked_Write")
    return RubyRequestType_Locked_Write;
  else if (str == "RMW_Read")
    return RubyRequestType_RMW_Read;
  else if (str == "RMW_Write")
    return RubyRequestType_RMW_Write;
  else
    assert(0);
  return RubyRequestType_NULL;
}

ostream& operator<<(ostream& out, const RubyRequestType& obj)
{
  out << RubyRequestType_to_string(obj);
  out << flush;
  return out;
}

ostream& operator<<(std::ostream& out, const RubyRequest& obj)
{
  out << hex << "0x" << obj.paddr << flush;
  return out;
}

vector<string> tokenizeString(string str, string delims)
{
  vector<string> tokens;
  char* pch;
  char* tmp;
  const char* c_delims = delims.c_str();
  tmp = new char[str.length()+1];
  strcpy(tmp, str.c_str());
  pch = strtok(tmp, c_delims);
  while (pch != NULL) {
    string tmp_str(pch);
    if (tmp_str == "null") tmp_str = "";
    tokens.push_back(tmp_str);

    pch = strtok(NULL, c_delims);
  }
  delete [] tmp;
  return tokens;
}

void libruby_init(const char* cfg_filename)
{
  ifstream cfg_output(cfg_filename);

  vector<RubyObjConf> * sys_conf = new vector<RubyObjConf>;

  string line;
  getline(cfg_output, line) ;
  while ( !cfg_output.eof() ) {
    vector<string> tokens = tokenizeString(line, " ");
    assert(tokens.size() >= 2);
    vector<string> argv;
    for (size_t i=2; i<tokens.size(); i++) {
      std::replace(tokens[i].begin(), tokens[i].end(), '%', ' ');
      std::replace(tokens[i].begin(), tokens[i].end(), '#', '\n');
      argv.push_back(tokens[i]);
    }
    sys_conf->push_back(RubyObjConf(tokens[0], tokens[1], argv));
    tokens.clear();
    argv.clear();
    getline(cfg_output, line);
  }

  RubySystem::create(*sys_conf);
  delete sys_conf;
}

RubyPortHandle libruby_get_port(const char* port_name, void (*hit_callback)(int64_t access_id))
{
  return static_cast<RubyPortHandle>(RubySystem::getPort(port_name, hit_callback));
}

RubyPortHandle libruby_get_port_by_name(const char* port_name)
{
  return static_cast<RubyPortHandle>(RubySystem::getPortOnly(port_name));
}

void libruby_write_ram(uint64_t paddr, uint8_t* data, int len)
{
  RubySystem::getMemoryVector()->write(Address(paddr), data, len);
}

void libruby_read_ram(uint64_t paddr, uint8_t* data, int len)
{
  RubySystem::getMemoryVector()->read(Address(paddr), data, len);
}

int64_t libruby_issue_request(RubyPortHandle p, struct RubyRequest request)
{
  return static_cast<RubyPort*>(p)->makeRequest(request);
}

int libruby_tick(int n)
{
  RubySystem::getEventQueue()->triggerEvents(RubySystem::getEventQueue()->getTime() + n);
  return 0;
}

void libruby_destroy()
{
}

const char* libruby_last_error()
{
  return "";
}

void libruby_print_config(std::ostream & out)
{
  RubySystem::printConfig(out);
}

void libruby_print_stats(std::ostream & out)
{
  RubySystem::printStats(out);
}
void libruby_playback_trace(char * trace_filename) 
{
  RubySystem::getTracer()->playbackTrace(trace_filename);
}

void libruby_start_tracing(char * record_filename) {
  // start the trace
  RubySystem::getTracer()->startTrace(record_filename);
}

void libruby_stop_tracing() {
  // start the trace
  RubySystem::getTracer()->stopTrace();
}

uint64_t libruby_get_time() {
  return RubySystem::getCycleCount(0);
}
