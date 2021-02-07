#pragma once

template<typename T>
class Atomic {
public:
    using Subscription = std::function<void(const T &previousValue, const T &value)>;

    using Update = std::function<void(T &value)>;

    Atomic() {};

    ~Atomic() {};

    virtual const T &get() const = 0;

    virtual void set(const T &value_) = 0;

    void update(const Update &update) {
        assert(update != nullptr);
        auto value = get();
        update(value);
        set(value);
    }

    void subscribe(const Subscription &update) {
        subscriptions.push_back(update);
    }

    void notifySubscriptions(const T &previousValue, const T &value) {
        for (const auto &sub : subscriptions) {
            sub(previousValue, value);
        }
    }

protected:
    std::vector<const Subscription> subscriptions;
};
