import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import sensor
from esphome.const import (
    CONF_CHANGE_MODE_EVERY,
    CONF_CURRENT,
    CONF_CURRENT_RESISTOR,
    CONF_ENERGY,
    CONF_ID,
    CONF_MODEL,
    CONF_POWER,
    CONF_SEL_PIN,
    CONF_VOLTAGE,
    CONF_VOLTAGE_DIVIDER,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_KILOWATT_HOURS,
    UNIT_VOLT,
    UNIT_WATT,
)

CONF_CF_PIN = "cf_pin"
CONF_CF1_PIN = "cf1_pin"
CONF_INITIAL_MODE = "initial_mode"

hlw8012_ns = cg.esphome_ns.namespace("hlw8012")
HLW8012Component = hlw8012_ns.class_("HLW8012Component", cg.PollingComponent)

HLW8012SensorModel = hlw8012_ns.enum("HLW8012SensorModel")
HLW8012InitialMode = hlw8012_ns.enum("HLW8012InitialMode")

SENSOR_MODELS = {
    "HLW8012": HLW8012SensorModel.HLW8012_SENSOR_MODEL_HLW8012,
    "CSE7759": HLW8012SensorModel.HLW8012_SENSOR_MODEL_CSE7759,
    "BL0937": HLW8012SensorModel.HLW8012_SENSOR_MODEL_BL0937,
}

INITIAL_MODES = {
    "CURRENT": HLW8012InitialMode.HLW8012_INITIAL_MODE_CURRENT,
    "VOLTAGE": HLW8012InitialMode.HLW8012_INITIAL_MODE_VOLTAGE,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HLW8012Component),
        cv.Required(CONF_SEL_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_CF_PIN): pins.internal_gpio_input_pin_schema,
        cv.Required(CONF_CF1_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ENERGY): sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_CURRENT_RESISTOR, default=0.001): cv.resistance,
        cv.Optional(CONF_VOLTAGE_DIVIDER, default=2351): cv.positive_float,
        cv.Optional(CONF_MODEL, default="HLW8012"): cv.enum(SENSOR_MODELS, upper=True),
        cv.Optional(CONF_CHANGE_MODE_EVERY, default=8): cv.uint32_t,
        cv.Optional(CONF_INITIAL_MODE, default="VOLTAGE"): cv.enum(
            INITIAL_MODES, upper=True
        ),
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sel_pin = await cg.gpio_pin_expression(config[CONF_SEL_PIN])
    cg.add(var.set_sel_pin(sel_pin))
    cf_pin = await cg.gpio_pin_expression(config[CONF_CF_PIN])
    cg.add(var.set_cf_pin(cf_pin))
    cf1_pin = await cg.gpio_pin_expression(config[CONF_CF1_PIN])
    cg.add(var.set_cf1_pin(cf1_pin))

    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(sens))
    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_POWER in config:
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sens))
    if CONF_ENERGY in config:
        sens = await sensor.new_sensor(config[CONF_ENERGY])
        cg.add(var.set_energy_sensor(sens))

    cg.add(var.set_current_resistor(config[CONF_CURRENT_RESISTOR]))
    cg.add(var.set_voltage_divider(config[CONF_VOLTAGE_DIVIDER]))
    cg.add(var.set_sensor_model(config[CONF_MODEL]))
    cg.add(var.set_change_mode_every(config[CONF_CHANGE_MODE_EVERY]))
    cg.add(var.set_initial_mode(config[CONF_INITIAL_MODE]))
