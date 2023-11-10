#include "mode.h"
#include "Rover.h"
#include <GCS_MAVLink/GCS.h>

bool ModeCruise::_enter()
{
    gcs().send_text(MAV_SEVERITY_CRITICAL, "Entering Cruise Mode");
    return true;
}

void ModeCruise::_exit()
{
    // clear lateral when exiting manual mode
    g2.motors.set_lateral(0);
    gcs().send_text(MAV_SEVERITY_CRITICAL, "Exiting Cruise Mode");
}

void ModeCruise::update()
{
    float steering_out, throttle_out, desired_lateral;
    // do basic conversion
    get_pilot_input(steering_out, throttle_out);
    gcs().send_text(MAV_SEVERITY_CRITICAL, "steering_in: %f, throttle_in %f", steering_out, throttle_out);

    // for skid steering vehicles, if pilot commands would lead to saturation
    // we proportionally reduce steering and throttle
    if (g2.motors.have_skid_steering()) {
        const float steer_normalised = constrain_float(steering_out / 4500.0f, -1.0f, 1.0f);
        const float throttle_normalised = constrain_float(throttle_out / 100.0f, -1.0f, 1.0f);
        const float saturation_value = fabsf(steer_normalised) + fabsf(throttle_normalised);
        if (saturation_value > 1.0f) {
            steering_out /= saturation_value;
            throttle_out /= saturation_value;
        }
    }
    // check for special case of input and output throttle being in opposite directions
    float throttle_out_limited = g2.motors.get_slew_limited_throttle(throttle_out, rover.G_Dt);
    if ((is_negative(throttle_out) != is_negative(throttle_out_limited)) &&
        ((g.pilot_steer_type == PILOT_STEER_TYPE_DEFAULT) ||
         (g.pilot_steer_type == PILOT_STEER_TYPE_DIR_REVERSED_WHEN_REVERSING))) {
        steering_out *= -1;
    }
    throttle_out = throttle_out_limited;

    get_pilot_desired_lateral(desired_lateral);
    gcs().send_text(MAV_SEVERITY_CRITICAL, "steering: %f, throttle %f, lateral: %f", steering_out, throttle_out, desired_lateral);

    // copy RC scaled inputs to outputs
    g2.motors.set_throttle(throttle_out);
    g2.motors.set_steering(steering_out, false);
    //g2.motors.set_lateral(desired_lateral);
}
