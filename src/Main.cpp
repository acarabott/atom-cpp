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

template<typename C>
struct get_template_type;

template<template<typename> class C, typename T>
struct get_template_type<C<T>> {
    using type = T;
};

template<class T_State, class T_Value>
class Cursor {
    using Get = std::function<T_Value(const T_State &)>;
    using Set = std::function<void(const T_Value &, T_State &)>;
    using Subscription = std::function<void(const T_Value &value)>;
public:
    Cursor(Atom<T_State> &atom, Get get, Set set)
            : mAtom(atom), mGet(get), mSet(set) {}

    T_Value get() {
        return mGet(mAtom.get());
    }

    void set(T_Value value) {
        auto state = mAtom.get();
        mSet(value, state);
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
    Get mGet = nullptr;
    Set mSet = nullptr;
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


#define defCursor(ATOM, PROP) \
    Cursor<get_template_type<typeof ATOM>::type, decltype(std::declval<get_template_type<typeof ATOM>::type>().PROP)>(ATOM, \
        [](const auto state) { return state.PROP; }, \
        [](const auto& value, auto &state) { state.PROP = value; })

int main(int argc, char *argv[]) {

    Atom<State> db;

    db.subscribe([](State newState) {
        std::cout << "Subscription!" << std::endl;

        printState(newState);
        std::cout << std::endl;
    });

    increment(db);
    increment(db);
    decrement(db);

    std::cout << "final count: " << getCount(db) << std::endl << std::endl;

//    const auto getCount = [](State state) { return state.count; };
//    const auto setCount = [](int value, State &state) { state.count = value; };
//
//    Cursor<State, int> countCursor(db, getCount, setCount);

    auto countCursor = defCursor(db, count);
    std::cout << "cursor: " << countCursor.get() << std::endl;
    countCursor.subscribe([](int count) {
        std::cout << "cursor sub: " << count << std::endl;
    });

    countCursor.set(6);

    auto nameCursor = defCursor(db, name);
    nameCursor.subscribe([](auto &name) {
        std::cout << "new name: " << name << std::endl;
    });
    nameCursor.set("jim");
}
