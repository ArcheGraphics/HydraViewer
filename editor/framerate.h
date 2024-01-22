//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <chrono>

#include <vector>

namespace vox {

class Framerate {

public:
    using Clock = std::chrono::steady_clock;
    using Timepoint = Clock::time_point;

private:
    std::vector<double> _durations;
    std::vector<size_t> _frames;
    Timepoint _last;
    size_t _history_size;

public:
    explicit Framerate(size_t n = 5) noexcept;
    void clear() noexcept;
    void record(size_t frame_count = 1u) noexcept;
    [[nodiscard]] double duration() const noexcept;
    [[nodiscard]] double report() const noexcept;
};

}// namespace vox
