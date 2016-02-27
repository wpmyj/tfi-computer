#include "platform.h"

#ifndef MAX_TRACES
#define MAX_TRACES 4096
#endif

typedef enum {
  TRACE_ENTER,
  TRACE_EXIT,
} trace_type;

struct {
  unsigned int num_traces;
  struct {
    trace_type type;
    void *this_fn;
    void *call_site;
    timeval_t time;
  } __attribute((packed)) traces[MAX_TRACES];
} __attribute((packed)) trace;

unsigned int trace_on = 0;

void trace_init() {
  trace.num_traces = 0; 
  trace_on = 1;
}

void __cyg_profile_func_enter(void *this_fn, void *call_site)
  __attribute__ ((no_instrument_function));
void __cyg_profile_func_exit(void *this_fn, void *call_site)
  __attribute__ ((no_instrument_function));

void __cyg_profile_func_enter(void *this_fn, void *call_site) { 
  if (!trace_on) {
    return;
  }
  if (trace.num_traces >= MAX_TRACES - 10) {
    return;
  }
  unsigned int t = trace.num_traces++;
  trace.traces[t].type = TRACE_ENTER;
  trace.traces[t].this_fn = this_fn;
  trace.traces[t].call_site = call_site;
  trace.traces[t].time = current_time();
  
}
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
  if (!trace_on) {
    return;
  }
  if (trace.num_traces >= MAX_TRACES - 10) {
    return;
  }
  unsigned int t = trace.num_traces++;
  trace.traces[t].type = TRACE_EXIT;
  trace.traces[t].this_fn = this_fn;
  trace.traces[t].call_site = call_site;
  trace.traces[t].time = current_time();
}

