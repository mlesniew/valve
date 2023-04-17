#pragma once

#include <string>

#include <PicoPrometheus.h>
#include <utils/timedvalue.h>

template <typename State>
class NamedFSM {
    public:
        NamedFSM(const char * name, const State state)
            : name(name), state(state) {
        }

        virtual ~NamedFSM() {}

        virtual void update_metric() const = 0;
        virtual void update_mqtt() const = 0;

        void set_name(const char * name) {
            delete_metric();
            this->name = name;
            update_metric();
            update_mqtt();
        }

        const char * get_name() const {
            return name.c_str();
        }

        const State get_state() const {
            return state;
        }

    protected:
        PrometheusLabels get_prometheus_labels() const {
            return {{"zone", name}};
        }

        unsigned long get_millis_elapsed_in_current_state() const { return state.elapsed_millis(); }

        void set_state(const State new_state) {
            if (state == new_state) {
                return;
            }
            Serial.printf("%s '%s' changing state from %s to %s.\n", get_class_name(), name.c_str(), to_c_str(state),
                          to_c_str(new_state));
            state = new_state;
            update_metric();
            update_mqtt();
        }

        virtual const char * get_class_name() const = 0;
        virtual void delete_metric() const = 0;

    private:
        std::string name;
        TimedValue<State> state;
};
