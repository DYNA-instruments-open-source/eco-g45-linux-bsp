#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H


#define I2C_READ_BIT  0   /* bit position for R/W bit in "address byte" */
#define I2C_ADR_BITS  1   /* bit position for LSB of the slave address bits in the init byte */
#define I2C_GEN_BIT   0   /* bit position for LSB of the general call bit in the init byte */

/* I2C slave transmitter staus codes */
#define I2C_STX_ADR_ACK            0xA8  /* own SLA+R has been received, ACK has been returned */
#define I2C_STX_ADR_ACK_M_ARB_LOST 0xB0  /* arbitration lost in SLA+R/W as master, own SLA+R has been received, ACK has been returned */
#define I2C_STX_DATA_ACK           0xB8  /* data byte in TWDR has been transmitted, ACK has been received */
#define I2C_STX_DATA_NACK          0xC0  /* data byte in TWDR has been transmitted, NOT ACK has been received */
#define I2C_STX_DATA_ACK_LAST_BYTE 0xC8  /* last data byte in TWDR has been transmitted (TWEA = 0), ACK has been received */

// TWI Slave Receiver staus codes
#define I2C_SRX_ADR_ACK            0x60  /* own SLA+W has been received ACK has been returned */
#define I2C_SRX_ADR_ACK_M_ARB_LOST 0x68  /* arbitration lost in SLA+R/W as master; own SLA+W has been received, ACK has been returned */
#define I2C_SRX_GEN_ACK            0x70  /* general call address has been received ACK has been returned */
#define I2C_SRX_GEN_ACK_M_ARB_LOST 0x78  /* arbitration lost in SLA+R/W as master, general call address has been received; ACK has been returned */
#define I2C_SRX_ADR_DATA_ACK       0x80  /* previously addressed with own SLA+W, data has been received, ACK has been returned */
#define I2C_SRX_ADR_DATA_NACK      0x88  /* previously addressed with own SLA+W, data has been received, NOT ACK has been returned */
#define I2C_SRX_GEN_DATA_ACK       0x90  /* previously addressed with general call, data has been received, ACK has been returned */
#define I2C_SRX_GEN_DATA_NACK      0x98  /* previously addressed with general call, data has been received, NOT ACK has been returned */
#define I2C_SRX_STOP_RESTART       0xA0  /* a STOP condition or repeated START condition has been received while still addressed as slave */

/* I2C Miscellaneous status codes */
#define I2C_NO_STATE               0xF8  /* no relevant state information available, TWINT = 0 */
#define I2C_BUS_ERROR              0x00  /* bus error due to an illegal START or STOP condition */

#define I2C_MAP_FIRST_WRITE_ADDRESS 0x00
#define I2C_MAP_LAST_WRITE_ADDRESS  0x05

union i2c_status_reg_t                       /* status byte holding flags */
{
    unsigned char all;
    struct
    {
        unsigned char last_trans_ok:1;      
        unsigned char rx_data_in_buf:1;
        unsigned char gen_address_call:1;                        /* TRUE = general call, FALSE = I2C address */
        unsigned char unused_bits:5;
    };
};

extern union i2c_status_reg_t i2c_status_reg;

struct memory_map_t                       /* status byte holding flags */
{
	unsigned char array[I2C_MAP_LAST_WRITE_ADDRESS+1];
};

extern struct memory_map_t memory_map;

void i2c_slave_init(void);
void i2c_slave_start_transceiver(void);
unsigned char i2c_slave_transceiver_busy(void);
unsigned char i2c_slave_get_state_info(void);

#endif /* I2C_SLAVE_H */