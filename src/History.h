#pragma once

#include "Atomic.h"
#include <vector>

template<typename T>
class History {

public:
    History(Atomic<T> &atom) : mAtom(atom) {
        store();

        mAtom.subscribe([this](auto &previous, auto &value) {
            (void)previous;
            (void)value;

            store();
        });
    }

    int size() const {
        return mHistory.size();
    }

    T get(const int index) const {
        assert(index < size());
        return mHistory[index];
    }

    void set(const int index) {
        mAtom.set(get(index));
    }

    ~History() {};


protected:
    void store() {
        mHistory.push_back(mAtom.get());
    }

    Atomic<T> &mAtom;
    std::vector<T> mHistory;
};
