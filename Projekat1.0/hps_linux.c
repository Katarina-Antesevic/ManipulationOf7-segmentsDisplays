#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#include "alt_generalpurpose_io.h"
#include "hps_linux.h"
#include "hwlib.h"
#include "socal/alt_gpio.h"
#include "socal/socal.h"
#include "hps_soc_system.h"

int num_of_leds;
int serial_num[LEN];
int colors[LEN]={5,5,5,5,5,5};

// Definisanje testnih slucajeva
int test1_colors[LEN] = {BIJELA,CRVENA,CRVENA,PLAVA};
int test1_serial_num[LEN] = {7,2,5,9,4,1};
int test2_colors[LEN] = {BIJELA,ZUTA,BIJELA};
int test2_serial_num[LEN] = {1,4,0,3,4,9};
int test3_colors[LEN] = {PLAVA,PLAVA,CRNA,ZUTA,BIJELA,ZUTA};
int test3_serial_num[LEN] = {1,3,8,6,5,2};
int test4_colors[LEN] = {CRNA,ZUTA,PLAVA,ZUTA,PLAVA,BIJELA};
int test4_serial_num[LEN] = {0,9,7,5,2,6};
int test5_colors[LEN] = {ZUTA,ZUTA,ZUTA,ZUTA,BIJELA};
int test5_serial_num[LEN] = {4,5,4,7,9,8};
int test6_colors[LEN] = {BIJELA,PLAVA,CRNA,PLAVA,CRNA,PLAVA};
int test6_serial_num[LEN] = {5,3,7,9,1,3};
int test7_colors[LEN] = {PLAVA,PLAVA,PLAVA};
int test7_serial_num[LEN] = {6,3,7,8,5,1};
int test8_colors[LEN] = {CRVENA,PLAVA,CRVENA,BIJELA,ZUTA,ZUTA};
int test8_serial_num[LEN] = {9,6,3,7,5,8};
int test9_colors[LEN] = {PLAVA,PLAVA,BIJELA,BIJELA,BIJELA,CRNA};
int test9_serial_num[LEN] = {3,2,0,7,6,7};
int test10_colors[LEN] = {BIJELA,CRNA,PLAVA,PLAVA,CRNA,CRVENA};
int test10_serial_num[LEN] = {0,7,3,9,5,8};

// Trenutni broj gresaka (pogresnih izbora prekidaca)
int num_of_errors = 0;

// Vrijednost koja predstavlja ispravan prekidac
int swich;

// Postavljanje vrijednosti tajmera na 120s (2min)
uint32_t timer_counter = 120;

// Vrijednost koja definise zavrsetak aplikacije (0-traje izvrsavanje, 1-kraj izvrsavanja)
int end_of_application = 0;

/*
 *	Funkcija za generisanje slucajnog broja u opsegu [lower, upper].
 */
int generate_num_in_range(int lower, int upper)
{
	int number;
	srand(time(NULL));
	number = (rand() % (upper - lower +1)) + lower;
	return number;
}
/*
 *	Koristi se za definisanje file descriptora i poziva se na pocetku izvrsavanja aplikacije.
 */
void open_physical_memory_device()
{
	fd_dev_mem = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd_dev_mem == -1)
	{
		printf("ERROR: could not open \"/dev/mem\"...\n");
		printf("    errno = %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/*
 * 	Koristi se za mapiranje periferala koji se nalaze na HPS strani Cyclone V cipa.
 */
void mmap_hps_peripherals()
{
	hps_gpio = mmap(NULL, hps_gpio_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, hps_gpio_ofst);
	if (hps_gpio == MAP_FAILED)
	{
		printf("ERROR: hps_gpio mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}
}

/*
 * 	Koristi se za podesavanje pocetne vrijednosti periferija sa HPS strane Cyclone V cipa.
 */
void setup_hps_gpio()
{
	/* Initialize the HPS PIO controller
	 *     Set the direction of the HPS_LED_GPIO bit to "output"
	 *     Set the direction of the HPS_KEY_N GPIO bit to "input"
	 */
	void *hps_gpio_direction = ALT_GPIO_SWPORTA_DDR_ADDR(hps_gpio);
	alt_setbits_word(hps_gpio_direction, ALT_GPIO_PIN_OUTPUT << HPS_LED_PORT_BIT);
	alt_setbits_word(hps_gpio_direction, ALT_GPIO_PIN_INPUT << HPS_KEY_N_PORT_BIT);
}

/*
 * 	Funckija koja se poziva u petlji iz main funkcije i predstavlja algoritam koji kontrolise led diodu sa HPS strane Cyclone V cipa.
 */
void handle_hps_led()
{
	void *hps_gpio_data = ALT_GPIO_SWPORTA_DR_ADDR(hps_gpio);
	void *hps_gpio_port = ALT_GPIO_EXT_PORTA_ADDR(hps_gpio);

	uint32_t hps_gpio_input = alt_read_word(hps_gpio_port) & HPS_KEY_N_MASK;

	/* HPS_KEY_N is active-low */
	bool toggle_hps_led = (~hps_gpio_input & HPS_KEY_N_MASK);

	if (toggle_hps_led)
	{
		uint32_t hps_led_value = alt_read_word(hps_gpio_data);
		hps_led_value >>= HPS_LED_PORT_BIT;
		hps_led_value = !hps_led_value;
		hps_led_value <<= HPS_LED_PORT_BIT;
		alt_replbits_word(hps_gpio_data, HPS_LED_MASK, hps_led_value);
	}
}

/*
 * 	Koristi se za mapiranje periferala koji se nalaze na FPGA strani Cyclone V cipa.
 * 	U nasem slucaju to su dugmadi, prekidaci, led diode i 7-segmentni displeji.
 * 	Koristi bazne adrese perfijerija definisanih u hps_soc_system.h fajlu.
 */
void mmap_fpga_peripherals()
{
	h2f_lw_axi_master = mmap(NULL, h2f_lw_axi_master_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, h2f_lw_axi_master_ofst);

	if(h2f_lw_axi_master == MAP_FAILED)
	{
		printf("ERROR: h2f_lw_axi_master mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	fpga_buttons = h2f_lw_axi_master + BUTTONS_0_BASE;
	fpga_hex_displays[0] = h2f_lw_axi_master + HEX_0_BASE;
	fpga_hex_displays[1] = h2f_lw_axi_master + HEX_1_BASE;
	fpga_hex_displays[2] = h2f_lw_axi_master + HEX_2_BASE;
	fpga_hex_displays[3] = h2f_lw_axi_master + HEX_3_BASE;
	fpga_hex_displays[4] = h2f_lw_axi_master + HEX_4_BASE;
	fpga_hex_displays[5] = h2f_lw_axi_master + HEX_5_BASE;
	fpga_switches = h2f_lw_axi_master + SWITCHES_0_BASE;
	fpga_leds = h2f_lw_axi_master + LEDS_0_BASE;
}

/*
 * 	Koristi se za provjeru da li je pritisnut odredjeni taster odnosno dugme.
 *
 * 	Prima jedan argument koji predstavlja redni broj dugmeta, vrijednosti koje imaju smisla su: 0, 1, 2 i 3.
 * 		0 - najnizi bit odnosno prvi taster sa desne strane
 * 		1 - drugi najnizi bit, odnosno drugi taster sa desne strane
 * 		2 - treci najnizi bit, odnosno drugi taster sa lijeve strane
 * 		3 - cetvrti najnizi bit, odnosno prvi taster sa desne strane
 * 	Vraca boolean reprezentaciju koja je true ako je trazeno dugme pritisnuto a false ako trazeno dugme nije pritisnuto.
 *
 * 	Jedan taster odgovara jednobitnoj vrijednosti, 0 predstavja situaciju kada je taster pritisnut dok 1 predstavlja situaciju kada taster nije pritisnut.
 */
bool is_fpga_button_pressed(uint32_t button_number)
{
	/* Buttons are active-low */
	return ((~alt_read_word(fpga_buttons)) & (1 << button_number));
}

/*
 * 	Koristi se za provjeru pozicija svih 10 prekidaca.
 *
 * 	Direktno vraca reprezentaciju prekidaca kao uint32_t vrijednost. Nama su zanimljivi prvih 10 bita ondnost 10 najnizih bita.
 * 	Mogu se koristiti binarni operatori kao i u funkciji is_fpga_button_pressed za provjeru manjeg broja prekidaca.
 * 	Najnizi bit predstavlja prvi prekidac sa desne strane, najvisi od prvih 10 bita predstavlja prvi prekidac sa lijeve strane.
 * 	Reprezentacije ostalih 8 bita je lako za zakljucit.
 *
 * 	Kada je prekidac u donjem polozaju, odnosno iskljucen, odgovarajuci bit ce imati vrijednost 0. U suprotnomm ce imati vriejdnost 1.
 */
uint32_t fpga_swiches_position()
{
	return alt_read_word(fpga_switches);
}
/*
 * 	Koristi se za provjeru da li je prebacen odredjeni prekidac (oznacen brojem switch_number).
 */
bool is_fpga_swiches_pressed(uint32_t switch_number)
{
	/* Switches are active-high */
	return ((alt_read_word(fpga_switches)) & (1 << switch_number));
}
/*
 * 	Koristi se za postavljanje pocetnih vrijednosti 7-segmentnih displeja. Pocetno stanje je predstavljeno kada su svi segmenti na displeju ugaseni.
 */
void setup_hex_displays()
{
	int i;
	/* Turn all hex displays off */
	for(i=0; i < HEX_DISPLAY_COUNT; i++)
	{
		alt_write_word(fpga_hex_displays[i], HEX_DISPLAY_CLEAR);
	}
}
/*
 * 	Prikaz zute boje na 7-segmentne displeje.
 */
void display_yellow()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_yellow[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz plave boje na 7-segmentne displeje.
 */
void display_blue()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_blue[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz crvene boje na 7-segmentne displeje.
 */
void display_red()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_red[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz crne boje na 7-segmentne displeje.
 */
void display_black()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_black[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz bijele boje na 7-segmentne displeje.
 */
void display_white()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_white[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz poruke o jednoj greski (2H) na 7-segmentne displeje.
 */
void display_2h()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_2h[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz poruke o dvije greske (4H) na 7-segmentne displeje.
 */
void display_4h()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_4h[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz poruke o tri greske (6H) na 7-segmentne displeje.
 */
void display_6h()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_6h[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz poruke FAILED na 7-segmentne displeje.
 */
void display_failed()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_failed[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz poruke SUCSES na 7-segmentne displeje.
 */
void display_clear()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_clear[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za gasenje svih 7-segmentnih displeja.
 */
void display_sucses()
{
	uint32_t hex_display_index;

	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_sucses[HEX_DISPLAY_COUNT - hex_display_index-1];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz serijskog broja na 7-segmentne displeje.
 */
void display_serial_num()
{
	uint32_t hex_display_index;
	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		int number = serial_num[hex_display_index];
		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_table[number];
		alt_write_word(fpga_hex_displays[HEX_DISPLAY_COUNT - hex_display_index - 1], hex_value_to_write);
	}
}
/*
 * 	Funkcija za prikaz vrijednosti tajmera na 7-segmentne displeje.
 */
void display_timer()
{
	char current_char[2] = " \0";
	char hex_counter_hex_string[HEX_DISPLAY_COUNT + 1];

	/* Get hex string representation of input value on HEX_DISPLAY_COUNT 7-segment displays */
	snprintf(hex_counter_hex_string, HEX_DISPLAY_COUNT + 1, "%0*d", HEX_DISPLAY_COUNT, (unsigned int)timer_counter);

	uint32_t hex_display_index;
	for (hex_display_index = 0; hex_display_index < HEX_DISPLAY_COUNT; hex_display_index++)
	{
		current_char[0] = hex_counter_hex_string[HEX_DISPLAY_COUNT - hex_display_index - 1];

		/* Get decimal representation for this 7-segment display */
		uint32_t number = (uint32_t)strtol(current_char, NULL, 16);

		/* Use lookup table to find active-low value representing number on the 7-segment display */
		uint32_t hex_value_to_write = hex_display_table[number];

		alt_write_word(fpga_hex_displays[hex_display_index], hex_value_to_write);
	}
}
/*
 * 	Funkcija za umanjivanje vrijednosti tajmera.
 */
void handle_timer()
{
	if(timer_counter==0)
	{
		display_failed();
		end_of_application = 1;
	}
	else
	{
		timer_counter--;
	}
}
/*
 * 	Funkcija koja obradjuje sve slucajeve za prebacivanje prekidaca.
 */
void set_diode_cases()
{
	switch (num_of_leds){
		case 3:
		{
			int red_counter=0;
			int blue_counter=0;
			//int white_counter=0;
			int last_blue_position=6;
			int i;
			for(i=0;i<num_of_leds;i++)
			{
				if(colors[i]==3)
					red_counter++;
				if(colors[i]==2)
				{
					blue_counter++;
					if(blue_counter>1)
					{
						last_blue_position=i;
					}
				}
			}

			if(red_counter==1)
			{
				swich=1;
			}
			else if(colors[num_of_leds-1]==0)
			{
				swich=2;
			}
			else if(blue_counter>1)
			{
				swich=last_blue_position;
			}
			else
			{
				swich=2;
			}

			break;
		}
		 case 4:
		 {
		 	 int red_counter=0;
		 	 int blue_counter=0;
		 	 int yellow_counter=0;
		 	 int last_red_position=6;
		 	 int i;
		 	 for(i=0;i<num_of_leds;i++)
		 	 {
		 		if(colors[i]==3)
		 		{
		 			red_counter++;
		 			if(red_counter>1)
		 			{
		 				last_red_position=i;
		 			}
		 		}
		 		if(colors[i]==2)
		 			blue_counter++;
		 		if(colors[i]==1)
		 			yellow_counter++;
		 	 }

			if(red_counter>1 && (serial_num[LEN-1] % 2 != 0))
			{
				swich=last_red_position;
			}
			else if(red_counter==0 && colors[num_of_leds-1]==1)
			{
				swich=0;
			}
			else if(blue_counter==1)
			{
				swich=0;
			}
			else if(yellow_counter>1)
			{
				swich=3;
			}
			else
			{
				swich=1;
			}
		 		break;
		 }
		  case 5:
		  {
			  int red_counter=0;
		  	  int black_counter=0;
		  	  int yellow_counter=0;
		  	  int i;
		  	  for(i=0;i<num_of_leds;i++)
		  	  {
		  		 if(colors[i]==3)
		  		 	red_counter++;
		  		 if(colors[i]==4)
		  		 	black_counter++;
		  		 if(colors[i]==1)
		  		 	yellow_counter++;
		  	  }

		  	  if(colors[num_of_leds-1]==4 && (serial_num[LEN-1] % 2 != 0))
		  	  {
		  		  swich=3;
		  	  }
		  	  else if(red_counter==1 && yellow_counter>1)
		  	  {
		  		  swich=0;
		  	  }
		  	  else if(black_counter==0)
		  	  {
		  		  swich=1;
		  	  }
		  	  else
		  	  {
		  		  swich=0;
		  	  }
		  		  break;
		  	}
		  case 6:
		  {
			  int red_counter=0;
			  int white_counter=0;
			  int yellow_counter=0;
			  int i;
			  for(i=0;i<num_of_leds;i++)
			  {
		  		  if(colors[i]==3)
		  		  	red_counter++;
		  		  if(colors[i]==0)
		  		  	white_counter++;
		  		  if(colors[i]==1)
		  		  	yellow_counter++;
			  }
			  if(yellow_counter==0 && (serial_num[LEN-1] % 2 != 0))
			  {
				  swich=2;
			  }
			  else if(yellow_counter==1 && white_counter>1)
			  {
		  		  swich=3;
			  }
			  else if(red_counter==0)
			  {
		  		  swich=5;
			  }
			  else
			  {
		  		  swich=3;
			  }
		  		  break;
		  }
		}
}
/*
 * 	Koristi se za promjenu vrijednosti koje su upisane u 7-segmentne displeje.
 */
void handle_hex_displays()
{

	uint32_t switches_positions = fpga_swiches_position();

	// Ako su istovremeno prebacena 2 ili 3 prekidaca (od zadnja 3 prekidaca)
	if((is_fpga_swiches_pressed(7) && is_fpga_swiches_pressed(8))
		|| (is_fpga_swiches_pressed(7) && is_fpga_swiches_pressed(9))
		|| (is_fpga_swiches_pressed(8) && is_fpga_swiches_pressed(9))
		|| ( is_fpga_swiches_pressed(7) && is_fpga_swiches_pressed(8) && is_fpga_swiches_pressed(9)))
	{
		display_clear();
	}
	// Ako je prebacen osmi prekidac
	else if(is_fpga_swiches_pressed(7))
	{
		display_timer();
	}
	// Ako je prebacen deveti prekidac
	else if(is_fpga_swiches_pressed(8))
	{
		display_serial_num();
	}
	// Ako je prebacen deseti prekidac
	else if(is_fpga_swiches_pressed(9))
	{
		if(num_of_errors==1)
			display_2h();
		else if(num_of_errors==2)
			display_4h();
		else if(num_of_errors==3)
			display_6h();
	}
	// Ako je broj gresaka jednak 3
	else if(num_of_errors==3)
	{
		display_6h();
		usleep(5*ALT_MICROSECS_IN_A_SEC);
		display_failed();
		end_of_application = 1;
	}
	// Ako je prebacen ispravan (pravi) prekidac
	else if((swich==1 && is_fpga_swiches_pressed(1)) || (swich==2 && is_fpga_swiches_pressed(2))
			|| (swich==0 && is_fpga_swiches_pressed(0)) || (swich==3 && is_fpga_swiches_pressed(3))
			|| (swich==5 && is_fpga_swiches_pressed(5)))
	{
		display_sucses();
		end_of_application = 1;
	}
	// U suprotnom se na displeju prikazuju boje dioda u ciklicnom redoslijedu
	else
	{
		int i=0;
		for(i=0;i<num_of_leds;i++)
		{
			if(colors[i] == 0)
			{
				display_white();
				usleep(ALT_MICROSECS_IN_A_SEC);
			}

			else if(colors[i] == 1)
			{
				display_yellow();
				usleep(ALT_MICROSECS_IN_A_SEC);
			}

			else if(colors[i] == 2)
			{
				display_blue();
				usleep(ALT_MICROSECS_IN_A_SEC);
			}
			else if(colors[i] == 3)
			{
				display_red();
				usleep(ALT_MICROSECS_IN_A_SEC);
			}

			else
			{
				display_black();
				usleep(ALT_MICROSECS_IN_A_SEC);
			}

	}
  }
}
/*
 * 	Koristi se za postavljanje pocetnih vrijednosti led dioda.
 */
void setup_leds()
{
	/* Paljenje razlicitog broja dioda u zavisnosti od generisanog broja dioda */
	switch (num_of_leds){
	    case 3:
	    	alt_write_word(fpga_leds, THREE_LEDS);
	    	break;

	    case 4:
	    	alt_write_word(fpga_leds, FOUR_LEDS);
	    	break;

	    case 5:
	    	alt_write_word(fpga_leds, FIVE_LEDS);
	    	break;

	    case 6:
	    	alt_write_word(fpga_leds, SIX_LEDS);
	    	break;

	    default:
	    	// default - gasenje svih dioda
	    	alt_write_word(fpga_leds, LEDS_CLEAR);
	}
}

/*
 * 	Koristi se za promjenu vrijednosti koje su upisane u diode.
 *
 * 	Prima decimalnu vrijednost 32-bitnog broja koju saljemo na diode
 *
 * 	Ovom funkcijom mijenjamo vrijednosti svih 10 dioda. Kada je potrebno promjeniti vrijednosti samo malom broju dioda
 * 	bolje je koristiti binarne operatore kao u slucaju citanja vrijednosti tastera.
 */
void set_leds(uint32_t value)
{
	leds_value = value;
	alt_write_word(fpga_leds, value);
}

/*
 * 	Funckija koja se poziva u petlji iz main funkcije i predstavlja algoritam koji kontrolise led diode.
 *
 * 	Cita vrijednost prekidaca i ukoliko je doslo do promjene stanja bar jednog prekidaca nova vriejdnost prekidaca se salje na diode,
 * 	tako da diode odgovaraju prekidacima.
 */
void handle_leds()
{
	switch_value_new = fpga_swiches_position();
	if(switch_value_old != switch_value_new){
		switch_value_old = switch_value_new;
		set_leds(switch_value_new);
	}
}

/*
 * 	Koristi se za oslobadjanje memorije koju zauzimaju periferije sa FPGA strane Cyclone V cipa.
 */
void munmap_fpga_peripherals()
{
	int i;

	if(munmap(h2f_lw_axi_master, h2f_lw_axi_master_span) != 0)
	{
		printf("ERROR: h2f_lw_axi_master munmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	h2f_lw_axi_master = NULL;
	fpga_buttons = NULL;
	fpga_switches = NULL;
	fpga_leds = NULL;

	for (i=0; i < HEX_DISPLAY_COUNT; i++)
	{
		fpga_hex_displays[i] = NULL;
	}

}

/*
 * 	Koristi se za oslobadjanje memorije koju zauzimaju periferije sa FPGA strane Cyclone V cipa.
 */
void munmap_hps_peripherals()
{
	if(munmap(hps_gpio, hps_gpio_span) != 0)
	{
		printf("ERROR: hps_gpio munmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	hps_gpio = NULL;
}

/*
 * 	Koristi se za objedinjavanje funkcija za mapiranje periferija na Cyclone V cipu. Poziva se na pocetku izvrsavanja aplikacije nakon
 * 	open_physical_memory_device funkcije.
 */
void mmap_peripherals() {
    mmap_hps_peripherals();
    mmap_fpga_peripherals();
}

/*
 * 	Koristi se za objedinjavanje funkcija za oslobadjavanje zauzete memorije od strane periferija na Cyclone V cipu.
 * 	Poziva se na kraju izvrsavanja aplikacije ali prije close_physical_memory_device funkcije
 */
void munmap_peripherals() {
    munmap_hps_peripherals();
    munmap_fpga_peripherals();
}

/*
 *	Koristi se za oslobadjanje file descriptora i poziva se na kraju izvrsavanja aplikacije.
 */
void close_physical_memory_device()
{
	close(fd_dev_mem);
}
/*
 * 	Funkcija koja uvecava broj gresaka ukoliko je prebacen pogresan prekidac za odgovarajuci broj dioda.
 */
void check_switches()
{
	switch_value_new = fpga_swiches_position() & 0b111111; // Koristimo masku jer ne uzimamo u obzir prvih 6 prekidaca

	if(switch_value_old == switch_value_new)
	{
		return;
	}

	switch_value_old = switch_value_new;

	if(switch_value_new == 0)
	{
		return;
	}

	switch(num_of_leds)
	{
	case 3:
	{
		if(swich==1)
		{
			if(switch_value_new!=2)
				num_of_errors++;

		}

		else if(swich==2)
		{
			if(switch_value_new!=4)
				num_of_errors++;
		}
			break;
	  }
	case 4:
	{
		if(swich==0)
		{
			if(switch_value_new!=1)
				num_of_errors++;
		}
		if(swich==1)
		{
			if(switch_value_new!=2)
				num_of_errors++;
		}
		else if(swich==2)
		{
			if(switch_value_new!=4)
				num_of_errors++;
		}
		else if(swich==3)
		{
			if(switch_value_new!=8)
				num_of_errors++;
		}
			break;
	  }
	case 5:
	{
		if(swich==0)
		{
			if(switch_value_new!=1)
				num_of_errors++;
		}
		if(swich==1)
		{
			if(switch_value_new!=2)
				num_of_errors++;
		}
		else if(swich==3)
		{
			if(switch_value_new!=8)
				num_of_errors++;
		}
			break;
	  }
	case 6:
	{
		if(swich==2)
		{
			if(switch_value_new!=4)
				num_of_errors++;
		}
		else if(swich==3)
		{
			if(switch_value_new!=8)
				num_of_errors++;
		}
		else if(swich==5)
		{
			if(switch_value_new!=32)
				num_of_errors++;
		}
			break;
	  }
	}
}
/*
 * Glavna funkcija koja u beskonacnoj petlji cita vriejdnosti ulaznih periferija i upisuje vrijednosti u izlazne periferije.
 * Koristi usleep funkciju cime uspava thread za 1s.
 * Pomocu usleep funkcije ogranicavamo da promjenu vrijednosti periferijama na jedanput u sekundi.
 */
int main()
{
	printf("DE1-SoC linux demo\n");

	open_physical_memory_device();
	mmap_peripherals();

	if(MANUALLY_OR_RANDOM) // Rucni unos ili nasumicno generisanje (0-rucno, 1-nasumicno)
	{
		// Generisanje slucajnog broja izmedju 3 i 6
		num_of_leds = generate_num_in_range(3,6);

		// Generisanje onoliko boja koliki je izgenerisani broj dioda
		int i=0;
		srand(time(NULL));
		for(i = 0; i < num_of_leds; i++)
			colors[i] = ((rand() % 5));

		// Generisanje serijskog broja od 6 cifara
		int j;
		srand(time(NULL));
		for(j = 0; j < LEN; j++)
			serial_num[j] = ((rand() % 10));
	}
	// Rucno definisanje broja dioda, boja i serijskog broja
	else
	{
		// Rucno definisanje broja led dioda
		num_of_leds = NUMBER_OF_LEDS;

		// Postavljanje boja prema odgovarajucem testnom slucaju
		int i;
		for(i = 0; i < num_of_leds; i++)
		{
			colors[i] = test7_colors[i];
		}
		// Postavljanje serijskog broja prema odgovarajucem testnom slucaju
		int j;
		for(j = 0; j < LEN; j++)
			serial_num[j] = test7_serial_num[j];

	}

	setup_hps_gpio();
	setup_hex_displays();
	setup_leds();
	set_diode_cases();


	while(end_of_application == 0)
	{
		//printf("%d \n",fpga_swiches_position());
		check_switches();
		handle_hex_displays();
		handle_timer();
		handle_hps_led();
		//handle_leds();
		// Vrijednost proslijedjena usleep funkciji je 1s zbog mjerenja proteklog vremena tajmera
		usleep(ALT_MICROSECS_IN_A_SEC);
	}

	munmap_peripherals();
	close_physical_memory_device();

	return 0;
}
