#include "valve.h"

const char * to_c_str(const Valve::State & s) {
    switch (s) {
        case Valve::State::open:
            return "open";
        case Valve::State::closed:
            return "closed";
        case Valve::State::opening:
            return "opening";
        case Valve::State::closing:
            return "closing";
        default:
            return "error";
    }
}

Valve::State parse_valve_state(const std::string & s) {
    if (s == "closed") { return Valve::State::closed; }
    if (s == "closing") { return Valve::State::closing; }
    if (s == "open") { return Valve::State::open; }
    if (s == "opening") { return Valve::State::opening; }
    return Valve::State::error;
}

Valve::Valve(BinaryOutput & output, const std::string & name, const unsigned long switch_time_millis)
    : name(name), switch_time_millis(switch_time_millis),
      demand_open(false), output(output), state(Valve::State::closed) {
}

void Valve::tick() {
    const bool switch_time_elapsed = (state.elapsed_millis() >= switch_time_millis);

    switch (state) {
        case State::closing:
            if (!demand_open && switch_time_elapsed) {
                set_state(State::closed);
                printf("Valve '%s' switching state to '%s'.\n", name.c_str(), to_c_str(state));
            }
        // fall through

        case State::closed:
            if (demand_open) {
                set_state(State::opening);
                printf("Valve '%s' switching state to '%s'.\n", name.c_str(), to_c_str(state));
            }
            break;

        case State::opening:
            if (demand_open && switch_time_elapsed) {
                set_state(State::open);
                printf("Valve '%s' switching state to '%s'.\n", name.c_str(), to_c_str(state));
            }
        // fall through

        case State::open:
            if (!demand_open) {
                set_state(State::closing);
                printf("Valve '%s' switching state to '%s'.\n", name.c_str(), to_c_str(state));
            }
            break;

        default:
            break;
    }

    output.set(demand_open);
}

void Valve::set_state(const Valve::State new_state) {
    if (new_state != state) {
        state = new_state;
        on_state_change();
    }
}

Valve::State Valve::get_state() const {
    return state;
}

DynamicJsonDocument Valve::get_config() const {
    DynamicJsonDocument json(64);

    json["name"] = name;
    json["switch_time"] = double(switch_time_millis) * 0.001;

    return json;
}

DynamicJsonDocument Valve::get_status() const {
    DynamicJsonDocument json = get_config();

    json["state"] = to_c_str(state);

    return json;
}

bool Valve::set_config(const JsonVariantConst & json) {
    const auto object = json.as<JsonObjectConst>();

    if (object.containsKey("name")) {
        name = object["name"].as<std::string>();
    }

    if (object.containsKey("switch_time")) {
        switch_time_millis = object["switch_time"].as<double>() * 1000;
    }

    return true;
}
