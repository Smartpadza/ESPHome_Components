#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace hlw8012 {

enum HLW8012SensorModel {
  HLW8012_SENSOR_MODEL_HLW8012 = 0,
  HLW8012_SENSOR_MODEL_CSE7759 = 1,
  HLW8012_SENSOR_MODEL_BL0937 = 2,
};

enum HLW8012InitialMode {
  HLW8012_INITIAL_MODE_CURRENT = 0,
  HLW8012_INITIAL_MODE_VOLTAGE = 1,
};

class HLW8012Component : public PollingComponent {
 public:
  void set_cf_pin(InternalGPIOPin *pin)   { cf_pin_ = pin; }
  void set_cf1_pin(InternalGPIOPin *pin)  { cf1_pin_ = pin; }
  void set_sel_pin(InternalGPIOPin *pin)  { sel_pin_ = pin; }

  // Primary setter methods (what ESPHome expects)
  void set_voltage_sensor(sensor::Sensor *s) { voltage_sensor_ = s; }
  void set_current_sensor(sensor::Sensor *s) { current_sensor_ = s; }
  void set_power_sensor(sensor::Sensor *s)   { power_sensor_ = s; }
  void set_energy_sensor(sensor::Sensor *s)  { energy_sensor_ = s; }

  // Legacy method names for compatibility
  void set_voltage(sensor::Sensor *s)     { voltage_sensor_ = s; }
  void set_current(sensor::Sensor *s)     { current_sensor_ = s; }
  void set_power(sensor::Sensor *s)       { power_sensor_ = s; }
  void set_energy(sensor::Sensor *s)      { energy_sensor_ = s; }

  void set_sensor_model(HLW8012SensorModel m) { sensor_model_ = m; }
  void set_model(HLW8012SensorModel m)        { sensor_model_ = m; }
  
  void set_initial_mode(HLW8012InitialMode mode) { current_mode_ = (mode == HLW8012_INITIAL_MODE_CURRENT); }
  void set_change_mode_every(uint32_t v)  { change_mode_every_ = v; }

  // Calibration setters - used by number entities
  void set_current_resistor(float r)      { current_resistor_ = r; recalc_ = true; }
  void set_voltage_divider(float v)       { voltage_divider_ = v; recalc_ = true; }

  void setup() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void recalculate_multipliers_();

  InternalGPIOPin *cf_pin_{nullptr};
  InternalGPIOPin *cf1_pin_{nullptr};
  InternalGPIOPin *sel_pin_{nullptr};

  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *energy_sensor_{nullptr};

  HLW8012SensorModel sensor_model_{HLW8012_SENSOR_MODEL_HLW8012};
  float current_resistor_{0.001f};
  float voltage_divider_{1535.36f};
  uint32_t change_mode_every_{8};

  bool current_mode_{true};
  uint32_t change_mode_at_{0};
  uint32_t nth_value_{0};
  bool recalc_{true};

  volatile uint32_t isr_cf_counter_{0};
  volatile uint32_t isr_cf1_counter_{0};
  uint32_t cf_timer_{0};
  uint32_t cf1_timer_{0};

  float power_multiplier_{0.0f};
  float current_multiplier_{0.0f};
  float voltage_multiplier_{0.0f};

  bool skip_next_cf1_{false};

  float get_hz_(uint32_t count, uint32_t delta);

  static void IRAM_ATTR cf_intr(HLW8012Component *arg);
  static void IRAM_ATTR cf1_intr(HLW8012Component *arg);
};

}  // namespace hlw8012
}  // namespace esphome