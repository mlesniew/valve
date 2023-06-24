#include <PicoMQTT.h>
#include <PicoPrometheus.h>

#include "valve.h"

Prometheus & get_prometheus();
PicoMQTT::Publisher & get_mqtt_publisher();

static PrometheusGauge gauge_valve_state(get_prometheus(), "valve_state", "Valve state enum");

const char * to_c_str(const ValveState & s) {
    switch (s) {
        case ValveState::open:
            return "open";
        case ValveState::closed:
            return "closed";
        case ValveState::opening:
            return "opening";
        case ValveState::closing:
            return "closing";
        default:
            return "error";
    }
}

ValveState parse_valve_state(const std::string & s) {
    if (s == "closed") { return ValveState::closed; }
    if (s == "closing") { return ValveState::closing; }
    if (s == "open") { return ValveState::open; }
    if (s == "opening") { return ValveState::opening; }
    return ValveState::error;
}

Valve::Valve(PicoUtils::BinaryOutput & output, const char * name, const unsigned long switch_time_millis)
    : NamedFSM(name, ValveState::closed), switch_time_millis(switch_time_millis),
      demand_open(false), output(output) {
    update_metric();
    update_mqtt();
}

void Valve::delete_metric() const {
    gauge_valve_state.remove(get_prometheus_labels());
}

void Valve::update_metric() const {
    gauge_valve_state[get_prometheus_labels()].set(static_cast<typename std::underlying_type<ValveState>::type>
            (get_state()));
}

void Valve::update_mqtt() const {
    const std::string topic = std::string("valvola/valve/") + get_name();
    get_mqtt_publisher().publish(topic.c_str(), to_c_str(get_state()));
}

void Valve::tick() {
    const bool switch_time_elapsed = (get_millis_elapsed_in_current_state() >= switch_time_millis);

    switch (get_state()) {
        case ValveState::closing:
            if (!demand_open && switch_time_elapsed) {
                set_state(ValveState::closed);
            }
        // fall through

        case ValveState::closed:
            if (demand_open) {
                set_state(ValveState::opening);
            }
            break;

        case ValveState::opening:
            if (demand_open && switch_time_elapsed) {
                set_state(ValveState::open);
            }
        // fall through

        case ValveState::open:
            if (!demand_open) {
                set_state(ValveState::closing);
            }
            break;

        default:
            break;
    }

    output.set(demand_open);
}

DynamicJsonDocument Valve::get_config() const {
    DynamicJsonDocument json(64);

    json["name"] = get_name();
    json["switch_time"] = double(switch_time_millis) * 0.001;

    return json;
}

DynamicJsonDocument Valve::get_status() const {
    DynamicJsonDocument json = get_config();

    json["state"] = to_c_str(get_state());

    return json;
}

bool Valve::set_config(const JsonVariantConst & json) {
    const auto object = json.as<JsonObjectConst>();

    if (object.containsKey("name")) {
        set_name(object["name"].as<std::string>().c_str());
    }

    if (object.containsKey("switch_time")) {
        switch_time_millis = object["switch_time"].as<double>() * 1000;
    }

    return true;
}
