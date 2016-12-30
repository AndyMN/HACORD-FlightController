#include "PressureSensor.h"


PressureSensor::PressureSensor(PinName pin, int resistor1, int resistor2):ASensor(pin, resistor1, resistor2), num_values_stored_(0){
    stored_values_.resize(10); // Filthy hardcoded values, lazy
    means_.resize(2);
    deviations_.resize(2);
    slope_condition_met_ = 0;
    slope_condition_satisfied_ = false;
    threshold_condition_met_ = 0;
    threshold_condition_satisfied_ = false;
}

float PressureSensor::pressure(float reference_voltage){
    float V_out = voltage() * Vcc_ref_ / reference_voltage;
    float press = (V_out - adcvalue1_) / (adcvalue2_ - adcvalue1_) * (press2_ - press1_) + press1_;
    return press;
}
    
void PressureSensor::mean_values(){
    int len_value_storage_vector = stored_values_.size();
    float mean = 0;
    for (int i = 0; i < len_value_storage_vector; i++){
        mean += stored_values_[i];
    }
    mean = mean / len_value_storage_vector;
    
    // Replace the old with the new
    means_[0] = means_[1];
    means_[1] = mean;
}

void PressureSensor::deviation_calculation(){
    int len_value_storage_vector = stored_values_.size();
    float deviation = 0;
    for (int i = 0; i < len_value_storage_vector; i++){
        deviation += pow(means_[1] - stored_values_[i], 2);
    }
    deviation = sqrt(deviation/(len_value_storage_vector * (len_value_storage_vector - 1)));
    
    // Replace the old with the new
    deviations_[0] = deviations_[1];
    deviations_[1] = deviation;
}    

std::vector<float> PressureSensor::vec_means(){
    return means_;
}

std::vector<float> PressureSensor::vec_deviations(){
    return deviations_;
}

void PressureSensor::measure(float source_voltage){
    if (num_values_stored_ == stored_values_.size()) {
        mean_values();
        deviation_calculation();
        num_values_stored_ = 0;
        check_condition_met();
        check_over_threshold();
   }
   stored_values_[num_values_stored_] = pressure(source_voltage);
   num_values_stored_++;
   
}

void PressureSensor::check_condition_met(){
    float max_value_next = means_[1] + deviations_[1];
    float min_value_next = means_[1] - deviations_[1];
    float max_value_prev = means_[0] + deviations_[0];
    float min_value_prev = means_[0] - deviations_[0];
    
    // Next pressure HIGHER than previous pressure (Descending slope of p(t) curve)
    if (min_value_next > max_value_prev) {
        slope_condition_met_++;
        if (slope_condition_met_ >= conseq_needed_){ // This function will return True if the sensor is sure that we are descending
            slope_condition_satisfied_ = true;
        }else {
            //slope_condition_satisfied_ = false;
        }
    }else {
        slope_condition_met_ = 0; // Reset this counter because we NEED a number of CONSEQUTIVE condition mets to remove false positives
        //slope_condition_satisfied_ = false;
    }
}

bool PressureSensor::slope_condition_satisfied(){
    return slope_condition_satisfied_;
}
void PressureSensor::check_over_threshold(){
    if (means_[1] > pressure_threshold_){
        threshold_condition_met_++;
        if (threshold_condition_met_ >= conseq_needed_){
            threshold_condition_satisfied_ = true;
            //return true;
        }else{
            threshold_condition_satisfied_ = false;
            //return false;
        }
    }else{
        threshold_condition_met_ = 0;
        threshold_condition_satisfied_ = false;
        //return false;
    }
}

bool PressureSensor::threshold_condition_satisfied(){
    return threshold_condition_satisfied_;
}

int PressureSensor::slope_condition_met(){
    return slope_condition_met_;
}

int PressureSensor::threshold_condition_met(){
    return threshold_condition_met_;
}
