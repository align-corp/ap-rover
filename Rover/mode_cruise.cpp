#include "mode.h"
#include "Rover.h"
#include <GCS_MAVLink/GCS.h>

bool ModeCruise::_enter()
{
    gcs().send_text(MAV_SEVERITY_CRITICAL, "Entering Cruise Mode");
    _speed_input = 0;
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
    float steering, throttle, speed;
    get_pilot_input(steering, throttle);
    _speed_input += throttle * g.cruise_p*0.01f;

    // do not reverse and stop immediately if minimum throttle
    if (_speed_input < 0.0f || throttle < -99.0f) {
        _speed_input = 0.0f;
    } else if (_speed_input > 100.0f) {
        _speed_input = 100.0f;
    }
    speed = _speed_input;

    // for skid steering vehicles, if pilot commands would lead to saturation
    // we proportionally reduce steering and _speed_input
    if (g2.motors.have_skid_steering()) {
        const float steer_normalised = constrain_float(steering / 4500.0f, -1.0f, 1.0f);
        const float _speed_normalised = constrain_float(_speed_input / 100.0f, -1.0f, 1.0f);
        const float saturation_value = fabsf(steer_normalised) + fabsf(_speed_normalised);
        if (saturation_value > 1.0f) {
            steering /= saturation_value;
            speed = _speed_input/saturation_value;
        }
    }
    // check for special case of input and output throttle being in opposite directions
    float throttle_limited = g2.motors.get_slew_limited_throttle(speed, rover.G_Dt);
    if ((is_negative(speed) != is_negative(throttle_limited)) &&
        ((g.pilot_steer_type == PILOT_STEER_TYPE_DEFAULT) ||
         (g.pilot_steer_type == PILOT_STEER_TYPE_DIR_REVERSED_WHEN_REVERSING))) {
        steering *= -1;
    }
    speed = throttle_limited;

    gcs().send_text(MAV_SEVERITY_CRITICAL, "steering: %f, throttle %f", steering, speed);

    // copy RC scaled inputs to outputs
    g2.motors.set_throttle(speed);
    g2.motors.set_steering(steering, false);
}
