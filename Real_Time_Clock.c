#include "gd32vf103.h"
#include "Real_Time_Clock.h"
#include <stdio.h>
#include <stdlib.h>

#define DS1307_ADDRESS 0x68
#define SECONDS 0x00
#define MINUTES 0x01
#define HOURS 0x02
#define DAY 0x04
#define MONTH 0x05
#define YEAR 0x06

#define FAST_I2C 400000
#define STANDARD_I2C 100000

uint8_t read_buffer[10] = {0};

int DecToHex(uint8_t dec){
    int rem = 0;
	int quo = 0;
	int hex = 0;
	int count = 1;
	rem = dec % 16;
	quo = dec / 16;
	hex = rem;
	do{
		count = count * 10;
		rem = quo % 16;
		quo = quo / 16;
		hex = hex + (rem * count);
	}while(quo != 0);
    return hex;
}

/* You can copy this function to be able to read from most i2c devices */
void i2c_read_register(int32_t port, uint8_t addr, uint8_t reg, uint8_t size, uint8_t *data){

    /* send a start condition to I2C bus */
    i2c_start_on_bus(port);
    while(!i2c_flag_get(port, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus with write flag */
    i2c_master_addressing(port, addr << 1, I2C_TRANSMITTER);
    /* Wait for sending address to finish */
    while(!i2c_flag_get(port, I2C_FLAG_ADDSEND));
    i2c_flag_clear(port, I2C_FLAG_ADDSEND);

    /* Send which register to read */
    i2c_data_transmit(port, reg);
    /* wait until the data has been sent */
    while(!i2c_flag_get(port, I2C_FLAG_TBE));

    /* Send new start condition */
    i2c_start_on_bus(port);
    while( ! i2c_flag_get(port, I2C_FLAG_SBSEND) );

    /* Now send address with read flag */
    i2c_master_addressing(port, addr << 1, I2C_RECEIVER);
    while( ! i2c_flag_get(port, I2C_FLAG_ADDSEND) );
    i2c_flag_clear(port, I2C_FLAG_ADDSEND);

    /* Enable acknowledge for receiving multiple bytes */
    i2c_ack_config(port, I2C_ACK_ENABLE);

    /* Receive bytes, read into buffer. */
    for(int i = 0; i < size; i++) {
        if(i == size - 1) {
            /* If last byte, do not send ack */
            i2c_ack_config(port, I2C_ACK_DISABLE);
        }
        while(!i2c_flag_get(port, I2C_FLAG_RBNE));
        *data++ = i2c_data_receive(port);
    }

    i2c_stop_on_bus(port);
    while( I2C_CTL0(port) & I2C_CTL0_STOP );
}

/* You can copy this function to be able to write most i2c devices */
void i2c_write_register(int32_t port, uint8_t addr, uint8_t reg, uint8_t size, uint8_t *data)
{    
    /* send a NACK for the next data byte which will be received into the shift register */
    while(i2c_flag_get(port, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(port);
    while(!i2c_flag_get(port, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(port, addr << 1, I2C_TRANSMITTER);
    while(!i2c_flag_get(port, I2C_FLAG_ADDSEND));
    i2c_flag_clear(port, I2C_FLAG_ADDSEND);

    
    /* Send which register to write */
    i2c_data_transmit(port, reg);
    /* wait until the TBE bit is set */
    while(!i2c_flag_get(port, I2C_FLAG_TBE));

    /* Send data */
    for(int i = 0; i < size; i++){
        i2c_data_transmit(port, *data++);
        /* wait until the TBE bit is set */
        while(!i2c_flag_get(port, I2C_FLAG_TBE));
    }

    /* Send stop condition */
    i2c_stop_on_bus(port);
    while( I2C_CTL0(port) & I2C_CTL0_STOP );
}

int get_time_sec(){
    i2c_read_register(I2C0, DS1307_ADDRESS, SECONDS, 1, read_buffer);
    uint8_t stime = DecToHex(read_buffer[0]);
    return stime;
}

int get_time_min(){
    i2c_read_register(I2C0, DS1307_ADDRESS, MINUTES, 1, read_buffer);
    uint8_t mtime = DecToHex(read_buffer[0]);
    return mtime;
}

int get_time_hour(){
    i2c_read_register(I2C0, DS1307_ADDRESS, HOURS, 1, read_buffer);
    uint8_t htime = DecToHex(read_buffer[0]);
    return htime;
}

int get_time_day(){
    i2c_read_register(I2C0, DS1307_ADDRESS, DAY, 1, read_buffer);
    uint8_t dtime = DecToHex(read_buffer[0]);
    return dtime;
}

int get_time_month(){
    i2c_read_register(I2C0, DS1307_ADDRESS, MONTH, 1, read_buffer);
    uint8_t MTime = DecToHex(read_buffer[0]);
    return MTime;
}

int get_time_year(){
    i2c_read_register(I2C0, DS1307_ADDRESS, YEAR, 1, read_buffer);
    uint8_t ytime = DecToHex(read_buffer[0]);
    return ytime + 2000;
}

//Set time values, Use hexadecimal, Ex: 0x59 to set minutes to 59 minutes

void set_time_sec(uint8_t s_time){
    read_buffer[0] = s_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, SECONDS, 1, read_buffer);
}

void set_time_min(uint8_t m_time){
    read_buffer[0] = m_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, MINUTES, 1, read_buffer);
}

void set_time_hour(uint8_t h_time){
    read_buffer[0] = h_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, HOURS, 1, read_buffer);
}

void set_time_day(uint8_t d_time){
    read_buffer[0] = d_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, DAY, 1, read_buffer);
}

void set_time_month(uint8_t M_time){
    read_buffer[0] = M_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, MONTH, 1, read_buffer);
}

void set_time_year(uint8_t y_time){
    read_buffer[0] = y_time;
    i2c_write_register(I2C0, DS1307_ADDRESS, YEAR, 1, read_buffer);
}
