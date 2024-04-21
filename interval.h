#ifndef INTERVAL_H
#define INTERVAL_H

#include "util.h"


class Interval
{
public:
    constexpr Interval(Time start, Time end, Time total)
        : _time(0), _start(start), _end(end), _total(total) {}

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
            enter();
        if (_time >= _end and _time <= _end + elapsed)
            exit();
        if (_time >= _total)
            _time -= _total;
    }

private:
    Time _time;
    const Time _start;
    const Time _end;
    const Time _total;

};


#endif // INTERVAL_H
