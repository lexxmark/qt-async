// code taken from https://github.com/charlesnicholson/scope_exit
// this code is in the public domain. see the LICENSE file for details.
// Simple implementation of Andrei Alexandrescu's scope guard, from one of his C++ talks.
#pragma once

#include <utility>

namespace detail {

template< typename Lambda > class scope_exit
{
public:
    explicit scope_exit(Lambda lambda);
    scope_exit(scope_exit&& lambda);
    ~scope_exit();
    scope_exit() = delete;
    scope_exit(scope_exit const&) = delete;
    scope_exit &operator =(scope_exit const&) = delete;
private:
    Lambda lambda_;
    bool active_;
};

template< typename Lambda > scope_exit< Lambda >::scope_exit(Lambda lambda)
    : lambda_(std::move(lambda))
    , active_(true)
{
}

template< typename Lambda > scope_exit< Lambda >::scope_exit(scope_exit&& rhs)
    : lambda_(std::move(rhs.lambda_))
    , active_(rhs.active_)
{
    rhs.active_ = false;
}

template< typename Lambda > scope_exit< Lambda >::~scope_exit()
{
    if (active_) {
        lambda_();
    }
}

template< typename Lambda > scope_exit< Lambda > make_scope_exit(Lambda l)
{
    return scope_exit< Lambda >(std::move(l));
}

enum class scope_exit_placeholder {};

template< typename Lambda > scope_exit< Lambda > operator +(scope_exit_placeholder, Lambda&& l)
{
    return make_scope_exit< Lambda >(std::forward< Lambda >(l));
}

}

#define SCOPE_EXIT_CONCAT_INNER(x, y) x##y
#define SCOPE_EXIT_CONCAT(x, y) SCOPE_EXIT_CONCAT_INNER(x, y)

#define SCOPE_EXIT \
    auto SCOPE_EXIT_CONCAT(scope_exit_, __LINE__) = ::detail::scope_exit_placeholder() + [&]()

