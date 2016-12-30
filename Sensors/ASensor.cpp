#include "ASensor.h"


ASensor::ASensor(PinName pin, double resistor1, double resistor2):analog_sensor_(pin){
    
    set_voltage_divider(resistor1, resistor2);
    
    }
    
void ASensor::set_voltage_divider(double resistor1, double resistor2){
    
    voltage_divider_ = resistor2 / (resistor1 + resistor2);
}

double ASensor::voltage_divider(){
    return voltage_divider_;
}

float ASensor::voltage_fraction_of_ref(){
    return analog_sensor_.read();
}

float ASensor::voltage(){
    return analog_sensor_.read() * reference_voltage_ * (1 / voltage_divider_);
}
