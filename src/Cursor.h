#pragma once

#include "Atomic.h"
#include "Atom.h"

template<class T_State, class T_Value>
class Cursor : public Atomic<T_Value> {
public:
    using Access = std::function<T_Value &(T_State &)>;

    Cursor(Atom<T_State> &atom, Access access)
            : mAtom(atom), mAccess(access) {

        assert(mAccess != nullptr);
    }

    const T_Value &get() const override {
        auto state = mAtom.get();
        return mAccess(state);
    }

    void set(const T_Value &value) override {
        auto state = mAtom.get();
        auto previous = mAccess(state);
        mAccess(state) = value;
        mAtom.set(state);

        Atomic<T_Value>::notifySubscriptions(previous, value);
    }

    ~Cursor() {};

private:
    Atom<T_State> &mAtom;
    Access mAccess = nullptr;
};

// helpers for creation
template<typename C>
struct get_template_type;

template<template<typename> class C, typename T>
struct get_template_type<C<T>> {
    using type = T;
};

#define STATE_TYPE(ATOM) get_template_type<typeof ATOM>::type
#define PROP_TYPE(ATOM, PROP) decltype(std::declval<STATE_TYPE(ATOM)>().PROP)
#define DEF_CURSOR(ATOM, PROP) \
    Cursor<STATE_TYPE(ATOM), PROP_TYPE(ATOM, PROP)>(ATOM, \
        [](STATE_TYPE(ATOM) &state) -> PROP_TYPE(ATOM, PROP) & { return state.PROP; });

// for some reason, using this creates SIGABRT issues when calling .set with a cursor made with it
template<typename T_State, typename T_Value>
Cursor<T_State, T_Value> defCursor(Atom<T_State> db, std::function<T_Value &(T_State &)> accessor) {
    return Cursor<T_State, T_Value>(db, accessor);
}
