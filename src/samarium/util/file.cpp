/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022 Jai Bellare
 * See <https://opensource.org/licenses/MIT/> or LICENSE.md
 * Project homepage: https://github.com/strangeQuark1041/samarium
 */

#include <array>      // for to_array, array
#include <cstring>    // for memcpy
#include <filesystem> // for path
#include <fstream>    // for ifstream, ofstream, basic_ostream::write
#include <iterator>   // for ifstreambuf_iterator
#include <string>     // for string

#include "fpng/fpng.hpp"
#include "range/v3/algorithm/copy.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

#include "samarium/core/types.hpp"     // for u8
#include "samarium/graphics/Color.hpp" // for BGR_t, bgr
#include "samarium/graphics/Image.hpp" // for Image
#include "samarium/math/Extents.hpp"   // for range
#include "samarium/math/Vector2.hpp"   // for Dimensions

#include "samarium/util/print.hpp" // for Dimensions

#include "file.hpp"

namespace sm::file
{
auto read(Text, const std::filesystem::path& file_path) -> ExpectedFile<std::string>
{
    if (std::filesystem::exists(file_path) && std::filesystem::is_regular_file(file_path))
    {
        auto ifs = std::ifstream{file_path};
        return {std::string(std::istreambuf_iterator<char>{ifs}, {})};
    }
    else { return tl::unexpected<FileError>{{}}; }
}

auto read(const std::filesystem::path& file_path) -> ExpectedFile<std::string>
{
    return read(Text{}, file_path);
}

auto read_image(const std::filesystem::path& file_path) -> ExpectedFile<Image>
{
    if (!std::filesystem::exists(file_path) || !std::filesystem::is_regular_file(file_path))
    {
        return tl::unexpected<FileError>{{}};
    }

    auto width         = 0;
    auto height        = 0;
    auto channel_count = 0;

    const auto data = stbi_load(file_path.string().c_str(), &width, &height, &channel_count, 0);

    if (!data) { return tl::unexpected<FileError>{{}}; }

    auto image = Image{{static_cast<u64>(width), static_cast<u64>(height)}};

    if (channel_count == 4) { std::memcpy(&image.front(), data, image.size() * 4); }
    else if (channel_count == 3)
    {
        for (auto i : range(image.size()))
        {
            image[i].r = data[i * 3];
            image[i].g = data[i * 3 + 1];
            image[i].b = data[i * 3 + 2];
        }
    }
    else if (channel_count == 2)
    {
        for (auto i : range(image.size()))
        {
            const auto value = data[i * 2];
            image[i].r       = value;
            image[i].g       = value;
            image[i].b       = value;
            image[i].a       = data[i * 2 + 1];
        }
    }
    else if (channel_count == 1)
    {
        for (auto i : range(image.size()))
        {
            const auto value = data[i];
            image[i].r       = value;
            image[i].g       = value;
            image[i].b       = value;
        }
    }
    else { tl::unexpected<FileError>{{}}; }

    return image;
}

void write(Targa, const Image& image, const std::filesystem::path& file_path)
{
    const auto tga_header = std::to_array<u8>(
        {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, static_cast<u8>(255 & image.dims.x),
         static_cast<u8>(255 & (image.dims.x >> 8)), static_cast<u8>(255 & image.dims.y),
         static_cast<u8>(255 & (image.dims.y >> 8)), 24, 32});

    const auto data = image.formatted_data(sm::bgr);

    std::ofstream(file_path, std::ios::binary)
        .write(reinterpret_cast<const char*>(&tga_header[0]), 18)
        .write(reinterpret_cast<const char*>(&data[0]),
               static_cast<std::streamsize>(data.size() * data[0].size()));
}

void write(Bmp, const Image& image, const std::filesystem::path& file_path)
{
    stbi_write_bmp(file_path.string().c_str(), static_cast<i32>(image.dims.x),
                   static_cast<i32>(image.dims.y), 4 /* RGBA */,
                   static_cast<const void*>(&image.front()));
}


auto find(const std::string& file_name, const std::filesystem::path& directory)
    -> tl::expected<std::filesystem::path, FileError>
{
    for (const auto& dir_entry : std::filesystem::recursive_directory_iterator(directory))
    {
        if (dir_entry.is_regular_file() && dir_entry.path().filename() == file_name)
        {
            return {std::filesystem::canonical(dir_entry)};
        }
    }

    return tl::unexpected<FileError>{{}};
}

auto find(const std::string& file_name, std::span<std::filesystem::path> search_paths)
    -> tl::expected<std::filesystem::path, FileError>
{
    for (const auto& path : search_paths)
    {
        if (std::filesystem::is_directory(path))
        {
            if (const auto found_path = find(file_name, path)) { return {found_path}; }
        }
        else if (std::filesystem::is_regular_file(path))
        {
            if (std::filesystem::exists(path)) { return {std::filesystem::canonical(path)}; }
        }
        else if (!std::filesystem::exists(path)) { continue; }
    }

    return tl::unexpected<FileError>{{}};
}

auto find(const std::string& file_name, std::initializer_list<std::filesystem::path> search_paths)
    -> tl::expected<std::filesystem::path, FileError>
{
    for (const auto& path : search_paths)
    {
        if (std::filesystem::is_directory(path))
        {
            if (const auto found_path = find(file_name, path)) { return {found_path}; }
        }
        else if (std::filesystem::is_regular_file(path))
        {
            if (std::filesystem::exists(path)) { return {std::filesystem::canonical(path)}; }
        }
        else if (!std::filesystem::exists(path)) { continue; }
    }

    return tl::unexpected<FileError>{{}};
}
} // namespace sm::file
