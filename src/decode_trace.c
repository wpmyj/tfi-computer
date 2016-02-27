#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_SYMBOLS 1024
#define MAX_TRACES 4096

typedef enum {
  TRACE_ENTER,
  TRACE_EXIT,
} trace_type;

#ifdef HOSTED
struct {
  uint32_t num_traces;
  struct {
    trace_type type;
    uint64_t this_fn;
    uint64_t call_site;
    uint64_t time;
  } __attribute((packed)) traces[MAX_TRACES];
} __attribute((packed)) trace;
#else
struct {
  uint32_t num_traces;
  struct {
    trace_type type;
    uint32_t this_fn;
    uint32_t call_site;
    uint32_t time;
  } __attribute((packed)) traces[MAX_TRACES];
} __attribute((packed)) trace;
#endif

struct symbols {
  uint64_t address;
  char name[64];
} symbols[MAX_SYMBOLS];

int load_symbols(struct symbols *s, int max_symbols, FILE *f) {
  int cur_index = 0;
  char *line = NULL;
  size_t len;
  while (getline(&line, &len, f) != -1) {
    if (cur_index >= max_symbols) {
      break;
    }
    sscanf(line, "%lx %*c %64s\n", 
      &s[cur_index].address, s[cur_index].name);
    if (s[cur_index].address != 0) {
      cur_index++;
    }
    if (line) {
      free(line);
      line = NULL;
    }
  }
  return cur_index;
}

const char *lookup_symbol(struct symbols *s, int n, uint64_t addr) {
  int i;
  for (i = 0; i < n; ++i) {
    if (s[i].address == addr) {
      return s[i].name;
    }
  }
  return NULL;
}

int main(int argc, char **argv) {
  int c, n_syms;
  FILE *symbolfile, *tracefile;
  while ((c = getopt(argc, argv, "s:t:")) != -1) {
    switch (c) {
      case 's':
        symbolfile = fopen(optarg, "r");
        break;
      case 't':
        tracefile = fopen(optarg, "r");
        break;
      default:
        printf("Usage: %s -s symbols -t trace\n", argv[0]);
        abort();
    }
  }
  if (!symbolfile) {
    printf("Unable to open symbols\n");
    abort();
  }
  if (!tracefile) {
    printf("Unable to open trace\n");
    abort();
  }
  n_syms = load_symbols(symbols, MAX_SYMBOLS, symbolfile);
  fread((void*)&trace, 1, sizeof(trace), tracefile);
  if (trace.num_traces >= MAX_TRACES) {
    printf("Too many traces recorded!\n");
    exit(EXIT_FAILURE);
  }

  int i;
  for (i = 0; i < trace.num_traces; ++i) {
    printf("%c %s %x %lu\n", trace.traces[i].type == 0 ? '{' : '}',
      lookup_symbol(symbols, n_syms, trace.traces[i].this_fn), 
      trace.traces[i].call_site, 
      trace.traces[i].time);
  } 

  return 0;
}
