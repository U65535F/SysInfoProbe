#pragma once
#include <exception>
#include <functional>

#define DEFER_CONCAT_IMPL(x, y) x##y
#define DEFER_CONCAT(x, y) DEFER_CONCAT_IMPL(x, y)

#define DEFER auto DEFER_CONCAT(defer_, __COUNTER__) = MakeDeferObj{} += [&]()
#define DEFER_ON_FAILURE auto DEFER_CONCAT(deferFailure_, __COUNTER__) = MakeDeferObj{DeferMode::OnFailure} += [&]()
#define DEFER_ON_SUCCESS auto DEFER_CONCAT(deferSuccess_, __COUNTER__) = MakeDeferObj{DeferMode::OnSuccess} += [&]()

enum class DeferMode {
    Always,
    OnFailure,
    OnSuccess
};

template<typename F> class Defer {
public:

    template <typename F>
    Defer(DeferMode mode, F&& fn) : _mode(std::move(mode)), _fn(std::forward<F>(fn)), _active(true) {}

    ~Defer() {
        if (!_active)  return;

        bool bException = std::uncaught_exceptions() > 0;

        if ((_mode == DeferMode::Always) ||
            (_mode == DeferMode::OnFailure && bException) ||
            (_mode == DeferMode::OnSuccess && !bException)) {
            _fn();
        }
    }

    Defer(Defer&& other) noexcept : _mode(std::move(other._mode)), _fn(std::move(other._fn)), _active(other._active) {
        other._active = false;
    }

    Defer(const Defer&) = delete;
    Defer& operator=(const Defer&) = delete;

private:
    DeferMode _mode;
    std::function<void()> _fn;
    bool _active;
};

struct MakeDeferObj {
    DeferMode mode;
    MakeDeferObj(DeferMode m = DeferMode::Always) : mode(m) {}

    template<typename F> Defer<F> operator+=(F&& fn) {
        return Defer<F>(mode, std::forward<F>(fn));
    }
};