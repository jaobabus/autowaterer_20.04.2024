#include "util.h"
#include "interval.h"
#include <EEPROM.h>


constexpr uint8_t relay1_pin = 10;
constexpr uint8_t relay2_pin = 11;
constexpr uint8_t button_pin = 12;
constexpr uint8_t autosave_led_pin = 13;


class Relay1Interval : public Interval
{
public:
    Relay1Interval(uint8_t pin, Time s, Time e, Time t)
        : _pin(pin), Interval(s, e, t) {}

public:
    void change_pin() {
        digitalWrite(_pin, enabled);
    }
    void enter() override {
        enabled = true;
        change_pin();
    }
    void exit() override {
        enabled = false;
        change_pin();
    }

public:
    bool enabled;

private:
    const uint8_t _pin;

};


class Relay2Interval : public Interval
{
public:
    Relay2Interval(const Relay1Interval* relay1, bool master_state, uint8_t pin, Time s, Time e, Time t)
        : _relay1(relay1), _master_state(master_state), _pin(pin), Interval(s, e, t) {}

public:
    void change_pin() {
        if (_relay1->enabled == _master_state)
            digitalWrite(_pin, enabled);
    }
    void enter() override {
        enabled = true;
        change_pin();
    }
    void exit() override {
        enabled = false;
        change_pin();
    }

public:
    bool enabled;

private:
    const Relay1Interval* const _relay1;
    const bool _master_state;
    const uint8_t _pin;

};


class AutosaveInterval : public Interval
{
public:
    using Interval::Interval;

public:
    void enter() override;
    void exit() override;

};


class CheckButtonInterval : public Interval
{
public:
    CheckButtonInterval(uint8_t pin, Time s, Time e, Time t)
        : _pin(pin), _last(false), Interval(s, e, t) {}

public:
    void enter() override;
    void exit() override {
        _last = digitalRead(_pin);
    }

private:
    const uint8_t _pin;
    bool _last;

};


Relay1Interval relay1(relay1_pin, 0ULL * 1000 * 3600, 16ULL * 1000 * 3600, 24ULL * 1000 * 3600);
Relay2Interval relay2_true(&relay1, true, relay2_pin, 0ULL * 1000 * 3600, 3ULL * 1000 * 60, 30ULL * 1000 * 60);
Relay2Interval relay2_false(&relay1, false, relay2_pin, 0ULL * 1000 * 3600, 3ULL * 1000 * 60, 120ULL * 1000 * 60);
AutosaveInterval autosave(0ULL * 1000, 0ULL * 1000 + 100, 30ULL * 1000);
CheckButtonInterval check_button(button_pin, 0ULL, 25ULL, 50ULL);
constexpr Interval* intervals[] = {&relay1, &relay2_true, &relay2_false, &autosave, &check_button};


struct State
{
    static constexpr auto intervals_count = sizeof(intervals) / sizeof(intervals)[0];
    struct Data {
        Time times[intervals_count];
    };
    Data data;
    uint32_t crc;
};

uint32_t get_crc(const void* data, size_t size)
{
    uint32_t crc = 0;
    for (size_t i = 0; i < size; i++)
        crc ^= ((const uint8_t*)data)[i] << (i % 32);
    return crc;
}


void load_state()
{
    State state;
    EEPROM.get(0, state);
    if (state.crc == get_crc(&state.data, sizeof(state.data))) {
        for (size_t i = 0; i < State::intervals_count; i++)
            intervals[i]->load(state.data.times[i]);
    }
}

void save_state()
{
    State state;
    for (size_t i = 0; i < State::intervals_count; i++)
        state.data.times[i] = intervals[i]->save();
    state.crc = get_crc(&state.data, sizeof(state.data));
    EEPROM.put(0, state);
}

void reset_state()
{
    State state;
    for (size_t i = 0; i < State::intervals_count; i++)
        state.data.times[i] = 0;
    state.crc = get_crc(&state.data, sizeof(state.data));
    EEPROM.put(0, state);
    load_state();
}

void setup()
{
    pinMode(button_pin, INPUT_PULLUP);
    pinMode(relay1_pin, OUTPUT);
    pinMode(relay2_pin, OUTPUT);
    load_state();
}

void AutosaveInterval::enter()
{
    save_state();
    digitalWrite(autosave_led_pin, true);
}

void AutosaveInterval::exit()
{
    digitalWrite(autosave_led_pin, false);
}

void CheckButtonInterval::enter()
{
    if (_last != digitalRead(_pin) and digitalRead(_pin))
        reset_state();
    _last = digitalRead(_pin);
}

void loop()
{
    static Time last_time = 0;
    auto now = millis();
    auto elapsed = now - last_time;
    if (elapsed != 0) {
        if (last_time != 0)
            for (auto rel : intervals)
                rel->tick(elapsed);
        last_time = now;
    }
}
