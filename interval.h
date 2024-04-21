#ifndef INTERVAL_H
#define INTERVAL_H

#include "util.h"


class Interval
{
public:
    constexpr Interval(Time start, Time end, Time total, const char* name)
        : _time(0), _start(start), _end(end), _total(total), _name(name) {}

public:
    virtual void enter() = 0;
    virtual void exit() = 0;

public:
    void load(Time time)
    {
        _time = time;
        if (_time >= _start and _time <= _end)
            enter();
        if (_time >= _end and _time <= _total)
            exit();
    }

    Time save() const
    {
        return _time;
    }

    void tick(Time elapsed)
    {
        _time += elapsed;
        if (_time >= _start and _time <= _start + elapsed)
            _enter();
        if (_time >= _end and _time <= _end + elapsed)
            _exit();
        if (_time >= _total)
            _time -= _total;
    }

    void _enter()
    {
        if (_name) {
            Serial.print(_name);
            Serial.print(" enter at ");
            Serial.print(millis());
            Serial.println("ms");
        }
        enter();
    }

    void _exit()
    {
        if (_name) {
            Serial.print(_name);
            Serial.print(" exit at ");
            Serial.print(millis());
            Serial.println("ms");
        }
        exit();
    }

private:
    Time _time;
    const Time _start;
    const Time _end;
    const Time _total;
    const char* const _name;

};


#endif // INTERVAL_H
