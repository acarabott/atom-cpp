#include <iostream>

struct State {
    int count = 0;
};

void printState(const State& state)
{
    std::cout << "State:" << std::endl;
    std::cout << "count: " << state.count <<  std::endl;
}

int main(int argc, char *argv[]) {

    State state;

    printState(state);
}
