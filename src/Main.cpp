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
    using Get = std::function<T_Value(T_State)>;
    using Set = std::function<void(T_Value, T_State &)>;
    using Subscription = std::function<void(T_Value &value)>;
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

    const auto getCount = [](State state) { return state.count; };
    const auto setCount = [](int value, State &state) { state.count = value; };

    Cursor<State, int> countCursor(db, getCount, setCount);

    std::cout << "cursor: " << countCursor.get() << std::endl;

    countCursor.subscribe([](int count) {
        std::cout << "cursor sub: " << count << std::endl;
    });

    countCursor.set(6);
}
