#include <iostream>

template<class T>
class Atom {
public:
    const T get() { return value; }

    void set(const T &value_) { value = value_; }

private:
    T value;
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

}
