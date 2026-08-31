#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

template<typename T, int N>
class CircularQueue {
public:
    CircularQueue() : _head(0), _size(0) {}

    void push(const T& value) {
        _buffer[(_head + _size) % N] = value;
        if (_size < N) _size++;
        else _head = (_head + 1) % N;  // sobrescreve o mais antigo
    }

    T get(int index) const {
        return _buffer[(_head + index) % N];
    }

    int size() const { return _size; }
    bool isFull() const { return _size == N; }
    void clear() { _head = 0; _size = 0; }

private:
    T _buffer[N];
    int _head;
    int _size;
};

#endif