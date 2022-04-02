#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <RTClib.h>

RTC_DS3231 rtc;

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS 10
#define TFT_RST 9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const DateTime birthdatePlus9Years = DateTime(2000, 6, 11, 16, 24, 0);
const DateTime deathdate = DateTime(2031, 6, 11, 16, 24, 0); // big 4-0

float p = 3.1415926;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170

#define DIGIT_WIDTH 50
#define DIGIT_HEIGHT 90
#define DIGIT_COLOR 0x07f9
#define DIGIT_SHADOW_COLOR 0x0022

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

const DrawableDigit digits[12] = {
    DrawableDigit{true, false, true, true, true, true, true},
    DrawableDigit{false, false, false, false, false, true, true},
    DrawableDigit{true, true, true, false, true, true, false},
    DrawableDigit{true, true, true, false, false, true, true},
    DrawableDigit{false, true, false, true, false, true, true},
    DrawableDigit{true, true, true, true, false, false, true},
    DrawableDigit{true, true, true, true, true, false, true},
    DrawableDigit{true, false, false, false, false, true, true},
    DrawableDigit{true, true, true, true, true, true, true},
    DrawableDigit{true, true, true, true, false, true, true},       // 9
    DrawableDigit{false, false, false, false, false, false, false}, // empty
    DrawableDigit{false, true, false, false, false, false, false}   // -
};

void setup(void)
{
    Serial.begin(9600);
    while (!Serial)
        delay(10);

    Serial.println("Setting up RTC");
    setupRtc();

    Serial.println("Setting up Tft display");
    setupTftDisplay();

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
    tft.setRotation(1);
    Serial.println(F("ST7789 Initialized"));

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
    refreshTft();
    delay(15000);
}

const uint8_t barLength = 20;
const uint8_t barWidth = 6;
// bars occupy lanes within this width
const uint8_t lanePadding = 3;
const uint8_t laneWidth = barWidth + lanePadding;

const uint8_t digitWidth = laneWidth * 2 + barLength + lanePadding * 2;
const uint8_t digitHeight = laneWidth * 3 + (barLength + lanePadding) * 2;

enum class ModeSelection
{
    HOURS,
    DAYS,
};

ModeSelection selectedMode = ModeSelection::DAYS;

void refreshTft()
{
    tft.fillScreen(ST77XX_BLACK);

    DateTime now = rtc.now();
    const uint32_t remainingTimeSeconds = deathdate.unixtime() - now.unixtime();
    const TimeSpan &remainingTimeSpan = TimeSpan(remainingTimeSeconds);
    const uint32_t hoursRemaining = ((uint32_t)remainingTimeSpan.days() * 24) + (uint32_t)remainingTimeSpan.hours();

    switch (selectedMode)
    {
    case ModeSelection::HOURS:
        drawDigits(hoursRemaining);
        break;

    case ModeSelection::DAYS:
        drawDigits(remainingTimeSpan.days());
        break;
    }

    const auto nineYears = TimeSpan(3285, 0, 0, 0);
    const float currentAgeSeconds = (now.unixtime() - birthdatePlus9Years.unixtime()) + nineYears.totalseconds();
    const float deathDateSeconds = deathdate.unixtime() - birthdatePlus9Years.unixtime() + nineYears.totalseconds();

    drawProgressBar(1.0f - (currentAgeSeconds / deathDateSeconds));
}

void drawProgressBar(const float percent)
{
    Serial.print("Percentage: ");
    Serial.println(percent);

    const uint16_t startX = lanePadding;
    const uint16_t startY = SCREEN_HEIGHT - lanePadding - barWidth;

    const float maxProgBarLength = SCREEN_WIDTH - lanePadding * 2;
    tft.fillRect(startX - 1, startY - 1, maxProgBarLength + 2, barWidth + 2, DIGIT_SHADOW_COLOR);

    const float progBarLength = percent * (maxProgBarLength - lanePadding);
    tft.fillRect(startX, startY, startX + progBarLength, barWidth - 1, DIGIT_COLOR);

    tft.setCursor(progBarLength, startY - 10);
    tft.setTextSize(1);
    tft.setTextColor(DIGIT_COLOR);
    tft.print(percent * 100);
    tft.print("% remaining life");
}

const uint16_t digitXOffs = DIGIT_WIDTH;
const uint16_t digitYOffs = (SCREEN_HEIGHT - digitHeight) / 2;
const uint8_t maxNumDigits = 5;

void drawDigits(uint32_t num)
{
    String numStr = String(num);

    for (int i = maxNumDigits; i >= 0; --i)
    {
        int index;
        if (i < numStr.length())
        {
            index = numStr[numStr.length() - 1 - i] - '0';
        }
        else if (i == numStr.length()) {
            index = 11;
        }
        else
        {
            index = 10;
        }
        paintDigit(digits[index], (digitWidth + lanePadding) * (maxNumDigits - i), digitYOffs);
    }

    // hours mode
    tft.setCursor(SCREEN_WIDTH - 34, digitYOffs + digitHeight - barLength - lanePadding * 2);
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
    tft.setCursor(SCREEN_WIDTH - 30, digitYOffs + digitHeight - barLength + lanePadding * 2);
    tft.setTextSize(1);
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
        startX - lanePadding, startY + lanePadding,
        startX, startY,
        startX, startY + barWidth - 1, color);                // left tri
    tft.fillRect(startX, startY, barLength, barWidth, color); //  vbar
    tft.fillTriangle(
        startX + barLength + lanePadding, startY + lanePadding,
        startX + barLength, startY,
        startX + barLength, startY + barWidth - 1, color); // right triangle
}

void paintVBar(uint16_t startX, uint16_t startY, uint16_t color)
{
    tft.fillTriangle(
        startX + barWidth / 2, startY - lanePadding,
        startX, startY,
        startX + barWidth - 1, startY, color);                // top tri
    tft.fillRect(startX, startY, barWidth, barLength, color); // hbar
    tft.fillTriangle(
        startX + barWidth / 2, startY + barLength + lanePadding,
        startX, startY + barLength,
        startX + barWidth - 1, startY, color); // bot triangle
}

void paintDigit(const DrawableDigit digit, uint16_t startX, uint16_t startY)
{
    if (digit.topHbar)
    {
        paintHBar(startX + laneWidth, startY, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + laneWidth, startY, DIGIT_SHADOW_COLOR);
    }
    if (digit.midHbar)
    {
        paintHBar(startX + laneWidth, startY + barLength + laneWidth + 1, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + laneWidth, startY + barLength + laneWidth + 1, DIGIT_SHADOW_COLOR);
    }
    if (digit.botHbar)
    {
        paintHBar(startX + laneWidth, startY + (barLength + laneWidth + 1) * 2, DIGIT_COLOR);
    }
    else
    {
        paintHBar(startX + laneWidth, startY + (barLength + laneWidth + 1) * 2, DIGIT_SHADOW_COLOR);
    }

    if (digit.topLeftVbar)
    {
        paintVBar(startX, startY + laneWidth, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX, startY + laneWidth, DIGIT_SHADOW_COLOR);
    }
    if (digit.botLeftVbar)
    {
        paintVBar(startX, startY + laneWidth * 2 + barLength, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX, startY + laneWidth * 2 + barLength, DIGIT_SHADOW_COLOR);
    }

    if (digit.topRightVbar)
    {
        paintVBar(startX + laneWidth + barLength + lanePadding, startY + laneWidth, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX + laneWidth + barLength + lanePadding, startY + laneWidth, DIGIT_SHADOW_COLOR);
    }
    if (digit.botRightVbar)
    {
        paintVBar(startX + laneWidth + barLength + lanePadding, startY + barLength + laneWidth * 2, DIGIT_COLOR);
    }
    else
    {
        paintVBar(startX + laneWidth + barLength + lanePadding, startY + barLength + laneWidth * 2, DIGIT_SHADOW_COLOR);
    }
}
