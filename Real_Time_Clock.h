#include <stdio.h>
#include <stdlib.h>

//Read from i2c device
void i2c_read_register(int32_t port, uint8_t addr, uint8_t reg, uint8_t size, uint8_t *data);

//Write to i2c device
void i2c_write_register(int32_t port, uint8_t addr, uint8_t reg, uint8_t size, uint8_t *data);

//Get

int get_time_sec();

int get_time_min();

int get_time_hour();

int get_time_day();

int get_time_month();

int get_time_year();

//Set (Use Hexadeciamal)

void set_time_sec(uint8_t s_time);

void set_time_min(uint8_t m_time);

void set_time_hour(uint8_t h_time);

void set_time_day(uint8_t d_time);

void set_time_month(uint8_t M_time);

void set_time_year(uint8_t y_time);

