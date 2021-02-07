# Atom C++

A very basic and incomplete implementation of the Atom/Cursor state model found
in [thi.ng/atom TypeScript package](https://github.com/thi-ng/umbrella/tree/develop/packages/atom) (which is in turn
based on [Clojure's Atom](https://clojuredocs.org/clojure.core/atom)).

This is just to illustrate the concept, and some of the challenges of implementing something similar in C++.

The general idea is to have your application state stored in a single place, and to use a wrapper for access and
updates. When accessing state with `get`, the returned values are immutable, the only way to modify your application
state is to use the `set` or `update` methods.

`Atom`s can be subscribed to with a function that will be called whenever changes are occur.

`Cursor`s provide read/write/subscription to a single value of the state.

`History` is used to keep track of all state changes, and restore previous states.

## Authors

- Arthur Carabott
