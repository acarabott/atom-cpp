#include <iostream>
#include <vector>

#include "Atom.h"
#include "Cursor.h"

struct SubState {
    float value = 0.5f;
};

struct State {
    int count = 0;
    std::string name = "";

    SubState sub;
};

void increment(Atom<State> &db) {
    db.update([](auto &state) {
        state.count++;
    });
}

void decrement(Atom<State> &db) {
    db.update([](auto &state) {
        state.count--;
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
