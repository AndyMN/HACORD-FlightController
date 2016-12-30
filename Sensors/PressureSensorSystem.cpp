#include "PressureSensorSystem.h"



PressureSensorSystem::PressureSensorSystem(std::vector<PressureSensor*>& pressure_sensors, ASensor* reference_voltage):pressure_sensors_(pressure_sensors), reference_voltage_(reference_voltage), num_sensors_(pressure_sensors.size()){
}


int PressureSensorSystem::num_sensors(){
    return num_sensors_;
}

void PressureSensorSystem::measure(){
    std::vector<PressureSensor*>::iterator end_of_vec = pressure_sensors_.end();
    for (std::vector<PressureSensor*>::iterator it = pressure_sensors_.begin(); it != end_of_vec; ++it) {
        (*it)->measure(reference_voltage_->voltage());
    }
}

bool PressureSensorSystem::is_descending_slope(){
    // MAJORITY VOTE SYSTEM
    if (num_sensors_ == 1){
        if (pressure_sensors_[0]->slope_condition_satisfied()){
            return true;
        }else {
            return false;
        }
    }else if (num_sensors_ == 2){
        if (pressure_sensors_[0]->slope_condition_satisfied() || pressure_sensors_[1]->slope_condition_satisfied()){
            return true;
        } else {
            return false;
        }
    }else if (num_sensors_ == 3) {
        if (pressure_sensors_[0]->slope_condition_satisfied()) {
            if (pressure_sensors_[1]->slope_condition_satisfied()){
                return true;
            } else if (pressure_sensors_[2]->slope_condition_satisfied()){
                return true;
            }else {
                return false;
            }
        }else if (pressure_sensors_[1]->slope_condition_satisfied()){
            if (pressure_sensors_[2]->slope_condition_satisfied()){
                return true;
            }else{
                return false;
            }
        } else {
            return false;
        }
    }else {
        return false;
    }
}

bool PressureSensorSystem::is_over_threshold(){
    //MAJORITY VOTE SYSTEM  
    if (num_sensors_ == 1){
        if (pressure_sensors_[0]->threshold_condition_satisfied()){
            return true;
        }else {
            return false;
        }
    }else if (num_sensors_ == 2){
        if (pressure_sensors_[0]->threshold_condition_satisfied() || pressure_sensors_[1]->threshold_condition_satisfied()){
            return true;
        } else {
            return false;
        }
    }else if (num_sensors_ == 3) {
        if (pressure_sensors_[0]->threshold_condition_satisfied()) {
            if (pressure_sensors_[1]->threshold_condition_satisfied()){
                return true;
            } else if (pressure_sensors_[2]->threshold_condition_satisfied()){
                return true;
            }else {
                return false;
            }
        }else if (pressure_sensors_[1]->threshold_condition_satisfied()){
            if (pressure_sensors_[2]->threshold_condition_satisfied()){
                return true;
            }else{
                return false;
            }
        } else {
            return false;
        }
    }else {
        return false;
    }    
}


bool PressureSensorSystem::HV_state(){
    // HV HEEFT GEINVERTEERDE INPUT ! TRUE = AFZETTEN, FALSE = OPZETTEN
    if (is_descending_slope() && is_over_threshold()){
        return true;
    }else{
        return false;
    }
}
