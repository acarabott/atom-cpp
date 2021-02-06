#include <iostream>
#include <vector>

template<class T>
class Atom {
public:
    using Update = std::function<void(T &value)>;

    const T get() { return value; }

    void set(const T &value_) {
        value = value_;
        for (const auto &sub : subs) {
            sub(value);
        }
    }

    void subscribe(const Update update) {
        subs.push_back(update);
    }

private:
    T value;

    std::vector<const Update> subs;
};

struct State {
    int count = 0;
};

void printState(const State &state) {
    std::cout << "State:" << std::endl;
    std::cout << "count: " << state.count << std::endl;
}

Atom<State> db;

int main(int argc, char *argv[]) {

    auto state = db.get();

    state.count++;

    std::cout << "Old: " << std::endl;
    printState(db.get());
    std::cout << std::endl;

    std::cout << "New: " << std::endl;
    printState(state);
    std::cout << std::endl;

    db.subscribe([](State& newState) {
        std::cout << "Update!" << std::endl;

        printState(newState);
    });

    db.set(state);

}
