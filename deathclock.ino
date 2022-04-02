#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <RTClib.h>

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8

const DateTime birthdate = DateTime(2000, 6, 11, 16, 24, 0);
const DateTime deathdate = DateTime(2031, 6, 11, 16, 24, 0); // big 4-0
// Needed cuz i'm old (9 years before Y2000)
const auto nineYears = TimeSpan(3285, 0, 0, 0);

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170

// Bright, but pleasant, cyan color
#define DIGIT_COLOR 0x07f9
// Dim, but still pleasant, cyan color
#define DIGIT_SHADOW_COLOR 0x0022

// Types

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

const DigitSize &digitSize = DigitSize();

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

ModeSelection selectedMode = ModeSelection::HOURS;
RTC_DS3231 rtc;
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void refreshTft()
{
    DateTime now = rtc.now();
    const uint32_t remainingTimeSeconds = deathdate.unixtime() - now.unixtime();
    const TimeSpan &remainingTimeSpan = TimeSpan(remainingTimeSeconds);
    const uint32_t hoursRemaining = ((uint32_t)remainingTimeSpan.days() * 24) + (uint32_t)remainingTimeSpan.hours();

    uint32_t startTime = millis();
    switch (selectedMode)
    {
    case ModeSelection::HOURS:
        drawDigits(hoursRemaining);
        break;

    case ModeSelection::DAYS:
        drawDigits(remainingTimeSpan.days());
        break;
    }
    uint32_t stopTime = millis();
    Serial.print("Digit draw took ");
    Serial.print(stopTime - startTime);
    Serial.println(" ms");

    const float currentAgeSeconds = (now.unixtime() - birthdate.unixtime()) + nineYears.totalseconds();
    const float deathDateSeconds = deathdate.unixtime() - birthdate.unixtime() + nineYears.totalseconds();

    startTime = millis();
    drawProgressBar(1.0f - (currentAgeSeconds / deathDateSeconds));
    stopTime = millis();
    Serial.print("Progbar draw took ");
    Serial.print(stopTime - startTime);
    Serial.println(" ms");
}

void drawProgressBar(const float percent)
{
    Serial.print("Percentage: ");
    Serial.println(percent);

    const uint16_t startX = digitSize.lanePadding;
    const uint16_t startY = SCREEN_HEIGHT - digitSize.lanePadding - digitSize.barWidth;

    const float maxProgBarLength = SCREEN_WIDTH - digitSize.lanePadding * 2;
    tft.fillRect(startX - 1, startY - 1, maxProgBarLength + 2, digitSize.barWidth + 2, DIGIT_SHADOW_COLOR);

    const float progBarLength = percent * (maxProgBarLength - digitSize.lanePadding);
    tft.fillRect(startX, startY, startX + progBarLength, digitSize.barWidth - 1, DIGIT_COLOR);

    tft.setCursor(progBarLength, startY - 10);
    tft.setTextSize(1);
    tft.setTextColor(DIGIT_COLOR);
    tft.print(percent * 100);
    tft.print("% remaining life");
}

const uint16_t digitYOffs = (SCREEN_HEIGHT - digitSize.digitHeight) / 2;
const uint8_t maxNumDigits = 5;

void drawDigits(const uint32_t num)
{
    const String numStr = String(num);

    for (int i = maxNumDigits; i >= 0; --i)
    {
        int index;
        if (i < numStr.length())
        {
            index = numStr[numStr.length() - 1 - i] - '0';
        }
        else if (i == numStr.length())
        {
            // Draw a negative-sign. Really make the number sink in
            index = 11;
        }
        else
        {
            // Draw a blank digit for that good ole classic digital clock feel
            index = 10;
        }

        const uint16_t startX = (digitSize.digitWidth + digitSize.lanePadding) * (maxNumDigits - i);
        const uint16_t startY = digitYOffs;

        paintDigit(digits[index], startX, startY);
    }

    // Render mode selection

    // hours mode
    tft.setCursor(
        SCREEN_WIDTH - 34,
        digitYOffs + digitSize.digitHeight - digitSize.barLength - digitSize.lanePadding * 2);
    tft.setTextSize(1);

    if (selectedMode == ModeSelection::HOURS)
    {
        tft.setTextColor(DIGIT_COLOR);
    }
    else
    {
        tft.setTextColor(DIGIT_SHADOW_COLOR);
    }
    tft.print("hours");

    // days mode
    tft.setCursor(
        SCREEN_WIDTH - 30,
        digitYOffs + digitSize.digitHeight - digitSize.barLength + digitSize.lanePadding * 2);

    if (selectedMode == ModeSelection::DAYS)
    {
        tft.setTextColor(DIGIT_COLOR);
    }
    else
    {
        tft.setTextColor(DIGIT_SHADOW_COLOR);
    }
    tft.print("days");
}

void paintHBar(uint16_t startX, uint16_t startY, uint16_t color)
{
    tft.fillTriangle(
        startX - digitSize.lanePadding, startY + digitSize.lanePadding,
        startX, startY,
        startX, startY + digitSize.barWidth - 1, color);                // left tri
    tft.fillRect(startX, startY, digitSize.barLength, digitSize.barWidth, color); //  vbar
    tft.fillTriangle(
        startX + digitSize.barLength + digitSize.lanePadding, startY + digitSize.lanePadding,
        startX + digitSize.barLength, startY,
        startX + digitSize.barLength, startY + digitSize.barWidth - 1, color); // right triangle
}

void paintVBar(uint16_t startX, uint16_t startY, uint16_t color)
{
    tft.fillTriangle(
        startX + digitSize.barWidth / 2, startY - digitSize.lanePadding,
        startX, startY,
        startX + digitSize.barWidth - 1, startY, color);                // top tri
    tft.fillRect(startX, startY, digitSize.barWidth, digitSize.barLength, color); // hbar
    tft.fillTriangle(
        startX + digitSize.barWidth / 2, startY + digitSize.barLength + digitSize.lanePadding,
        startX, startY + digitSize.barLength,
        startX + digitSize.barWidth - 1, startY, color); // bot triangle
}

void paintDigit(const DrawableDigit digit, uint16_t startX, uint16_t startY)
{
    if (digit.topHbar)
    {
        paintHBar(startX + digitSize.laneWidth, startY, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + digitSize.laneWidth, startY, DIGIT_SHADOW_COLOR);
    }
    if (digit.midHbar)
    {
        paintHBar(startX + digitSize.laneWidth, startY + digitSize.barLength + digitSize.laneWidth + 1, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + digitSize.laneWidth, startY + digitSize.barLength + digitSize.laneWidth + 1, DIGIT_SHADOW_COLOR);
    }
    if (digit.botHbar)
    {
        paintHBar(startX + digitSize.laneWidth, startY + (digitSize.barLength + digitSize.laneWidth + 1) * 2, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + digitSize.laneWidth, startY + (digitSize.barLength + digitSize.laneWidth + 1) * 2, DIGIT_SHADOW_COLOR);
    }

    if (digit.topLeftVbar)
    {
        paintVBar(startX, startY + digitSize.laneWidth, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX, startY + digitSize.laneWidth, DIGIT_SHADOW_COLOR);
    }
    if (digit.botLeftVbar)
    {
        paintVBar(startX, startY + digitSize.laneWidth * 2 + digitSize.barLength, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX, startY + digitSize.laneWidth * 2 + digitSize.barLength, DIGIT_SHADOW_COLOR);
    }

    if (digit.topRightVbar)
    {
        paintVBar(startX + digitSize.laneWidth + digitSize.barLength + digitSize.lanePadding, startY + digitSize.laneWidth, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX + digitSize.laneWidth + digitSize.barLength + digitSize.lanePadding, startY + digitSize.laneWidth, DIGIT_SHADOW_COLOR);
    }
    if (digit.botRightVbar)
    {
        paintVBar(startX + digitSize.laneWidth + digitSize.barLength + digitSize.lanePadding, startY + digitSize.barLength + digitSize.laneWidth * 2, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX + digitSize.laneWidth + digitSize.barLength + digitSize.lanePadding, startY + digitSize.barLength + digitSize.laneWidth * 2, DIGIT_SHADOW_COLOR);
    }
}

void setup(void)
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);

    Serial.println("Setting up RTC");
    setupRtc();

    Serial.println("Setting up Tft display");
    setupTftDisplay();
    tft.fillScreen(ST77XX_BLACK);

    Serial.println("Set up complete");
}

void setupRtc()
{
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1)
            delay(10);
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
}

void setupTftDisplay()
{
    tft.init(SCREEN_HEIGHT, SCREEN_WIDTH);
    tft.setRotation(1); // Horizontal display. logic as-is does not support vertical display cleanly

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(25, SCREEN_HEIGHT / 2);
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_RED);
    tft.setTextWrap(true);
    tft.print("DEATHCLOCK");
    delay(1000);
}

void loop()
{
    uint32_t startTime = millis();
    refreshTft();
    uint32_t stopTime = millis();
    Serial.print("Total screen draw took ");
    Serial.print(stopTime - startTime);
    Serial.println(" ms");
    
    
    delay(15000);
}
