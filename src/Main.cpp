#include <iostream>
#include <vector>

#include "Atom.h"
#include "Cursor.h"
#include "History.h"

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
    std::cout << "{ count: << " << state.count << ", "
              << "name: " << state.name << ", "
              << "sub: { value: " << state.sub.value << " } }" << std::endl;
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    // create atom
    Atom<State> db;
    History history(db);

    // copy current state
    auto first = db.get();

    // add subscription
    db.subscribe([](const State &previousState, const State &state) {
        std::cout << "Subscription!" << std::endl;

        std::cout << "old: ";
        printState(previousState);

        std::cout << "new: ";
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

    // print original and current state
    std::cout << "first: ";
    printState(first);
    std::cout << "now  : ";
    printState(db.get());
    std::cout << std::endl;


    // create name cursor with macro
    auto nameCursor = DEF_CURSOR(db, name);
    nameCursor.subscribe([](const std::string &previous, const std::string &name) {
        (void) previous;

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
    valueCursor.subscribe([](auto &previous, auto &value) {
        (void) previous;
        std::cout << "sub value changed: " << value << std::endl;
    });

    // update sub
    valueCursor.set(3.33f);

    // create cursor with macro
    auto vc = DEF_CURSOR(db, sub.value);
    std::cout << "vc: " << vc.get() << std::endl;


    // history

    std::cout << std::endl << "history size: " << history.size() << std::endl;

    for (int i = 0; i < history.size(); ++i) {
        const auto istr = std::to_string(i);
        std::cout << std::string(2 - istr.length(), ' ').append(istr) << ": ";
        printState(history.get(i));
    }

    std::cout << std::endl << "restore from history" << std::endl;
    history.set(0);

}
