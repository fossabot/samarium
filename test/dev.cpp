/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Jai Bellare
 * See <https://opensource.org/licenses/MIT/> or LICENSE.md
 * Project homepage: https://github.com/strangeQuark1041/samarium
 */

#include "samarium/gl/gl.hpp"
#include "samarium/math/Vector2.hpp"
#include "samarium/samarium.hpp"

using namespace sm;
using namespace sm::literals;

int main()
{
    print(gl::version_major, gl::version_minor);
    auto window = Window{{1024, 1024}};
    auto watch  = Stopwatch{};

    window.keymap.push_back(keyboard::OnKeyPress(*window.handle, {Key::W}, [] { print("Key"); }));

    while (window.is_open())
    {
        draw::fill("#1c151b"_c);
        draw::circle(window, {{0.2, 0.3}, 0.4}, {.fill_color = "#fa2844"_c});
        watch.reset();

        if (Window::resized) { print("Resized"); }
        if (window.mouse.left) { print("Left"); }
        if (window.mouse.right) { print("Right"); }
        if (window.is_key_pressed(Key::Space)) { print("Space"); }
        window.display();
    }
}
