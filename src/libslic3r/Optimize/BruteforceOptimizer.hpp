#ifndef BRUTEFORCEOPTIMIZER_HPP
#define BRUTEFORCEOPTIMIZER_HPP

#include <libslic3r/Optimize/NLoptOptimizer.hpp>

namespace Slic3r { namespace opt {

namespace detail {
// Implementing a bruteforce optimizer

template<size_t N>
constexpr long num_iter(const std::array<size_t, N> &idx, size_t gridsz)
{
    long ret = 0;
    for (size_t i = 0; i < N; ++i) ret += idx[i] * std::pow(gridsz, i);
    return ret;
}

struct AlgBurteForce {
    bool to_min;
    StopCriteria stc;
    size_t gridsz;

    AlgBurteForce(const StopCriteria &cr, size_t gs): stc{cr}, gridsz{gs} {}

    template<int D, size_t N, class Fn, class Cmp>
    void run(std::array<size_t, N> &idx,
             Result<N> &result,
             const Bounds<N> &bounds,
             Fn &&fn,
             Cmp &&cmp)
    {
        if (stc.stop_condition()) return;

        if constexpr (D < 0) {
            Input<N> inp;

            auto max_iter = stc.max_iterations();
            if (max_iter && num_iter(idx, gridsz) >= max_iter) return;

            for (size_t d = 0; d < N; ++d) {
                const Bound &b = bounds[d];
                double step = (b.max() - b.min()) / (gridsz - 1);
                inp[d] = b.min() + idx[d] * step;
            }

            auto score = fn(inp);
            if (cmp(score, result.score)) {
                result.score = score;
                result.optimum = inp;
            }

        } else {
            for (size_t i = 0; i < gridsz; ++i) {
                idx[D] = i;
                run<D - 1>(idx, result, bounds, std::forward<Fn>(fn),
                           std::forward<Cmp>(cmp));
            }
        }
    }

    template<class Fn, size_t N>
    Result<N> optimize(Fn&& fn,
                       const Input<N> &/*initvals*/,
                       const Bounds<N>& bounds)
    {
        std::array<size_t, N> idx = {};
        Result<N> result;

        if (to_min) {
            result.score = std::numeric_limits<double>::max();
            run<int(N) - 1>(idx, result, bounds, std::forward<Fn>(fn),
                            std::less<double>{});
        }
        else {
            result.score = std::numeric_limits<double>::lowest();
            run<int(N) - 1>(idx, result, bounds, std::forward<Fn>(fn),
                            std::greater<double>{});
        }

        return result;
    }
};

} // namespace bruteforce_detail

using AlgBruteForce = detail::AlgBurteForce;

template<>
class Optimizer<AlgBruteForce> {
    AlgBruteForce m_alg;

public:

    Optimizer(const StopCriteria &cr = {}, size_t gridsz = 100)
        : m_alg{cr, gridsz}
    {}

    Optimizer& to_max() { m_alg.to_min = false; return *this; }
    Optimizer& to_min() { m_alg.to_min = true;  return *this; }

    template<class Func, size_t N>
    Result<N> optimize(Func&& func,
                       const Input<N> &initvals,
                       const Bounds<N>& bounds)
    {
        return m_alg.optimize(std::forward<Func>(func), initvals, bounds);
    }

    Optimizer &set_criteria(const StopCriteria &cr)
    {
        m_alg.stc = cr; return *this;
    }

    const StopCriteria &get_criteria() const { return m_alg.stc; }
};

}} // namespace Slic3r::opt

#endif // BRUTEFORCEOPTIMIZER_HPP