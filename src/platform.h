#ifndef _PLATFORM_H
#define _PLATFORM_H
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

struct decoder;
#ifdef TIMER64
typedef uint64_t timeval_t;
#else
typedef uint32_t timeval_t;
#endif
typedef uint16_t degrees_t;
/* timeval_t is gauranteed to be 32 bits */

timeval_t current_time();

void set_event_timer(timeval_t);
timeval_t get_event_timer();
/* Clear any pending interrupt */
void clear_event_timer();
void disable_event_timer();

void platform_init(int, char*[]);

void disable_interrupts();
void enable_interrupts();
int interrupts_enabled();

void set_output(int output, char value);
int get_output(int output);
void set_gpio(int output, char value);
void set_pwm(int output, float value);
void adc_gather();

int usart_tx_ready();
int usart_rx_ready();
void usart_rx_reset();
void usart_tx(char *, unsigned short);

void platform_load_config();
void platform_save_config();

timeval_t init_output_thread(uint32_t *buf0, uint32_t *buf1, uint32_t len);
int current_output_buffer();
int current_output_slot();

#endif

