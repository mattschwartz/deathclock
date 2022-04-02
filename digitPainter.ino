namespace DeathclockDigit
{
    struct DrawableDigit
    {
        bool topHbar;
        bool midHbar;
        bool botHbar;

        bool topLeftVbar;
        bool botLeftVbar;

        bool topRightVbar;
        bool botRightVbar;
    };

    struct DigitSize
    {
        // The long-end of a digit bar
        uint8_t barLength = 20;
        // The small-end of a digit bar
        uint8_t barWidth = 6;
        // The space between digit bars
        uint8_t lanePadding = 3;
        // The total width of a lane, including long-end of digit bar including padding
        uint8_t laneWidth = barWidth + lanePadding;
        // Entire width of a single digit
        uint8_t digitWidth = laneWidth * 2 + barLength + lanePadding * 2;
        // Entire height of a single digit
        uint8_t digitHeight = laneWidth * 3 + (barLength + lanePadding) * 2;
    };

    enum class ModeSelection
    {
        HOURS,
        DAYS,
    };

    // Digit lookup
    const DrawableDigit digits[12] = {
        DrawableDigit{true, false, true, true, true, true, true},       // 0
        DrawableDigit{false, false, false, false, false, true, true},   // 1
        DrawableDigit{true, true, true, false, true, true, false},      // 2
        DrawableDigit{true, true, true, false, false, true, true},      // 3
        DrawableDigit{false, true, false, true, false, true, true},     // 4
        DrawableDigit{true, true, true, true, false, false, true},      // 5
        DrawableDigit{true, true, true, true, true, false, true},       // 6
        DrawableDigit{true, false, false, false, false, true, true},    // 7
        DrawableDigit{true, true, true, true, true, true, true},        // 8
        DrawableDigit{true, true, true, true, false, true, true},       // 9
        DrawableDigit{false, false, false, false, false, false, false}, // blank
        DrawableDigit{false, true, false, false, false, false, false}   // - (hypen)
    };
}