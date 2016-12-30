#ifndef ASENSOR_H
#define ASENSOR_H
#include "mbed.h"

// resistor1 and resistor2 are voltage divide resistors.
// resistor1 is at output of sensor.
// resistor2 is connected to ground.


class ASensor{
public:
    ASensor(PinName pin, double resistor1 = 0.0, double resistor2 = 1.0);
    double voltage_divider();
    float voltage_fraction_of_ref();
    float voltage();   
    
    void set_voltage_divider(double resistor1, double resistor2);
    
private:
    AnalogIn analog_sensor_;
    double voltage_divider_;   
    static const double reference_voltage_ = 3.3; 
    
};

#endif
