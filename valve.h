#pragma once

#include <string>

#include <PicoUtils.h>

#include <ArduinoJson.h>
#include "namedfsm.h"


enum class ValveState {
    closed = 0,
    opening = 1,
    closing = 2,
    open = 3,
    error = -1,
};

class Valve: public PicoUtils::Tickable, public NamedFSM<ValveState> {
    public:
        Valve(PicoUtils::BinaryOutput & output, const char * name = "", const unsigned long switch_time_millis = 0);
        virtual ~Valve() { delete_metric(); }

        void tick() override;

        void update_metric() const override;
        void update_mqtt() const override;

        DynamicJsonDocument get_config() const;
        DynamicJsonDocument get_status() const;
        bool set_config(const JsonVariantConst & json);

        unsigned long switch_time_millis;
        bool demand_open;
    protected:
        void delete_metric() const override;
        virtual const char * get_class_name() const override { return "Valve"; }

        PicoUtils::BinaryOutput & output;
};

const char * to_c_str(const ValveState & s);
ValveState parse_valve_state(const std::string & s);
