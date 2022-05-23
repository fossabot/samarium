/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Jai Bellare
 * See <https://opensource.org/licenses/MIT/> or LICENSE.md
 * Project homepage: https://github.com/strangeQuark1041/samarium
 */

#include "samarium/samarium.hpp"
// #include <ranges>

using namespace sm;
using namespace sm::literals;

using IntegerPair = Vector2_t<i32>;
using Path        = std::vector<Vector2>;

auto point_at(i32 index, i32 level)
{
    static constexpr auto points = std::to_array<IntegerPair>({{0, 0}, {0, 1}, {1, 1}, {1, 0}});

    i32 thingy = index & 3;
    auto v     = points[static_cast<u64>(thingy)];

    for (auto j : range(1, level))
    {
        index >>= 2;
        thingy = index & 3;

        const auto len = static_cast<i32>(std::pow(2, j));

        if (thingy == 0) { std::swap(v.x, v.y); }
        else if (thingy == 1)
        {
            v.y += len;
        }
        else if (thingy == 2)
        {
            v += IntegerPair::combine(len);
        }
        else
        {
            const auto temp = len - 1 - v.x;
            v.x             = len - 1 - v.y;
            v.y             = temp;
            v.x += len;
        }
    }

    return v;
}

auto point_count(u64 level) { return static_cast<u64>(std::pow(2.0, 2.0 * level)); }

auto points(u64 level)
{
    auto vec = std::vector<IntegerPair>(point_count(level));
    for (auto i : range(vec.size())) { vec[i] = point_at(i, level); }
    return vec;
}

auto points_f64(u64 level)
{
    auto vec = Path(point_count(level));
    for (auto i : range(vec.size()))
    {
        vec[i] = point_at(i, level).as<f64>() / std::pow(2.0, static_cast<f64>(level)) +
                 Vector2::combine(0.5 / std::pow(2.0, level));
    }
    return vec;
}

auto rescale(const Path& input, u64 factor)
{
    if (factor == 1UL) { return input; }

    auto output = Path();
    output.reserve(input.size() * factor);
    const auto extra_size = input.size() * (factor - 1UL);

    for (auto i : range(input.size() - 1UL))
    {
        auto count = extra_size / (input.size() - 1);
        if (i < extra_size % (input.size() - 1)) { count++; }

        output.push_back(input[i]);

        for (auto j : range(1, count + 1UL))
        {
            const auto segment = input[i + 1] - input[i];
            const auto adder   = segment / static_cast<f64>(count + 1);
            output.push_back(input[i] + static_cast<f64>(j) * adder);
        }
    }
    output.push_back(input.back());
    return output;
}

auto levels_till(u64 order)
{
    auto levels = std::vector<Path>(order);
    for (auto i : range(order)) { levels[i] = points_f64(i + 1); }
    return levels;
}

auto rescaled_levels_till(u64 order)
{
    auto levels         = std::vector<Path>(order);
    const auto max_size = point_count(order);

    for (auto i : range(order))
    {
        const auto points = points_f64(i + 1);
        levels[i]         = rescale(points, max_size / points.size());
    }
    return levels;
}

static constexpr auto width    = 1000UL;
static constexpr auto order    = 6UL;
static constexpr auto N        = math::power<order>(2UL);
static constexpr auto total    = N * N;
static constexpr auto duration = 70UL;

i32 main()
{
    const auto levels = rescaled_levels_till(order);
    auto path         = levels[0];
    auto current_iter = 0UL;

    auto app = App{{.dims = Dimensions::combine(static_cast<u64>(width))}};

    const auto draw = [&]
    {
        app.fill({});

        if (app.frame_counter != 0 && app.frame_counter % duration == 0)
        {
            current_iter = (current_iter + 1UL) % order;
            path         = levels[current_iter];
        }
        else
        {
            const auto lerp_factor =
                static_cast<f64>(app.frame_counter % duration) / static_cast<f64>(duration);

            for (auto i : range(path.size()))
            {
                path[i] = interp::lerp(interp::smooth(lerp_factor, 3),
                                       Extents<Vector2>{levels[current_iter][i],
                                                        levels[(current_iter + 1UL) % order][i]});
            }
        }

        const auto mapper = [&](Vector2 vec) { return (vec) * static_cast<f64>(width); };
        print(0.5 / std::pow(2.0, current_iter));

        for (auto i : range(path.size() - 1))
        {
            app.draw_line_segment({mapper(path[i]), mapper(path[i + 1])}, colors::aliceblue, 1);
        }
    };

    app.transform = {{0, 0}, {1, 1}};

    app.run(draw);
}
