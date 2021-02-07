#include <iostream>
#include <vector>

template<class T>
class Atom {
public:
    using Subscription = std::function<void(T &value)>;

    const T get() const { return value; }

    void set(const T value_) {
        value = value_;

        for (const auto &sub : subs) {
            sub(value);
        }
    }

    void update(std::function<T(const T value)> update) {
        if (update != nullptr) {
            set(update(get()));
        }
    }

    void subscribe(const Subscription &update) {
        subs.push_back(update);
    }

private:
    T value;

    std::vector<const Subscription> subs;
};

template<class T_State, class T_Value>
class Cursor {
    using Access = std::function<T_Value &(T_State &)>;
    using Subscription = std::function<void(const T_Value &value)>;
public:
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
        mAccess(state) = value;
        mAtom.set(state);

        for (const auto &sub : subs) {
            sub(value);
        }
    }

    void subscribe(const Subscription &subscriber) {
        subs.push_back(subscriber);
    }

    ~Cursor() {};

private:
    Atom<T_State> &mAtom;
    Access mAccess = nullptr;
    std::vector<const Subscription> subs;
};

struct State {
    int count = 0;
    std::string name = "";
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
    std::cout << "State: { count: " << state.count << " }" << std::endl;
}

template<typename C>
struct get_template_type;

template<template<typename> class C, typename T>
struct get_template_type<C<T>> {
    using type = T;
};

//#define STATE_TYPE(ATOM) get_template_type<typeof ATOM>::type
//#define DEF_CURSOR(ATOM, PROP) \
//    Cursor<STATE_TYPE(ATOM), decltype(std::declval<STATE_TYPE(ATOM)>().PROP)>(ATOM, \
//        [](const auto state) { return state.PROP; }, \
//        [](const auto& value, auto &state) { state.PROP = value; })

//template<typename T_State, typename T_Value>
//Cursor<T_State, T_Value> defCursor(Atom<T_State> db, std::function<T_Value *(const T_State &)> accessor) {
//    return Cursor<T_State, T_Value>(db,
//                                    [accessor](const auto state) { return *accessor(state); },
//                                    [accessor](const T_Value &value, T_State &state) { *accessor(state) = value; });
//}

int main(int argc, char *argv[]) {

    Atom<State> db;

    auto first = db.get();

    db.subscribe([](State newState) {
        std::cout << "Subscription!" << std::endl;

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

//    auto nameCursor = DEF_CURSOR(db, name);
//    nameCursor.subscribe([](auto &name) {
//        std::cout << "new name: " << name << std::endl;
//    });
//    nameCursor.set("jim");
//
//    auto thing = defCursor<State, int>(db, [](auto state) { return &(state.count); });

//    thing.subscribe([](auto count) {
//        std::cout << "super new count: " << count << std::endl;
//    });

//    thing.set(666);

}
