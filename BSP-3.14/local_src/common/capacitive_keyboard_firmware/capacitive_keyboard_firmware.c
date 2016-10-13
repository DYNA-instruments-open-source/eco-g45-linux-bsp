/*----------------------------------------------------------------------------
compiler information
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
include files
----------------------------------------------------------------------------*/
#include <avr/io.h>
#include <avr/interrupt.h>
#define __delay_cycles(n)     __builtin_avr_delay_cycles(n)
#define __enable_interrupt()  sei()
#include "touch_api.h"
#include "i2c_slave.h"
#include "capacitive_keyboard.h"

/*----------------------------------------------------------------------------
manifest constants
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
type definitions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
prototypes
----------------------------------------------------------------------------*/
/*  initialise host app, pins, watchdog, etc    */
static void init_system( void );
/*  configure timer ISR to fire regularly   */
static void init_timer_isr( void );

/*----------------------------------------------------------------------------
Structure Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Clock is 8MHz, Timer Counter clk is 1MHz = 1000 Ticks ps ms
----------------------------------------------------------------------------*/
#define TICKS_PER_MS          1000u
#define MEASUREMENT_PERIOD_MS 5u		// (5..65)
#define BURST_LENGTH          64

/*----------------------------------------------------------------------------
global variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
extern variables
----------------------------------------------------------------------------*/
/* This configuration data structure parameters if needs to be changed will be
changed in the qt_set_parameters function */
extern qt_touch_lib_config_data_t qt_config_data;
/* measurement data */
extern qt_touch_lib_measure_data_t qt_measure_data;
qt_touch_lib_measure_data_t *pqt_measure_data = &qt_measure_data;
/* Get sensor delta values */
extern int16_t qt_get_sensor_delta( uint8_t sensor);

extern y_line_info_t y_line_info[NUM_Y_LINES];
extern x_line_info_t x_line_info[NUM_X_LINES];


/* Fill out the X-Line masks on the X- Port selected.
 * The order of X - Line numbering follows from the way the
 * X-Lines are filled as below.
 * Here, X0,X1,X2,X3,X4,X5,X6,X7 are on port-pin specified.
 * 1 - if to specify if X line is on PORT_X_1,pin on the same selected port.
 * 2 - if to specify if X line is on PORT_X_2,pin on the same selected port.
 * 3 - if to specify if X line is on PORT_X_3,pin on the same selected port.
 *
 * Note: 1. The Number entries should be based on NUM_X_LINES
 *          4 entries when NUM_X_LINES =4 and
 *          8 entries when NUM_X_LINES=8
 */
x_line_info_t x_line_info[NUM_X_LINES]= {
    FILL_OUT_X_LINE_INFO(  1,0u ),
    FILL_OUT_X_LINE_INFO(  1,1u ),
    FILL_OUT_X_LINE_INFO(  1,2u ),
    FILL_OUT_X_LINE_INFO(  1,3u ),
    FILL_OUT_X_LINE_INFO(  1,4u ),
    FILL_OUT_X_LINE_INFO(  1,5u ),
    FILL_OUT_X_LINE_INFO(  1,6u ),
    FILL_OUT_X_LINE_INFO(  1,7u )
};

/* Fill out the Y-Line masks on the Y- Line port selected
 * The order of Y - Line numbering follows from the way the
 * Y-Lines are filled as below
 * Here, Y0,Y1,Y2,Y3 on 0,1,2,3
 * Note: 1. The Number entries should be based on NUM_X_LINES
 *          2 entries when NUM_Y_LINES=2
 *          4 entries when NUM_Y_LINES=4
 *          8 entries when NUM_Y_LINES=8
 */
y_line_info_t y_line_info[NUM_Y_LINES]= {
    FILL_OUT_Y_LINE_INFO(  0u ),
    FILL_OUT_Y_LINE_INFO(  1u )
};

/*----------------------------------------------------------------------------
static variables
----------------------------------------------------------------------------*/
/* flag set by timer ISR when it's time to measure touch */
static volatile uint8_t time_to_measure_touch = 0u;
/* current time, set by timer ISR */
static volatile uint16_t current_time_ms_touch = 0u;

/*============================================================================
Name    :   main
------------------------------------------------------------------------------
Purpose :   main code entry point
Input   :
Output  :
Notes   :
============================================================================*/
int main(int argc, char *argv[])
{
	uint16_t i = 0;
		
	/* status flags to indicate the re-burst for library */
	uint16_t status_flag = 0u;
	uint16_t burst_flag = 0u;

	/* Initialize the !CHANGE line to input (= high level)
	 * and deactivate pull-up */
    DDR_CHANGE  &= (uint8_t)(~CHANGE);
	PORT_CHANGE &= (uint8_t)(~CHANGE);
	/* initialise host app, pins, watchdog, etc */
	//init_system();
	
	/* initialize default values for at42qt2160 */
	memory_map.array[HOSTREG_CHIPID] = 0x11;
	memory_map.array[HOSTREG_VERSION] = 0x40;
	memory_map.array[HOSTREG_SUB_REVISION] = 0x01;

	/* Initialize the LEDs off */
	memory_map.array[HOSTREG_GPIO_GPO_DRIVE_1] = 0x00;
	DDRB = 0xFF;
	PORTB = memory_map.array[HOSTREG_GPIO_GPO_DRIVE_1];

	/* Initial key status is released */ 
	memory_map.array[HOSTREG_KEY_STATUS_1] = 0x00;

	/* configure the three keys */
	qt_enable_key( CHANNEL_0, AKS_GROUP_1, 20u, HYST_6_25 );
	qt_enable_key( CHANNEL_1, AKS_GROUP_1, 20u, HYST_6_25 );
	qt_enable_key( CHANNEL_2, AKS_GROUP_1, 20u, HYST_6_25 );

	/*  Address to pass address of user functions   */
	/*  This function is called after the library has made capacitive measurements,
	 *   but before it has processed them. The user can use this hook to apply filter
	 *   functions to the measured signal values.(Possibly to fix sensor layout faults)    */
	qt_filter_callback = 0;

	/* initialise touch sensing */
	qt_init_sensing();

	/*  Set the parameters like recalibration threshold, Max_On_Duration etc in this function by the user */
	qt_config_data.qt_di                     = DEF_QT_DI; /* 4 counts */
	qt_config_data.qt_neg_drift_rate         = 160; /* 160 * 200 ms = 32 sec. Negative Drift Compensation of the AT42QT2160 */
	qt_config_data.qt_pos_drift_rate         = 40; /* 160 * 200 ms = 32 sec. Positive Drift Compensation of the AT42QT2160 */
	//qt_config_data.qt_max_on_duration        = DEF_QT_MAX_ON_DURATION;
	//qt_config_data.qt_max_on_duration        = 204; /* 204 * 200 ms = 40.8 sec., Negative Recall Delay of the AT42QT2160 */
	qt_config_data.qt_max_on_duration        = 0; /* 0 = off, Negative Recall Delay of the AT42QT2160 */
	qt_config_data.qt_drift_hold_time        = DEF_QT_DRIFT_HOLD_TIME; /* 4 sec., DHT of the AT42QT2160 */
	qt_config_data.qt_recal_threshold        = DEF_QT_RECAL_THRESHOLD; /* does not exist in the AT42QT2160 */
	qt_config_data.qt_pos_recal_delay        = DEF_QT_POS_RECAL_DELAY; /* does not exist in the AT42QT2160 */

	/* set default burst length */
	/* 3 keys */
	for(i=0; i<3; i++)
	{
		qt_burst_lengths[i] = BURST_LENGTH;
	}

	/* all other disabled */
	for(i=3; i<16; i++)
	{
		qt_burst_lengths[i] = 0;
	}

	/* recalibrate */
	qt_calibrate_sensing();	
	
	/* configure timer ISR to fire regularly */
	init_timer_isr();

	/* initialize i2c slave to operate on slave address 0x0D, no common calls*/
	i2c_slave_init(); 

	/* enable interrupts */
	sei();
	
	/* start transceiver */
	i2c_slave_start_transceiver();

	while(1) {
	
		if(time_to_measure_touch)
		{
			/*  clear flag: it's time to measure touch  */
			time_to_measure_touch = 0u;
			status_flag = qt_measure_sensors(current_time_ms_touch);
			burst_flag = status_flag & QTLIB_BURST_AGAIN;
			
			/* report only new events if no reburst is needed */
			if(burst_flag == 0u)
			{
				/* Non time-critical application code goes here */
				/* report a key status change if the key status of the keys have changed */
				if(memory_map.array[HOSTREG_KEY_STATUS_1] != qt_measure_data.qt_touch_status.sensor_states[0])
				{
					memory_map.array[HOSTREG_KEY_STATUS_1] = qt_measure_data.qt_touch_status.sensor_states[0];
					/* signal !CHANGE  */
					DDR_CHANGE |= (uint8_t)(CHANGE);
				}
			}
			
			/* capture changes of the registers */
			/* GPO drives (X0 -X7) */
			if(memory_map.array[HOSTREG_GPIO_GPO_DRIVE_1] != PORTB)
			{
				DDRB = 0xFF;
				PORTB = memory_map.array[HOSTREG_GPIO_GPO_DRIVE_1];
			}

			if (!i2c_slave_transceiver_busy())                              
			{
				i2c_slave_start_transceiver();
			}		
		}		
	}
	return 0;
}

/*============================================================================
Name    :   init_timer_isr
------------------------------------------------------------------------------
Purpose :   configure timer ISR to fire regularly
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
static void init_timer_isr( void )
{
   /*  set timer compare value (how often timer ISR will fire) */
   OCR1A = (TICKS_PER_MS * MEASUREMENT_PERIOD_MS);
   /*  enable timer ISR */
   TIMSK1 |= (1u << OCIE1A);
   /*  timer prescaler = system clock / 8  */
   TCCR1B |= (1u << CS11);
   /*  timer mode = CTC (count up to compare value, then reset)    */
   TCCR1B |= (1u << WGM12);
}

/*============================================================================
Name    :   init_system
------------------------------------------------------------------------------
Purpose :   initialise host app, pins, watchdog, etc
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
static void init_system( void )
{
   /*  run at 4MHz */
   CLKPR = 0x80u;
   CLKPR = 0x01u;
   /*  disable pull-ups    */
   MCUCR |= (1u << PUD);
}

/*============================================================================
Name    :   timer_isr
------------------------------------------------------------------------------
Purpose :   timer 1 compare ISR
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/
ISR(TIMER1_COMPA_vect)
{
   /*  set flag: it's time to measure touch    */
   time_to_measure_touch = 1u;
   /*  update the current time */
   current_time_ms_touch += MEASUREMENT_PERIOD_MS;
}
