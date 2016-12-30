#ifndef PRESSURESENSORSYSTEM_H
#define PRESSURESENSORSYSTEM_H
#include <vector>
#include "PressureSensor.h"


class PressureSensorSystem{
public:
    PressureSensorSystem(std::vector<PressureSensor*>& pressure_sensors, ASensor* reference_voltage);
  
    int num_sensors();    
    bool is_descending_slope();
    bool is_over_threshold();
    bool HV_state();
    void measure();
    
private:
    std::vector<PressureSensor*> pressure_sensors_;
    ASensor* reference_voltage_;
    
    int num_sensors_;       
};

#endif
