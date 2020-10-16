#include <atomic>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>

#include "mavsdk.h"
#include "integration_test_helper.h"
#include "plugins/action/action.h"
#include "plugins/gimbal/gimbal.h"
#include "plugins/offboard/offboard.h"
#include "plugins/telemetry/telemetry.h"

using namespace mavsdk;

void send_new_gimbal_command(std::shared_ptr<Gimbal> gimbal, int i);
void gimbal_pattern(std::shared_ptr<Gimbal> gimbal);
void send_gimbal_roi_location(
    std::shared_ptr<Gimbal> gimbal, double latitude_deg, double longitude_deg, float altitude_m);
void receive_gimbal_result(Gimbal::Result result);
void receive_gimbal_attitude_euler_angles(Telemetry::EulerAngle euler_angle);

// Note, this test does not work in SITL because the gimbal does not move
// unless it is armed.
TEST(SitlTestGimbal, GimbalMove)
{
    Mavsdk mavsdk;

    ConnectionResult ret = mavsdk.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::Success);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto system = mavsdk.systems().at(0);

    // FIXME: This is what it should be, for now though with the typhoon_h480
    //        SITL simulation, the gimbal is hooked up to the autopilot.
    // ASSERT_TRUE(system.has_gimbal());
    ASSERT_TRUE(system->has_autopilot());

    auto telemetry = std::make_shared<Telemetry>(system);
    auto gimbal = std::make_shared<Gimbal>(system);

    telemetry->set_rate_camera_attitude(10.0);

    telemetry->subscribe_camera_attitude_euler(&receive_gimbal_attitude_euler_angles);

    for (int i = 0; i < 500; i += 1) {
        send_new_gimbal_command(gimbal, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

TEST(SitlTestGimbal, GimbalTakeoffAndMove)
{
    Mavsdk mavsdk;

    ConnectionResult ret = mavsdk.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::Success);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto system = mavsdk.systems().at(0);
    // ASSERT_TRUE(system.has_gimbal());
    ASSERT_TRUE(system->has_autopilot());

    auto telemetry = std::make_shared<Telemetry>(system);
    auto gimbal = std::make_shared<Gimbal>(system);
    auto action = std::make_shared<Action>(system);
    auto offboard = std::make_shared<Offboard>(system);

    while (!telemetry->health_all_ok()) {
        LogInfo() << "waiting for system to be ready";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    Action::Result action_result = action->arm();
    EXPECT_EQ(action_result, Action::Result::Success);

    action_result = action->takeoff();
    EXPECT_EQ(action_result, Action::Result::Success);

    telemetry->set_rate_camera_attitude(10.0);

    telemetry->subscribe_camera_attitude_euler(&receive_gimbal_attitude_euler_angles);

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Slowly fly circle.
    Offboard::VelocityBodyYawspeed circle;
    circle.forward_m_s = 0.5f;
    circle.right_m_s = 0.0f;
    circle.down_m_s = 0.0f;
    circle.yawspeed_deg_s = 10.0f;

    EXPECT_EQ(offboard->set_velocity_body(circle), Offboard::Result::Success);
    EXPECT_EQ(offboard->start(), Offboard::Result::Success);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // First use gimbal in yaw follow mode.
    EXPECT_EQ(gimbal->set_mode(Gimbal::GimbalMode::YawFollow), Gimbal::Result::Success);

    gimbal_pattern(gimbal);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Now set to lock mode
    EXPECT_EQ(gimbal->set_mode(Gimbal::GimbalMode::YawLock), Gimbal::Result::Success);

    gimbal_pattern(gimbal);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    action->land();
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void gimbal_pattern(std::shared_ptr<Gimbal> gimbal)
{
    // Look forward
    gimbal->set_pitch_and_yaw_async(0.0f, 0.0f, &receive_gimbal_result);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Look right
    for (int i = 0; i < 90; ++i) {
        gimbal->set_pitch_and_yaw_async(0.0f, static_cast<float>(i), &receive_gimbal_result);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Look left
    for (int i = 90; i >= -90; --i) {
        gimbal->set_pitch_and_yaw_async(0.0f, static_cast<float>(i), &receive_gimbal_result);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Look forward
    for (int i = -90; i <= 0; ++i) {
        gimbal->set_pitch_and_yaw_async(0.0f, static_cast<float>(i), &receive_gimbal_result);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Tilt down
    for (int i = 0; i > -90; --i) {
        gimbal->set_pitch_and_yaw_async(static_cast<float>(i), 0.0f, &receive_gimbal_result);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // And back up
    for (int i = -90; i <= 0; ++i) {
        gimbal->set_pitch_and_yaw_async(static_cast<float>(i), 0.0f, &receive_gimbal_result);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

TEST(SitlTestGimbal, GimbalROIOffboard)
{
    Mavsdk mavsdk;

    ConnectionResult ret = mavsdk.add_udp_connection();
    ASSERT_EQ(ret, ConnectionResult::Success);

    // Wait for system to connect via heartbeat.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    auto system = mavsdk.systems().at(0);
    // ASSERT_TRUE(system.has_gimbal());
    ASSERT_TRUE(system->has_autopilot());

    auto telemetry = std::make_shared<Telemetry>(system);
    auto gimbal = std::make_shared<Gimbal>(system);
    auto action = std::make_shared<Action>(system);
    auto offboard = std::make_shared<Offboard>(system);

    while (!telemetry->health_all_ok()) {
        LogInfo() << "waiting for system to be ready";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    const Telemetry::Position& position = telemetry->position();

    // set the ROI location: a bit to the north of the vehicle's location
    const double latitude_offset_deg = 3. / 111111.; // this is about 3 m
    send_gimbal_roi_location(
        gimbal,
        position.latitude_deg + latitude_offset_deg,
        position.longitude_deg,
        position.absolute_altitude_m + 1.f);

    Action::Result action_result = action->arm();
    EXPECT_EQ(action_result, Action::Result::Success);

    action_result = action->takeoff();
    EXPECT_EQ(action_result, Action::Result::Success);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    telemetry->set_rate_camera_attitude(10.0);

    telemetry->subscribe_camera_attitude_euler(&receive_gimbal_attitude_euler_angles);

    // Send it once before starting offboard, otherwise it will be rejected.
    Offboard::VelocityNedYaw still;
    still.north_m_s = 0.0f;
    still.east_m_s = 0.0f;
    still.down_m_s = 0.0f;
    still.yaw_deg = 0.0f;
    offboard->set_velocity_ned(still);
    Offboard::Result offboard_result = offboard->start();
    EXPECT_EQ(offboard_result, Offboard::Result::Success);

    auto fly_straight = [&offboard](int step_count, float max_speed) {
        for (int i = 0; i < step_count; ++i) {
            int k = (i <= step_count / 2) ? i : step_count - i;
            float vy = static_cast<float>(k) / (step_count / 2) * max_speed;

            Offboard::VelocityNedYaw move_east;
            move_east.north_m_s = 0.0f;
            move_east.east_m_s = vy;
            move_east.down_m_s = 0.0f;
            move_east.yaw_deg = 90.0f;
            offboard->set_velocity_ned(move_east);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    // fly east for a bit
    fly_straight(300, 2.f);

    // fly in north-south direction (back & forth a few times)
    const float step_size = 0.01f;
    const float one_cycle = 2.0f * M_PI_F;
    const unsigned steps = static_cast<unsigned>(2.5f * one_cycle / step_size);

    for (unsigned i = 0; i < steps; ++i) {
        float vx = 5.0f * sinf(i * step_size);

        Offboard::VelocityNedYaw move_north;
        move_north.north_m_s = vx;
        move_north.east_m_s = 0.0f;
        move_north.down_m_s = 0.0f;
        move_north.yaw_deg = 90.0f;
        offboard->set_velocity_ned(move_north);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // fly west for a bit
    fly_straight(600, -4.f);

    action->land();
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void send_new_gimbal_command(std::shared_ptr<Gimbal> gimbal, int i)
{
    float pitch_deg = 30.0f * cosf(i / 360.0f * 2 * M_PI_F);
    float yaw_deg = 45.0f * sinf(i / 360.0f * 2 * M_PI_F);

    gimbal->set_pitch_and_yaw_async(pitch_deg, yaw_deg, &receive_gimbal_result);
}

void send_gimbal_mode_command(std::shared_ptr<Gimbal> gimbal, const Gimbal::GimbalMode gimbal_mode)
{
    gimbal->set_mode_async(gimbal_mode, &receive_gimbal_result);
}

void send_gimbal_roi_location(
    std::shared_ptr<Gimbal> gimbal, double latitude_deg, double longitude_deg, float altitude_m)
{
    gimbal->set_roi_location_async(latitude_deg, longitude_deg, altitude_m, &receive_gimbal_result);
}

void receive_gimbal_result(Gimbal::Result result)
{
    EXPECT_EQ(result, Gimbal::Result::Success);
}

void receive_gimbal_attitude_euler_angles(Telemetry::EulerAngle euler_angle)
{
    UNUSED(euler_angle);
    LogInfo() << "Received gimbal attitude: " << euler_angle;
}
