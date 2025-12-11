#include "hlw8012.h"
#include "esphome/core/log.h"

namespace esphome {
namespace hlw8012 {

static const char *const TAG = "hlw8012";
static const uint32_t CLOCK_FREQ = 3579000;

void HLW8012Component::recalculate_multipliers_() {
  float ref = (sensor_model_ == HLW8012_SENSOR_MODEL_BL0937) ? 1.218f : 2.43f;
  if (sensor_model_ == HLW8012_SENSOR_MODEL_BL0937) {
    power_multiplier_   = ref * ref * voltage_divider_ / current_resistor_ / 1750000.0f;
    current_multiplier_ = ref / current_resistor_ / 95000.0f;
    voltage_multiplier_ = ref * voltage_divider_ / 15500.0f;
  } else {
    power_multiplier_   = ref * ref * voltage_divider_ / current_resistor_ * 64.0f / 24.0f / CLOCK_FREQ;
    current_multiplier_ = ref / current_resistor_ * 512.0f / 24.0f / CLOCK_FREQ;
    voltage_multiplier_ = ref * voltage_divider_ * 256.0f / CLOCK_FREQ;
  }
}

void HLW8012Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up HLW8012...");
  if (sel_pin_) { sel_pin_->setup(); sel_pin_->digital_write(current_mode_); }
  cf_pin_->setup();  cf_timer_ = micros();  cf_pin_->attach_interrupt(cf_intr, this, gpio::INTERRUPT_RISING_EDGE);
  cf1_pin_->setup(); cf1_timer_ = micros(); cf1_pin_->attach_interrupt(cf1_intr, this, gpio::INTERRUPT_RISING_EDGE);
  recalculate_multipliers_();
}

void HLW8012Component::dump_config() {
  ESP_LOGCONFIG(TAG, "HLW8012:");
  LOG_PIN("  SEL Pin: ", sel_pin_);
  LOG_PIN("  CF Pin:  ", cf_pin_);
  LOG_PIN("  CF1 Pin: ", cf1_pin_);
  ESP_LOGCONFIG(TAG, "  Model: %s", sensor_model_ == HLW8012_SENSOR_MODEL_BL0937 ? "BL0937" : "HLW8012/CSE7759");
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Voltage", voltage_sensor_);
  LOG_SENSOR("  ", "Current", current_sensor_);
  LOG_SENSOR("  ", "Power",   power_sensor_);
  LOG_SENSOR("  ", "Energy",  energy_sensor_);
}

float HLW8012Component::get_hz_(uint32_t count, uint32_t delta) {
  return delta > 0 ? (count * 1000000.0f) / delta : 0.0f;
}

void HLW8012Component::update() {
  if (recalc_) { recalculate_multipliers_(); recalc_ = false; }

  if (change_mode_every_ && change_mode_at_++ == change_mode_every_) {
    if (sel_pin_) { current_mode_ = !current_mode_; sel_pin_->digital_write(current_mode_); }
    change_mode_at_ = 0;
    skip_next_cf1_ = true;
    return;
  }

  uint32_t cf_count  = isr_cf_counter_;  isr_cf_counter_  = 0;
  uint32_t cf1_count = isr_cf1_counter_; isr_cf1_counter_ = 0;
  uint32_t cf_time   = cf_timer_;        cf_timer_ = micros();
  uint32_t cf1_time  = cf1_timer_;       cf1_timer_ = micros();

  if (nth_value_++ < 2) return;

  float cf_hz  = get_hz_(cf_count,  micros() - cf_time);
  float cf1_hz = get_hz_(cf1_count, micros() - cf1_time);

  float power = cf_hz * power_multiplier_;
  if (power_sensor_) power_sensor_->publish_state(power);

  if (skip_next_cf1_) {
    skip_next_cf1_ = false;
  } else if (current_mode_) {
    float voltage = cf1_hz * voltage_multiplier_;
    if (voltage_sensor_) voltage_sensor_->publish_state(voltage);
  } else {
    float current = cf1_hz * current_multiplier_;
    if (current_sensor_) current_sensor_->publish_state(current);
  }

  float delta_s = (micros() - cf_time) / 1000000.0f;
  if (energy_sensor_ && delta_s > 0 && power >= 0)
    energy_sensor_->publish_state(energy_sensor_->state + power * delta_s / 3600.0f);
}

void IRAM_ATTR HLW8012Component::cf_intr(HLW8012Component *arg)  { arg->isr_cf_counter_++; }
void IRAM_ATTR HLW8012Component::cf1_intr(HLW8012Component *arg) { arg->isr_cf1_counter_++; }

}  // namespace hlw8012
}  // namespace esphome