#include <iostream>

struct State {
    int count = 0;
};

void printState(const State& state)
{
    std::cout << "State:" << std::endl;
    std::cout << "count: " << state.count <<  std::endl;
}

class Atom
{
public:
    const State get() { return state; }
    void set(const State& state_) { state = state_; }
private:
    State state;
};

Atom db;
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
