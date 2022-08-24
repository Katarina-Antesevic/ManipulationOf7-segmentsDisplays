/*
 /*
 /*
 * hps_linux.h
 *
 *  Created on: Apr 23, 2021
 *      Author: mknezic
 */

#ifndef HPS_LINUX_H_
#define HPS_LINUX_H_

#include "socal/hps.h"
#include <stdbool.h>
#include "hps_soc_system.h"

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |   HPS_LED   |  GPIO53  |   GPIO1[24]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_LED_IDX      (ALT_GPIO_1BIT_53)                      // GPIO53
#define HPS_LED_PORT     (alt_gpio_bit_to_pid(HPS_LED_IDX))      // ALT_GPIO_PORTB
#define HPS_LED_PORT_BIT (alt_gpio_bit_to_port_pin(HPS_LED_IDX)) // 24 (from GPIO1[24])
#define HPS_LED_MASK     (1 << HPS_LED_PORT_BIT)

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |  HPS_KEY_N  |  GPIO54  |   GPIO1[25]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_KEY_N_IDX      (ALT_GPIO_1BIT_54)                        // GPIO54
#define HPS_KEY_N_PORT     (alt_gpio_bit_to_pid(HPS_KEY_N_IDX))      // ALT_GPIO_PORTB
#define HPS_KEY_N_PORT_BIT (alt_gpio_bit_to_port_pin(HPS_KEY_N_IDX)) // 25 (from GPIO1[25])
#define HPS_KEY_N_MASK     (1 << HPS_KEY_N_PORT_BIT)

/* The HPS will only use HEX_DISPLAY_COUNT of 7-segment displays */
#define HEX_DISPLAY_COUNT (6)

/* The 7-segment display is active low */
/*
 * 	Maske za 7-segmentne displeje. Najnizi bit predstavlja segment A, najvisi bit je uvijek 0, drugi najvisi bit predstavlja segment G.
 * 	Ukoliko je neki bit jednak 0 odgovarajuci segmenat je upaljen.
 * 	Ukoliko je neki bit jednak 1 odgovarajuci segmenat je ugasen.
 */
#define HEX_DISPLAY_CLEAR 	(0x7f)
#define HEX_DISPLAY_0		(0x40)
#define HEX_DISPLAY_1		(0x79)
#define HEX_DISPLAY_2		(0x24)
#define HEX_DISPLAY_3		(0x30)
#define HEX_DISPLAY_4		(0x19)
#define HEX_DISPLAY_5		(0x12)
#define HEX_DISPLAY_6		(0x02)
#define HEX_DISPLAY_7		(0x78)
#define HEX_DISPLAY_8		(0x00)
#define HEX_DISPLAY_9		(0x18)
#define HEX_DISPLAY_A		(0x08)
#define HEX_DISPLAY_B		(0x03)
#define HEX_DISPLAY_C		(0x46)
#define HEX_DISPLAY_D		(0x21)
#define HEX_DISPLAY_E		(0x06)
#define HEX_DISPLAY_F		(0x0E)
#define HEX_DISPLAY_I		(0x4F)
#define HEX_DISPLAY_J		(0x70)
#define HEX_DISPLAY_L		(0x47)
#define HEX_DISPLAY_N		(0x2B)
#define HEX_DISPLAY_P		(0x0C)
#define HEX_DISPLAY_R		(0x2F)
#define HEX_DISPLAY_T		(0x07)
#define HEX_DISPLAY_U		(0x41)
#define HEX_DISPLAY_V		(0x63)
#define HEX_DISPLAY_H		(0x09) // za ispis greske

#define HEX_COUNTER_MASK	((1 << (4 * HEX_DISPLAY_COUNT)) - 1)

#define NUMBER_OF_LEDS 		3  // 3, 4, 5 ili 6
#define MANUALLY_OR_RANDOM	1  // 0 - rucno, 1 - generisano nasumicno
#define LEN 				6  // duzina serijskog broja

#define BIJELA 0
#define ZUTA   1
#define PLAVA  2
#define CRVENA 3
#define CRNA   4

/*
 * 	Maska za gasenje svih 10 led dioda.
 */
#define LEDS_CLEAR (0b0000000000)
/*
 * 	Maske za paljenje prve 3, 4, 5 i 6 dioda, respektivno.
 */
#define THREE_LEDS	(0b0000000111)
#define FOUR_LEDS	(0b0000001111)
#define FIVE_LEDS	(0b0000011111)
#define SIX_LEDS	(0b0000111111)
#define ALL_LEDS	(0b1111111111)

/*
 * Pomocni niz koji olaksava upisivanje heksadecimalnih vrijednosti na 7-segmentne displeje.
 */
uint32_t hex_display_table[25] = {
	HEX_DISPLAY_0, HEX_DISPLAY_1, HEX_DISPLAY_2, HEX_DISPLAY_3,
	HEX_DISPLAY_4, HEX_DISPLAY_5, HEX_DISPLAY_6, HEX_DISPLAY_7,
	HEX_DISPLAY_8, HEX_DISPLAY_9, HEX_DISPLAY_A, HEX_DISPLAY_B,
	HEX_DISPLAY_C, HEX_DISPLAY_D, HEX_DISPLAY_E, HEX_DISPLAY_F,
	HEX_DISPLAY_I, HEX_DISPLAY_J, HEX_DISPLAY_L, HEX_DISPLAY_N,
	HEX_DISPLAY_P, HEX_DISPLAY_R, HEX_DISPLAY_T, HEX_DISPLAY_U,
	HEX_DISPLAY_V, HEX_DISPLAY_H
};
/*
 * Pomocni niz za ispis zute boje na 7-segmentne displeje.
 */
uint32_t hex_display_yellow[6] = {
	HEX_DISPLAY_2,HEX_DISPLAY_U,HEX_DISPLAY_T,HEX_DISPLAY_A,
	HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR
};
/*
 * Pomocni niz za ispis crne boje na 7-segmentne displeje.
 */
uint32_t hex_display_black[6] = {
		HEX_DISPLAY_C, HEX_DISPLAY_R, HEX_DISPLAY_N, HEX_DISPLAY_A,
		HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR
};
/*
 * Pomocni niz za ispis bijele boje na 7-segmentne displeje.
 */
uint32_t hex_display_white[6] = {
		HEX_DISPLAY_B, HEX_DISPLAY_I, HEX_DISPLAY_J, HEX_DISPLAY_E,
		HEX_DISPLAY_L, HEX_DISPLAY_A
};
/*
 * Pomocni niz za ispis crvene boje na 7-segmentne displeje.
 */
uint32_t hex_display_red[6] = {
		HEX_DISPLAY_C, HEX_DISPLAY_R, HEX_DISPLAY_V, HEX_DISPLAY_E,
		HEX_DISPLAY_N, HEX_DISPLAY_A
};
/*
 * Pomocni niz za ispis plave boje na 7-segmentne displeje.
 */
uint32_t hex_display_blue[6] = {
		HEX_DISPLAY_P, HEX_DISPLAY_L, HEX_DISPLAY_A, HEX_DISPLAY_V,
		HEX_DISPLAY_A, HEX_DISPLAY_CLEAR
};
/*
 * Pomocni niz za ispis FAILED na 7-segmentne displeje.
 */
uint32_t hex_display_failed[6] = {
		HEX_DISPLAY_F, HEX_DISPLAY_A, HEX_DISPLAY_I, HEX_DISPLAY_L,
		HEX_DISPLAY_E, HEX_DISPLAY_D
};
/*
 * Pomocni niz za ispis SUCSES na 7-segmentne displeje.
 */
uint32_t hex_display_sucses[6] = {
		HEX_DISPLAY_5, HEX_DISPLAY_U, HEX_DISPLAY_C, HEX_DISPLAY_5,
		HEX_DISPLAY_E, HEX_DISPLAY_5
};
/*
 * Pomocni niz za ispis 1 greske (2 H na dsiplejima) boje na 7-segmentne displeje.
 */
uint32_t hex_display_2h[6] = {
		HEX_DISPLAY_H, HEX_DISPLAY_H, HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR,
		HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR
};
/*
 * Pomocni niz za ispis 2 greske (4 H na dsiplejima) boje na 7-segmentne displeje.
 */
uint32_t hex_display_4h[6] = {
		HEX_DISPLAY_H, HEX_DISPLAY_H, HEX_DISPLAY_H, HEX_DISPLAY_H,
		HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR
};
/*
 * Pomocni niz za ispis 3 greske (6 H na dsiplejima) boje na 7-segmentne displeje.
 */
uint32_t hex_display_6h[6] = {
		HEX_DISPLAY_H, HEX_DISPLAY_H, HEX_DISPLAY_H, HEX_DISPLAY_H,
		HEX_DISPLAY_H, HEX_DISPLAY_H
};
/*
 * Pomocni niz za gasenje svih 7-segmentnih displeja.
 */
uint32_t hex_display_clear[6] = {
		HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR,
		HEX_DISPLAY_CLEAR, HEX_DISPLAY_CLEAR
};
/*
 * 	Pomocne vrijednosti koje cuvaju staru i novu vrijednost odredjenih promjenljviih da su vidljive u bilo kojem djelu koda.
 */
uint32_t leds_value;
uint32_t switch_value_old;
uint32_t switch_value_new;

/* Physical memory file descriptor */
int fd_dev_mem = 0;

/* Memory-mapped HPS peripherals */
void *hps_gpio = NULL;
size_t hps_gpio_span = ALT_GPIO1_UB_ADDR - ALT_GPIO1_LB_ADDR + 1;
size_t hps_gpio_ofst = ALT_GPIO1_OFST;

/* Memory-mapped FPGA peripherals */
void *h2f_lw_axi_master = NULL;
size_t h2f_lw_axi_master_span = ALT_LWFPGASLVS_UB_ADDR - ALT_LWFPGASLVS_LB_ADDR + 1;
size_t h2f_lw_axi_master_ofst = ALT_LWFPGASLVS_OFST;

/*
 *  Pokazivaci na fpga periferije koje se koriste u aplikaciji.
 */
void *fpga_buttons = NULL;
void *fpga_hex_displays[HEX_DISPLAY_COUNT] = {NULL};
void *fpga_switches = NULL;
void *fpga_leds = NULL;

/*
 * Deklaracije funkcija koje su definisane u .c fajlu.
 */
int generate_num_in_range(int lower, int upper);
bool is_fpga_swiches_pressed(uint32_t switch_number);
void display_yellow();
void display_blue();
void display_red();
void display_black();
void display_white();
void display_2h();
void display_4h();
void display_6h();
void display_failed();
void display_clear();
void display_sucses();
void display_serial_num();
void display_timer();
void handle_timer();
void set_diode_cases();
void check_switches();

void open_physical_memory_device();
void close_physical_memory_device();
void mmap_hps_peripherals();
void munmap_hps_peripherals();
void mmap_fpga_peripherals();
void munmap_fpga_peripherals();
void mmap_peripherals();
void munmap_peripherals();
void setup_hps_gpio();
void handle_hps_led();
bool is_fpga_button_pressed(uint32_t button_number);
void setup_hex_displays();
void set_hex_displays();
void handle_hex_displays();
void setup_leds();
void set_leds(uint32_t value);
void handle_leds();

#endif /* HPS_LINUX_H_ */
