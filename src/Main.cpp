#include <iostream>
#include <vector>

template<typename T>
using Subscription = std::function<void(const T &value, const T &previousValue)>;

template<typename T>
class Subscribable {
public:

    virtual const T &get() const = 0;

    virtual void set(const T &value_) = 0;
};

template<class T>
class Atom : public Subscribable<T> {
public:

    const T &get() const override { return value; }

    void set(const T &value_) override {
        previousValue = value;
        value = value_;

        for (const auto &sub : subscriptions) {
            sub(value, previousValue);
        }
    }

    void update(std::function<T(const T &value)> update) {
        if (update != nullptr) {
            set(update(get()));
        }
    }

    void subscribe(const Subscription<T> &update) {
        subscriptions.push_back(update);
    }

private:
    T value;
    T previousValue;

    std::vector<const Subscription<T>> subscriptions;
};

template<class T_State, class T_Value>
class Cursor {
public:
    using Access = std::function<T_Value &(T_State &)>;

    Cursor(Atom<T_State> &atom, Access access)
            : mAtom(atom), mAccess(access) {

        assert(mAccess != nullptr);
    }

    T_Value get() {
        auto state = mAtom.get();
        return mAccess(state);
    }

    void set(T_Value value) {
        auto state = mAtom.get();
        auto previous = mAccess(state);
        mAccess(state) = value;
        mAtom.set(state);

        for (const auto &sub : subscriptions) {
            sub(value, previous);
        }
    }

    void subscribe(const Subscription<T_Value> &subscriber) {
        subscriptions.push_back(subscriber);
    }

    ~Cursor() {};

private:
    Atom<T_State> &mAtom;
    Access mAccess = nullptr;
    std::vector<const Subscription<T_Value>> subscriptions;
};

struct SubState {
    float value = 0.5f;
};

struct State {
    int count = 0;
    std::string name = "";

    SubState sub;
};

void increment(Atom<State> &db) {
    db.update([](State state) {
        state.count++;
        return state;
    });
}

void decrement(Atom<State> &db) {
    db.update([](State state) {
        state.count--;
        return state;
    });
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

template<typename T_State, typename T_Value>
Cursor<T_State, T_Value> defCursor(Atom<T_State> db, std::function<T_Value &(T_State &)> accessor) {
    return Cursor<T_State, T_Value>(db, accessor);
}


int main(int argc, char *argv[]) {

    Atom<State> db;

    auto first = db.get();

    db.subscribe([](const State &newState, const State &previousState) {
        std::cout << "Subscription!" << std::endl;
        std::cout << "previous: " << std::endl;
        printState(previousState);
        std::cout << std::endl;

        std::cout << "new: " << std::endl;
        printState(newState);
        std::cout << std::endl;
    });

    increment(db);
    increment(db);
    decrement(db);

    std::cout << "final count: " << getCount(db) << std::endl << std::endl;

    Cursor<State, int> cursor(db, [](auto &state) -> int & { return state.count; });

    std::cout << "cursor: " << cursor.get() << std::endl;

    cursor.set(666);

    std::cout << "first: " << std::endl;
    printState(first);


//    const auto getCount = [](State state) { return state.count; };
//    const auto setCount = [](int value, State &state) { state.count = value; };
//
//    Cursor<State, int> countCursor(db, getCount, setCount);

//    auto countCursor = DEF_CURSOR(db, count);
//    std::cout << "cursor: " << countCursor.get() << std::endl;
//    countCursor.subscribe([](int count) {
//        std::cout << "cursor sub: " << count << std::endl;
//    });
//
//    countCursor.set(6);

    auto nameCursor = DEF_CURSOR(db, name);
    nameCursor.subscribe([](const std::string &name, const std::string &previousName) {
        std::cout << "new name: " << name << std::endl;
    });
    nameCursor.set("jim");
//
    auto thing = defCursor<State, int>(db, [](State &state) -> int & { return state.count; });
    thing.subscribe([](auto count, auto previousCount) {
        std::cout << "super new count: " << count << std::endl;
    });
    thing.set(666);

    auto thing2 = defCursor<State, int>(db, [](auto &state) -> auto & { return state.count; });

    Cursor<State, int> cursor2(db, [](auto &state) -> auto & { return state.count; });


    db.update([](auto state) {
        state.sub.value = 6.66f;
        return state;
    });

    Cursor<State, float> valueCursor(db, [](State &state) -> auto & { return state.sub.value; });

    std::cout << "sub value: " << valueCursor.get() << std::endl;
    valueCursor.subscribe([](auto value, auto previous) {
        std::cout << "sub value changed: " << value << std::endl;
    });

    valueCursor.set(3.33f);

    auto vc = DEF_CURSOR(db, sub.value);
    std::cout << "vc: " << vc.get() << std::endl;
}
