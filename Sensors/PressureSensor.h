#ifndef PRESSURESENSOR_H
#define PRESSURESENSOR_H

#include "ASensor.h"
#include <vector>


class PressureSensor: public ASensor{
public:
    PressureSensor(PinName pin, int resistor1= 0, int resistor2 = 1);
    float pressure(float source_voltage);
    
    void measure(float source_voltage);
    bool slope_condition_satisfied();
    bool threshold_condition_satisfied();
    int slope_condition_met();
    int threshold_condition_met();
    
    std::vector<float> vec_means();
    std::vector<float> vec_deviations();
    
private:
    void mean_values();
    void deviation_calculation();
    void check_condition_met();    
    void check_over_threshold();
    std::vector<float> stored_values_; 
    int num_values_stored_;
    std::vector<float> means_; // store means corresponding to a full set of stored_values vector
    std::vector<float> deviations_; // store deviations to the mean corresponding to a full set of stored_values vector

    // INTERPOLATION VALUES
    static const float adcvalue1_ = 0.5; 
    static const float adcvalue2_ = 4.5;
    static const float press1_ = 0;     
    static const float press2_ = 103.421;
    static const float Vcc_ref_ = 5;  
    
    // CONDITION MET VALUES
    static const int conseq_needed_ = 5;
    int slope_condition_met_;
    bool slope_condition_satisfied_;
    
    int threshold_condition_met_;
    bool threshold_condition_satisfied_;
    static const float pressure_threshold_ = 40; // Threshold in kPa
};

#endif
