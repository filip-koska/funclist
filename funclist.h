#ifndef FUNCLIST_H
#define FUNCLIST_H

#include <ranges>
#include <functional>
#include <sstream>

namespace flist {

    auto empty = [](auto, auto a) {
        return a;
    };

    // append x to the beginning of l
    auto cons = [](auto x, auto l) {
        return [=](auto f, auto a) {return f(x, l(f, a));};
    };

namespace detail {

    // base of template parameter pack recursion
    inline auto _create() {
        return empty;
    }

    // at least one argument for create left
    template <typename T, typename... Args>
    inline auto _create(T first_arg, Args... args) {
        return [=](auto f, auto a) {
            return f(first_arg, _create(args...)(f, a)); 
        };
    }

    // helper function to recursively create the list from elements of r
    inline auto _of_range(auto const& r, auto it, auto f, auto a) {
        if (it == std::ranges::end(r))
            return a;
        return f(*it, _of_range(r, std::next(it), f, a));
    }
    
} // namespace detail

    // create list from given arguments
    auto create = [](auto... args) {
        return detail::_create(args...);
    };

    // create list from elements of given range
    auto of_range = [](auto r) {
        return [=](auto f, auto a) {
            if constexpr (std::ranges::bidirectional_range<decltype(r)>)
                return detail::_of_range(r, std::ranges::begin(r), f, a);
            else // extract reference hidden inside std::reference_wrapper
                return detail::_of_range(r.get(), std::ranges::begin(r.get()), f, a);
        };
    };

    // concatenate two lists
    auto concat = [](auto l, auto r) {
        return [=](auto f, auto a) {return l(f, r(f, a));};
    };

    
    // transform each element xi of l into m(xi)
    auto map = [](auto m, auto l) {
        return [=](auto f, auto a) {
            auto g = [=](auto x, auto y) {return f(m(x), y);};
            return l(g, a);
        };
    };

    // discard elements xi of l that do not satisfy p(xi)
    auto filter = [](auto p, auto l) {
        return [=](auto f, auto a) {
            auto g = [=](auto x, auto y) {
                if (p(x))
                    return f(x, y);
                return y;
            };
            return l(g, a);
        };
    };

    // reverse l
    auto rev = [](auto l) {
        return [=](auto f, auto a) {
            using A = decltype(a);
            using F = decltype(f);
            auto g = [=](auto x, auto y) {return std::function<A(F, A)>(concat(y, cons(x, empty)));};
            return l(g, std::function<A(F, A)>(empty))(f, a);
        };
    };

    // create a list that is the concatenation of all elements of l
    auto flatten = [](auto l) {
        return [=](auto f, auto a) {
            auto g = [=](auto x, auto y) {
                return x(f, y);
            };
            return l(g, a);
        };
    };

    auto as_string = [](auto const& l) -> std::string {
        std::ostringstream oss;
        oss << "[";
        bool first = true;
        rev(l)([&](auto x, auto a) {
            if (!first)
                oss << ';';
            first = false;
            oss << x;
            return a;
        }, 0);
        oss << ']';
        return oss.str();
    };

} // namespace flist



#endif