#include <concepts>
#include <iostream>
#include <cstdint>
#include <functional>
#include <memory>

template<typename>
class MyFunc;

template<typename Ret, typename... Args>
class MyFunc<Ret(Args...)> {
private:
    class ICallable {
    public:
        virtual ~ICallable() = default;
        virtual Ret Invoke(Args...) = 0;
        virtual Ret Invoke(Args&&...) = 0;
    };

    template<typename T>
    class Callable : public ICallable {
    public:
        Callable(const T& t) : t_(t) {
        }

        Callable(T&& t) : t_(std::move(t)) {
        }

        Ret Invoke(Args... args) override {
            return t_(args...);
        }

        Ret Invoke(Args&&... args) override {
            return t_(std::move(args)...);
        }

    private:
        T t_;
    };

public:
    MyFunc() = default;

    template<typename T>
    MyFunc(T&& t) {
        callable_ = std::make_shared<Callable<std::decay_t<T>>>(std::forward<T>(t));
    }

    template<typename T>
    MyFunc& operator =(T&& t) {
        callable_ = std::make_shared<Callable<std::decay_t<T>>>(std::forward<T>(t));
        return *this;
    }

    Ret operator ()(Args... args) const {
        return callable_->Invoke(args...);
    }

private:
    std::shared_ptr<ICallable> callable_;
};

template <class T>
class Spy {
private:
    using LoggerType = MyFunc<void(unsigned int)>;

    T obj_;
    LoggerType logger_ = [](unsigned int) {};

    struct Counter {
        uint32_t active_count = 0;
        uint32_t total_count = 0;

        Counter() = default;

        Counter(const Counter&) : Counter() {}

        Counter& operator =(const Counter&) {
            active_count = 0;
            total_count = 0;
            return *this;
        }

        Counter(Counter&&) : Counter() {}

        Counter& operator =(Counter&&) {
            active_count = 0;
            total_count = 0;
            return *this;
        }

        ~Counter() = default;
    };

    Counter counter_;

    template<typename U>
    class Proxy {
    private:
        Spy<U>* spy_;

    public:
        Proxy(Spy<U>* spy) : spy_(spy) {
            ++(spy_->counter_).active_count;
            ++(spy_->counter_).total_count;
        }

        ~Proxy() {
            --(spy_->counter_).active_count;
            if ((spy_->counter_).active_count == 0) {
                spy_->logger_(spy_->counter_.total_count);
                (spy_->counter_).total_count = 0;
            }
        }

        U* operator ->() {
            return &(spy_->obj_);
        }
    };

public:
    Spy() = default;

    explicit Spy(const T& obj) : obj_(obj) {
    }

    explicit Spy(T&& obj) : obj_(std::move(obj))  {
    }

    bool operator == (const Spy<T>& other) const requires std::equality_comparable<T> {
        return obj_ == other.obj_;
    }

    T& operator *() {
        return obj_;
    }

    const T& operator *() const {
        return obj_;
    }

    Proxy<T> operator ->() {
        return Proxy<T>(this);
    }

    template <std::invocable<unsigned int> Logger>
        requires (!std::copyable<T> || std::copyable<Logger>) && (!std::movable<T> || std::movable<Logger>)
    void setLogger(Logger&& logger) {
        logger_ = std::forward<Logger>(logger);
    }

    template <std::invocable<unsigned int> Logger>
        requires (!std::copyable<T> || std::copyable<Logger>) && (!std::movable<T> || std::movable<Logger>)
    void setLogger(Logger& logger) {
        logger_ = logger;
    }
};
