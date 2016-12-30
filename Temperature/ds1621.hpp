#ifndef _DS1621_HPP_
#define _DS1621_HPP_
#include "mbed.h"

//class I2C;

class DS1621 {
//private :

I2C* i2c; // 
int addr;

public :

// assign i2c channel , set address and set in one shot mode 
DS1621(I2C* i2cp, int addrin );
//DS1621( int addrin );
void unset_thermostat(); 
void init_meassure();
void init_read();
float  read_T();

// destructor ??
};
#endif

