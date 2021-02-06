#include <iostream>
#include <vector>

template<class T>
class Atom {
public:
    using Update = std::function<void(T &value)>;

    const T get() const { return value; }

    void set(const T value_) {
        std::cout << "set" << std::endl;
        std::cout << "before" << std::endl;
        print();

        value = value_;

        std::cout << "after" << std::endl;
        print();
        std::cout << std::endl;

        for (const auto &sub : subs) {
            sub(value);
        }
    }

    void subscribe(const Update &update) {
        subs.push_back(update);
    }

    void print() const {
        if (printValue != nullptr) {
            printValue(value);
        }
    }

    std::function<void(const T &value)> printValue;


private:
    T value;

    std::vector<const Update> subs;
};

struct State {
    int count = 0;
};

void increment(Atom<State> &db) {
    auto state = db.get();
    std::cout << "increment: " << state.count << std::endl;
    state.count++;
    db.set(state);
}

void decrement(Atom<State> &db) {
    auto state = db.get();
    state.count--;
    db.set(state);
}

int getCount(Atom<State> &db) {
    return db.get().count;
}

void printState(const State &state) {
    std::cout << "State: { count: " << state.count << " }" << std::endl;
}

int main(int argc, char *argv[]) {

    Atom<State> db;

    db.printValue = [](const State &state) {
        printState(state);
    };

    db.subscribe([](State newState) {
        std::cout << "Update!" << std::endl;

        printState(newState);
        std::cout << std::endl;
    });

    increment(db);
    increment(db);
    decrement(db);

    std::cout << "final count: " << getCount(db) << std::endl;
}
