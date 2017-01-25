#include <sys/time.h>

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "platform.h"
#include "scheduler.h"
#include "config.h"

_Atomic static timeval_t curtime;
lua_State *lstate;

struct slot {
  uint16_t on_mask;
  uint16_t off_mask;
} __attribute__((packed)) *output_slots[2];
size_t max_slots;
size_t cur_slot = 0;
size_t cur_buffer = 0;

timeval_t current_time() {
	return curtime;
}

void set_event_timer(timeval_t t) {
}

timeval_t get_event_timer() {
  return 0;
}

void clear_event_timer() {
}

void disable_event_timer() {
}

void disable_interrupts() {
}
  
void enable_interrupts() {
}

int interrupts_enabled() {
  return 0;
}

void set_output(int output, char value) {
}

void set_gpio(int output, char value) {
}

void set_pwm(int output, float value) {
}

void adc_gather() {
}

timeval_t last_tx = 0;
int usart_tx_ready() {
  return (curtime -  last_tx) > 200;
}

int usart_rx_ready() {
  return 0;
}

void usart_rx_reset() {
}

void usart_tx(char *str, unsigned short len) {
  write(STDOUT_FILENO, str, len);
  last_tx = curtime;
}

void platform_load_config() {
}

void platform_save_config() {
}

timeval_t init_output_thread(uint32_t *buf0, uint32_t *buf1, uint32_t len) {
  output_slots[0] = (struct slot *)buf0; 
  output_slots[1] = (struct slot *)buf1; 
  max_slots = len;
  return 0;
}

int current_output_buffer() {
  return cur_buffer;
}

int current_output_slot() {
  return cur_slot;
}

void enable_test_trigger(trigger_type t, unsigned int rpm) {
}


static int hosted_lua_get_output(lua_State *l) {

  uint16_t on = output_slots[cur_buffer][cur_slot].on_mask;
  uint16_t off = output_slots[cur_buffer][cur_slot].off_mask;
  lua_pushnumber(l, on);
  lua_pushnumber(l, off);
  return 2;
}

static int hosted_lua_event_timer(lua_State *l) {

  return 0;
}

static int hosted_lua_trigger1(lua_State *l) {
  timeval_t time = (timeval_t)lua_tonumber(l, 1);
  config.decoder.last_t0 = time;
  config.decoder.needs_decoding_t0 = 1;
  return 0;
}

static void hosted_platform_timer() {
  /* - Increase "time"
   * - Trigger appropriate interrupts 
   *     timer event
   *     input event
   *     dma buffer swap
   *
   * - Set outputs from buffer
   */
  curtime++;
  cur_slot++;
  if (cur_slot == max_slots) {
    cur_buffer = (cur_buffer + 1) % 2;
    cur_slot = 0;
    scheduler_buffer_swap();
  }

  lua_getglobal(lstate, "tick");
  lua_pushnumber(lstate, (double)curtime);
  lua_pcall(lstate, 1, 0, 0);
}

void platform_init() {

  /* Set up signal handling */
  sigset_t smask;
  sigemptyset(&smask);
  struct sigaction sa = {
    .sa_handler = hosted_platform_timer,
    .sa_mask = smask,
  };
  sigaction(SIGVTALRM, &sa, NULL);




  /* Initialize lua framework */
  lstate = lua_open();
  luaL_openlibs(lstate);

  if (luaL_loadfile(lstate, "lua/platform.lua") || lua_pcall(lstate, 0, 0, 0)) {
    printf("Failed to load lua scripts\n");
    return;
  }

  /* Set up C callbacks */
  lua_pushcfunction(lstate, hosted_lua_trigger1);
  lua_setglobal(lstate, "hosted_trigger1");

  lua_pushcfunction(lstate, hosted_lua_get_output);
  lua_setglobal(lstate, "hosted_get_output");

  lua_getglobal(lstate, "tfi_platform_init");
  lua_pcall(lstate, 0, 0, 0);

#ifdef SUPPORTS_POSIX_TIMERS
  struct sigevent sev = {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo = SIGVTALRM,
  };

  timer_t timer;
  if (timer_create(CLOCK_MONOTONIC, &sev, &timer) == -1) {
    perror("timer_create"); 
  }
  struct itimerspec interval = {
    .it_interval = {
      .tv_sec = 0,
      .tv_nsec = 100000,
    },
    .it_value = {
      .tv_sec = 0,
      .tv_nsec = 100000,
    },
  };
  timer_settime(timer, 0, &interval, NULL);
#else
  /* Use interval timer */
  struct itimerval t = {
    .it_interval = (struct timeval) {
      .tv_sec = 0,
      .tv_usec = 1000,
    },
    .it_value = (struct timeval) {
      .tv_sec = 0,
      .tv_usec = 1000,
    },
  };
  setitimer(ITIMER_VIRTUAL, &t, NULL);
#endif

}
