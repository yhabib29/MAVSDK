#include "telemetry.h"
#include "telemetry_impl.h"

namespace dronelink {

Telemetry::Telemetry(TelemetryImpl *impl) :
    _impl(impl)
{
}

Telemetry::~Telemetry()
{
}

Telemetry::Position Telemetry::position() const
{
    return _impl->get_position();
}

Telemetry::Position Telemetry::home_position() const
{
    return _impl->get_home_position();
}

bool Telemetry::in_air() const
{
    return _impl->in_air();
}

Telemetry::Quaternion Telemetry::attitude_quaternion() const
{
    return _impl->get_attitude_quaternion();
}

Telemetry::EulerAngle Telemetry::attitude_euler_angle() const
{
    return _impl->get_attitude_euler_angle();
}

Telemetry::GroundSpeedNED Telemetry::ground_speed_ned() const
{
    return _impl->get_ground_speed_ned();
}

Telemetry::GPSInfo Telemetry::gps_info() const
{
    return _impl->get_gps_info();
}

Telemetry::Battery Telemetry::battery() const
{
    return _impl->get_battery();
}

void Telemetry::position_async(double rate_hz, position_callback_t callback)
{
    return _impl->position_async(rate_hz, callback);
}

void Telemetry::home_position_async(double rate_hz, position_callback_t callback)
{
    return _impl->home_position_async(rate_hz, callback);
}

void Telemetry::in_air_async(double rate_hz, in_air_callback_t callback)
{
    return _impl->in_air_async(rate_hz,  callback);
}

void Telemetry::attitude_quaternion_async(double rate_hz,
                                          attitude_quaternion_callback_t callback)
{
    return _impl->attitude_quaternion_async(rate_hz, callback);
}

void Telemetry::attitude_euler_angle_async(double rate_hz,
                                           attitude_euler_angle_callback_t callback)
{
    return _impl->attitude_euler_angle_async(rate_hz, callback);
}

void Telemetry::ground_speed_ned_async(double rate_hz,
                                       ground_speed_ned_callback_t callback)
{
    return _impl->ground_speed_ned_async(rate_hz, callback);
}

void Telemetry::gps_info_async(double rate_hz, gps_info_callback_t callback)
{
    return _impl->gps_info_async(rate_hz, callback);
}

void Telemetry::battery_async(double rate_hz, battery_callback_t callback)
{
    return _impl->battery_async(rate_hz, callback);
}


} // namespace dronelink
