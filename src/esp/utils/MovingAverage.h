#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include "../config/Constants.h"

template<int N>
class MovingAverage {
public:
    MovingAverage() : _index(0), _count(0), _sum(0.0) {
        for (int i = 0; i < N; i++) _buffer[i] = 0.0;
    }

    double filter(double input) {
        _sum -= _buffer[_index];
        _buffer[_index] = input;
        _sum += input;
        _index = (_index + 1) % N;
        if (_count < N) _count++;
        return _sum / _count;
    }

    void reset() {
        _index = 0;
        _count = 0;
        _sum   = 0.0;
        for (int i = 0; i < N; i++) _buffer[i] = 0.0;
    }

private:
    double _buffer[N];
    int _index;
    int _count;
    double _sum;
};

#endif