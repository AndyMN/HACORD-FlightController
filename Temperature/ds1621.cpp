#include "ds1621.hpp"
#include "mbed.h"


DS1621::DS1621(I2C* i2cp, int addrin ){

 i2c=i2cp;
 addr=addrin; 
 char configset[2]={0xAC,0x1};// set single shot mode
 i2c->write(addr,configset, sizeof(configset)); 
 wait(0.1);
 unset_thermostat();
 
 }
 
 
 void DS1621::init_meassure(){
 char start      = 0xEE;
  i2c->write(addr,(char *)&start, sizeof(start));
 }
 
 void DS1621::unset_thermostat()
{
    //A1: acces th  
    char th[3];
        th[0] = 0xA1;
        th[1] = 0x7D;
        th[2] = 0x00;
    //A2: acces tl
    char tl[3];
        tl[0] = 0xA2;
        tl[1] = 0xC9;
        tl[2] = 0x00;
    i2c->write(addr,(char *)&th,sizeof(th));   
    i2c->write(addr,(char *)&tl,sizeof(tl));
}


void DS1621::init_read(){
 char read      = 0xAA;
  i2c->write(addr,(char *)&read,sizeof(read));
 }
 
 
 float  DS1621::read_T(){
 float temperature;
  char temp8[2];
  i2c->read(addr+1, temp8 ,sizeof(temp8), false);
   uint16_t temp16;
       
        // Format temperature
        temp16 = temp8[0];
        temp16 = temp16 << 1;
        temp16 = temp16 + (temp8[1] >> 7);
        if ((temp16 & 0x100) == 0) { 
            temperature = ((float) temp16 / 2);
        } else {  
            temp16 = ~temp16 & 0xFF;
            temperature = ((float) temp16 / 2);
            temperature = -1.0 * temperature;
        }
    return temperature;    
}



