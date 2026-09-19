/*
 * Arin32 - Modern OpenGL Graphical User Interface Library
 *
 * Copyright (c) 2026, Arin32 & ArinOS Contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARIN32_METRICS_HPP
#define ARIN32_METRICS_HPP

#include "types.hpp"

namespace arin {

/**
 * @brief Central repository for default widget dimensions and behavior limits.
 *
 * Every value previously written as a floating point literal inside widget
 * constructors or render paths now has a named home here. Widgets and styles
 * reference these constants so defaults stay consistent and discoverable.
 */
struct UiMetrics {
    // --- Default widget extents (pixels) ---------------------------------
    static constexpr Vec2 kDefaultButtonSize{85.0f, 32.0f};
    static constexpr Vec2 kDefaultCheckBoxSize{160.0f, 20.0f};
    static constexpr Vec2 kDefaultTextInputSize{200.0f, 32.0f};
    static constexpr Vec2 kDefaultProgressBarSize{260.0f, 20.0f};
    static constexpr Vec2 kDefaultListBoxSize{220.0f, 180.0f};
    static constexpr Vec2 kDefaultImageSize{100.0f, 100.0f};
    static constexpr float kDefaultIconSize = 16.0f;
    static constexpr float kDefaultImageCornerRadius = 5.0f;

    // --- Shared layout behavior ------------------------------------------
    static constexpr float kDefaultSpacing = 8.0f;
    static constexpr float kDefaultLayoutSpacing = 8.0f;
    static constexpr float kValidationTolerance = 0.5f;
    static constexpr float kMinimumItemHeight = 16.0f;
    static constexpr float kMinimumRangeSpan = 0.0001f;

    // --- Text rendering ---------------------------------------------------
    static constexpr float kApproximateLineHeight = 12.0f;
    static constexpr float kListItemTextBaselineAdjust = 1.0f;
    static constexpr float kButtonTextScale = 1.0f;
    static constexpr float kMenuLabelScale = 0.88f;
    static constexpr float kMenuShortcutScale = 0.80f;
    static constexpr float kSmallLabelScale = 0.92f;
    static constexpr float kListLabelScale = 0.95f;

    // --- Menu geometry (pixels) -------------------------------------------
    static constexpr float kMenuScreenMargin = 4.0f;
    static constexpr float kMenuSeparatorInset = 8.0f;
    static constexpr float kMenuSeparatorThickness = 1.0f;
    static constexpr float kMenuItemCornerRadius = 3.5f;
    static constexpr float kMenuIconSize = 14.0f;
    static constexpr float kMenuIconColumnWidth = 26.0f;
    static constexpr float kMenuTextInsetNoIcon = 8.0f;
    static constexpr float kMenuIconInset = 6.0f;
    static constexpr float kMenuRowTextTopOffset = 6.0f;
    static constexpr float kMenuShortcutTrailingInset = 8.0f;

    // --- List geometry (pixels) -------------------------------------------
    static constexpr float kListRowTextInset = 8.0f;
    static constexpr float kListCheckBoxSize = 16.0f;
    static constexpr float kListCheckBoxInset = 6.0f;
    static constexpr float kListCheckBoxCorner = 3.0f;
    static constexpr float kListCheckMarkWidth = 1.8f;
    static constexpr float kListScrollTrackWidth = 7.0f;
    static constexpr float kListTrackCorner = 3.5f;

    // --- Input behavior ---------------------------------------------------
    static constexpr float kOffscreenCoordinate = -99999.0f;
    static constexpr float kMaxFrameDeltaTime = 0.1f;
    static constexpr float kProgressPhaseWrap = 10000.0f;
};

/**
 * @brief Text alignment choices for rectangle-based text layout helpers.
 */
enum class TextAlignH : uint8_t {
    Left,   ///< Align text against the left edge of the rectangle.
    Center, ///< Center text horizontally inside the rectangle.
    Right   ///< Align text against the right edge of the rectangle.
};

/**
 * @brief Vertical text alignment choices for rectangle-based text layout helpers.
 */
enum class TextAlignV : uint8_t {
    Top,    ///< Align text against the top edge of the rectangle.
    Center, ///< Center text vertically using optical font metrics.
    Bottom  ///< Align text against the bottom edge of the rectangle.
};

} // namespace arin

#endif // ARIN32_METRICS_HPP
