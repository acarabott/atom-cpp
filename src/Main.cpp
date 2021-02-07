#include <iostream>
#include <vector>

template<typename T>
using Subscription = std::function<void(const T &previousValue, const T &value)>;

template<typename T>
using Update = std::function<void(T &value)>;

template<typename T>
class Atomic {
public:
    Atomic() {};

    ~Atomic() {};

    virtual const T &get() const = 0;

    virtual void set(const T &value_) = 0;

    void update(const Update<T> &update) {
        assert(update != nullptr);
        auto value = get();
        update(value);
        set(value);
    }

    void subscribe(const Subscription<T> &update) {
        subscriptions.push_back(update);
    }

    void notifySubscriptions(const T &previousValue, const T &value) {
        for (const auto &sub : subscriptions) {
            sub(previousValue, value);
        }
    }

protected:
    std::vector<const Subscription<T>> subscriptions;
};

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

struct SubState {
    float value = 0.5f;
};

struct State {
    int count = 0;
    std::string name = "";

    SubState sub;
};

void increment(Atom<State> &db) {
    db.update([](auto &state) { state.count++; });
}

void decrement(Atom<State> &db) {
    db.update([](auto &state) { state.count--; });
}

int getCount(Atom<State> &db) {
    return db.get().count;
}

void printState(const State &state) {
    std::cout << "State: { " << std::endl
              << "    count: " << state.count << "," << std::endl
              << "    name: " << state.name << "," << std::endl
              << "    sub: { value: " << state.sub.value << " }," << std::endl
              << "}" << std::endl;
}

int main(int argc, char *argv[]) {
    // create atom
    Atom<State> db;

    // copy current state
    auto first = db.get();

    // add subscription
    db.subscribe([](const State &previousState, const State &state) {
        std::cout << "Subscription!" << std::endl;
        std::cout << "previous: " << std::endl;
        printState(previousState);
        std::cout << std::endl;

        std::cout << "new: " << std::endl;
        printState(state);
        std::cout << std::endl;
    });

    // modify state
    increment(db);
    increment(db);
    decrement(db);

    // create cursor
    Cursor<State, int> cursor(db, [](auto &state) -> int & { return state.count; });
    // get value via cursor
    std::cout << "cursor: " << cursor.get() << std::endl;
    // update value via cursor
    cursor.set(666);

    // print original state
    std::cout << "first: " << std::endl;
    printState(first);

    // print new state
    std::cout << "final count: " << getCount(db) << std::endl << std::endl;




    // create name cursor with macro
    auto nameCursor = DEF_CURSOR(db, name);
    nameCursor.subscribe([](const std::string &previousName, const std::string &name) {
        std::cout << "new name: " << name << std::endl;
    });
    nameCursor.set("jim");

    // create cursor directly
    Cursor<State, int> cursor2(db, [](auto &state) -> auto & { return state.count; });

    // trigger update to sub
    db.update([](auto state) {
        state.sub.value = 6.66f;
        return state;
    });

    // create cursor to sub value
    Cursor<State, float> valueCursor(db, [](State &state) -> auto & { return state.sub.value; });
    std::cout << "sub value: " << valueCursor.get() << std::endl;

    // subscribe to sub cursor
    valueCursor.subscribe([](auto previous, auto value) {
        std::cout << "sub value changed: " << value << std::endl;
    });

    // update sub
    valueCursor.set(3.33f);

    // create cursor with macro
    auto vc = DEF_CURSOR(db, sub.value);
    std::cout << "vc: " << vc.get() << std::endl;
}
