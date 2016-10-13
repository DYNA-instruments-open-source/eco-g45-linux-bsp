#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2c_slave.h"
#include "capacitive_keyboard.h"

#define NOT_WRITING 0
#define WRITING 	1

/* Address pointer and reset value */
static unsigned char address_pointer;
static unsigned char base_address_pointer;
struct memory_map_t memory_map;

static unsigned char i2c_state    = I2C_NO_STATE;  /* state byte, default set to I2C_NO_STATE */
static unsigned char i2c_busy = 0;
static unsigned char i2c_com = NOT_WRITING;

union i2c_status_reg_t i2c_status_reg = {0};       /* i2c_status_reg is defined in i2c_slave.h */

static unsigned char sent_status_out[NUM_STATUS_BYTES];

#define DONT_MATCH 0
#define MATCH 1

extern uint16_t interrupt_in_progress;

unsigned char comms_match(void)
{
	int i = 0;
	unsigned char ret = DONT_MATCH;
	for(i=FIRST_STATUS_BYTE; i<FIRST_STATUS_BYTE + NUM_STATUS_BYTES; i++)
	{
		if(sent_status_out[i-FIRST_STATUS_BYTE] == memory_map.array[i])
		{
			ret = MATCH;
		}
		else
		{
			ret = DONT_MATCH;
			goto out;
		}
	}
out:
	return ret;
}

void i2c_slave_init(void)
{
	if( PINC & 0x08u )
	{
		if ( PINC & 0x04u )
		{
			TWAR = (uint8_t)(0x6B << 1); /* 1 1 */
		}
		else
		{
			TWAR = (uint8_t)(0x44 << 1); /* 1 0 */
		}
	}
	else if ( PINC & 0x04u )
	{
		TWAR = (uint8_t)(0x17 << 1); /* 0 1 */
	}
	else
	{
		TWAR = (uint8_t)(0x0D << 1); /* 0 0 */
	}
	TWCR = (1<<TWEN)|                              /* enable I2C-interface and release I2C pins */
		   (0<<TWIE)|(0<<TWINT)|                   /* disable I2C interupt */
           (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|        /* do not ACK on any requests, yet */
           (0<<TWWC);                                 
	i2c_busy = 0;
}

void i2c_slave_start_transceiver(void)
{
	while (i2c_slave_transceiver_busy()) {}             /* wait until TWI is ready for next transmission */
	i2c_state         = I2C_NO_STATE ;
	TWCR = (1<<TWEN)|                             /* i2c Interface enabled */
		   (1<<TWIE)|(1<<TWINT)|                  /* enable TWI Interupt and clear the flag */
           (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|       /* prepare to ACK next time the Slave is addressed */
           (0<<TWWC);                             
		   i2c_busy = 0;
}

unsigned char i2c_slave_transceiver_busy(void)
{
	return i2c_busy;
}

unsigned char i2c_slave_get_state_info(void)
{
	while (i2c_slave_transceiver_busy()){}            /* wait until I2C has completed the transmission */
	return (i2c_state);                         /* return error state */ 
}

ISR(TWI_vect)
{
	int i = 0;
	switch(TWSR)
	{
		/* this is correct that both of these cases run the same code
                  * 0xA8 - we have been addressed to read
                  * 0xB8 - we have previously sent data
                  * either way want to load TWDR with the data to be read */
		case I2C_STX_ADR_ACK: /* own SLA+R has been received, ACK has been returned */
		case I2C_STX_DATA_ACK: /* data byte in TWDR has been transmitted, ACK has been received */ 
			/* If we had previously been sent data but host NACK'd we know that
			 * that was the last byte it intended to read - prevents incorrect
			 * clearing of the change line */
			/* Determine where in the address map we are pointing */
			if(I2C_MAP_LAST_WRITE_ADDRESS >= address_pointer)
			{
				/* pointing within readable range */
				/* load data to sent */
				TWDR = memory_map.array[address_pointer];

				if((address_pointer >= FIRST_STATUS_BYTE) &&
                   (address_pointer < (FIRST_STATUS_BYTE + NUM_STATUS_BYTES)))
				{
					/* Status bytes */
					if( HOSTREG_KEY_STATUS_1 == address_pointer ) /* General Status byte*/
					{
						/* clear the overrun and reset flags */
						//memory_map.array[HOSTREG_GENERAL_STATUS] &= (unsigned char)(~RESET_FLAG);
					}

					sent_status_out[address_pointer - FIRST_STATUS_BYTE] =
					memory_map.array[address_pointer];
				}

				if(MATCH == comms_match())
				{
					/* all status bytes are read, relase !CHANGE */
					DDR_CHANGE &= (uint8_t)(~CHANGE);
					/* reset the status array for next check */
					for(i=0; i<NUM_STATUS_BYTES; i++)
						sent_status_out[i] = 0;
				}
				/* flag any particular commands that need actioning */
			}
			else
			{ /* do nothing as either pointing outside readable ranges */
				TWDR = 0u;   /* send 0x00 */
			}

			address_pointer++;       /* point to next location */

			i2c_com = NOT_WRITING;      /* flag that there is an ongoing com. */

			/* data byte will be transmitted and ACK should be received */
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
				   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // 
				   (0<<TWWC);                                 //
			i2c_busy = 1;
		break;

		case I2C_STX_DATA_NACK: /* data byte in TWDR has been transmitted, NACK has been received */
			TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
				   (1<<TWIE)|(1<<TWINT)|                      // Keep interrupt enabled and clear the flag
                   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Answer on next address match
                   (0<<TWWC);                                 //
			i2c_busy = 0;   // Transmit is finished, we are not busy anymore
		break;
		
		case I2C_SRX_ADR_ACK: /* own SLA+W has been received ACK has been returned */
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
                   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Expect ACK on this transmission
                   (0<<TWWC);  
			i2c_busy = 1;
			i2c_com = NOT_WRITING;
		break;

		case I2C_SRX_ADR_DATA_ACK: /* previously addressed with own SLA+W, data has been received, ACK has been returned */
			if( WRITING == i2c_com ) /* are we writing setup data? */
			{
				/* Determing which section of address map we are pointing to */
				if((I2C_MAP_FIRST_WRITE_ADDRESS <= address_pointer) &&
				   (I2C_MAP_LAST_WRITE_ADDRESS >= address_pointer))
				{
					/* pointing within writeable section of common map */
					memory_map.array[address_pointer] = TWDR;
					/* this addition could be done with a logical OR
					 * assumes common range starts at zero */
				}
				else
				{
					/* do nothing as pointing outside writeable range */
				}

				address_pointer++;       /* point to next location */
			}
			else
			{
				/* First Address Write
				 * assign address pointer */
				base_address_pointer = TWDR;
				address_pointer      = base_address_pointer;
			}

			i2c_com = WRITING;      /* flag that there is an ongoing com. */
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
				   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after next reception
				   (0<<TWWC);                                 // 
			i2c_busy = 1;
 		break;

		case I2C_SRX_STOP_RESTART: /* a STOP condition or repeated START condition has been received while still addressed as slave */
			TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
				   (1<<TWIE)|(1<<TWINT)|                      // Enable interrupt and clear the flag
					(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Wait for new address match
					(0<<TWWC);                                 //
			i2c_busy = 0;  // We are waiting for a new address match, so we are not busy
		break;
		
		case I2C_SRX_ADR_DATA_NACK:      /* previously addressed with own SLA+W; data has been received, NOT ACK has been returned */
		case I2C_SRX_GEN_DATA_NACK:      /* previously addressed with general call; data has been received, NOT ACK has been returned */
		case I2C_STX_DATA_ACK_LAST_BYTE: /* last data byte in TWDR has been transmitted (TWEA = 0), ACK has been received */
		case I2C_NO_STATE:               /* no relevant state information available, TWINT = 0 */
		case I2C_BUS_ERROR:         	 /* bus error due to an illegal START or STOP condition */
			i2c_state = TWSR;                 /* store TWI State as errormessage, operation also clears noErrors bit */
			TWCR =   (1<<TWSTO)|(1<<TWINT);   /* recover from TWI_BUS_ERROR, this will release the SDA and SCL pins thus enabling other devices to use the bus */
		break;
		
		default:
			i2c_state = TWSR;                                 /* store TWI State as errormessage, operation also clears the Success bit */      
			TWCR = (1<<TWEN)|                                 /* enable TWI-interface and release TWI pins */
				   (1<<TWIE)|(1<<TWINT)|                      /* keep interrupt enabled and clear the flag */
				   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           /* acknowledge on any new requests */
                   (0<<TWWC);      
			i2c_busy = 0; /* unknown status, so we wait for a new address match that might be something we can handle */
		break;
	} /* end switch(TWSR) */

}
