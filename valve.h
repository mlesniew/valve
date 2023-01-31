#ifndef CALOR_VALVE_H
#define CALOR_VALVE_H

#include <string>

#include <utils/io.h>
#include <utils/tickable.h>
#include <utils/timedvalue.h>

#include <ArduinoJson.h>

class Valve: public Tickable {
    public:
        enum class State {
            closed = 0,
            opening = 1,
            closing = 2,
            open = 3,
            error = -1,
        };

        Valve(BinaryOutput & output, const std::string & name = "", const unsigned long switch_time_millis = 0);
        virtual ~Valve() {}

        void tick() override;
        State get_state() const;

        DynamicJsonDocument get_config() const;
        DynamicJsonDocument get_status() const;
        bool set_config(const JsonVariantConst & json);

        std::string name;
        unsigned long switch_time_millis;
        bool demand_open;

    protected:
        void set_state(const State new_state);
        virtual void on_state_change() const {}

    public:
        BinaryOutput & output;
        TimedValue<State> state;
};

const char * to_c_str(const Valve::State & s);

#endif
