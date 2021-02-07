#pragma once

#include "Atomic.h"

template<class T>
class Atom : public Atomic<T> {
public:

    const T &get() const override { return value; }

    void set(const T &value_) override {
        previousValue = value;
        value = value_;

        Atomic<T>::notifySubscriptions(previousValue, value);
    }

private:
    T value;
    T previousValue;
};
