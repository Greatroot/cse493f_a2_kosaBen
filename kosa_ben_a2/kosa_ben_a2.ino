/****************************************************
This is Ben Kosa's A2 project for CSE 493F, 
Sp24 at the Univeristy of Washington

*****************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Shape.hpp>;
#include <ParallaxJoystick.hpp>;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // our OLED display width, in pixels
#define SCREEN_HEIGHT 64 // our OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define I/O pins
const int TONE_OUTPUT_PIN = 5;
const int VIBROMOTOR_OUTPUT_PIN = 8;
const int DEATH_LED_OUTPUT_PIN = 10;

// Set constants
const int DELAY_LOOP_MS = 5; 
const int P1_COLLISION_TONE_FREQUENCY = 100;
const int P2_COLLISION_TONE_FREQUENCY = 200;
const int PLAY_TONE_DURATION_MS = 200;
const int VIBROMOTOR_DURATION_MS = 200;

// Vibromotor timing
unsigned long _vibroMotorStartTimeStamp = -1;

// Joystick for player 1 (p1)
const int P1_SHOOT_BUTTON_PIN = 4;
const int JOYSTICK_UPDOWN_PIN = A0; // Because our joysticks are sideways, this is updown
const int JOYSTICK_LEFTRIGHT_PIN = A0;  // For current version of cowboy game, 
                                // we only care about this

const int MAX_ANALOG_VAL = 1023;
const enum JoystickYDirection JOYSTICK_Y_DIR = RIGHT;

// Digital buttons (joystick) for right player (P2)
const int UP_BUTTON_INPUT_PIN = 11;
const int DOWN_BUTTON_INPUT_PIN = 12;
const int P2_SHOOT_BUTTON_PIN = 13;

// Analog joystick for p1 (left player)
ParallaxJoystick _analogJoystick(JOYSTICK_UPDOWN_PIN, JOYSTICK_LEFTRIGHT_PIN, MAX_ANALOG_VAL, JOYSTICK_Y_DIR);

// Bitmaps:
// 'cowboy_simple', 15x23px (facing right)
const unsigned char P1CowboyBitmap [] PROGMEM = {
	0x0b, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x3f, 0xc0, 0x0f, 0x00, 0x0f, 0x00, 0x0e, 0x00, 0x0e, 0x00, 
	0x0e, 0x00, 0x3f, 0x80, 0x7f, 0xce, 0xef, 0xfc, 0xef, 0x38, 0xef, 0x00, 0x6f, 0x00, 0x0f, 0x80, 
	0x1d, 0xc0, 0x19, 0xe0, 0x38, 0xe0, 0x38, 0xe0, 0x78, 0xf0, 0x7c, 0xf8, 0x00, 0x00
};

// 'cowboy_simple_flipped', 15x23px (flipped and facing left)
const unsigned char P2CowboyBitmap [] PROGMEM = {
	0x01, 0xa0, 0x01, 0xe0, 0x01, 0xe0, 0x07, 0xf8, 0x01, 0xe0, 0x01, 0xe0, 0x00, 0xe0, 0x00, 0xe0, 
	0x00, 0xe0, 0x03, 0xf8, 0xe7, 0xfc, 0x7f, 0xee, 0x39, 0xee, 0x01, 0xee, 0x01, 0xec, 0x03, 0xe0, 
	0x07, 0x70, 0x0f, 0x30, 0x0e, 0x38, 0x0e, 0x38, 0x1e, 0x3c, 0x3e, 0x7c, 0x00, 0x00
};

// 'dead_bush', 15x23px
const unsigned char DeadBushBitmap [] PROGMEM = {
	0x03, 0x00, 0x03, 0x00, 0x33, 0x00, 0x33, 0x00, 0x3f, 0x30, 0x3f, 0xe0, 0x17, 0xc0, 0x0b, 0xc0, 
	0x3f, 0xfc, 0x7f, 0xfc, 0x7f, 0x30, 0x07, 0x20, 0x07, 0x00, 0x07, 0x00, 0x07, 0x80, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 'cactus', 24x27px
const unsigned char CactusBitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x40, 0x00, 0x0a, 0x40, 0x00, 0x0a, 0x50, 0x00, 0x0a, 
	0xd0, 0x00, 0x0f, 0x10, 0x00, 0x07, 0x90, 0x00, 0x13, 0x90, 0x00, 0x13, 0xf0, 0x00, 0x13, 0xe0, 
	0x00, 0x0f, 0xc4, 0x00, 0x03, 0x84, 0x00, 0x03, 0x04, 0x00, 0x03, 0x04, 0x00, 0x03, 0x1c, 0x00, 
	0x03, 0x18, 0x00, 0x03, 0x18, 0x00, 0x03, 0x18, 0x00, 0x03, 0x38, 0x00, 0x07, 0xfc, 0x00, 0x1f, 
	0xfe, 0x00, 0x3f, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00
};

// 'cowboyAiming7', 128x64px
const unsigned char CowboyAimingBitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x01, 0xf9, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x01, 0xfd, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x40, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xd3, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x01, 0xbf, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x03, 0x7f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0x80, 0x00, 0x20, 0x00, 0x01, 0x0f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xfe, 0x10, 0x00, 0xe0, 0x00, 0x03, 0x1f, 0xff, 0xfc, 0x20, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xfe, 0x78, 0x00, 0xe0, 0x00, 0x0f, 0x1f, 0xff, 0xfe, 0x10, 0x00, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xf0, 0x38, 0xe0, 0x07, 0x7f, 0x9f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xff, 0xe0, 0x19, 0xc0, 0x07, 0xff, 0xbf, 0xff, 0xff, 0x98, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xf0, 0x01, 0xc0, 0x07, 0xff, 0xbf, 0xff, 0xff, 0xe8, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xff, 0xff, 0xfc, 0x11, 0xe0, 0x07, 0xff, 0xbf, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xfd, 0xff, 0xd5, 0xe0, 0x08, 0x5f, 0xbf, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
	0x00, 0x1f, 0xff, 0xfc, 0xff, 0xfd, 0xf0, 0x00, 0x3f, 0xbf, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
	0x00, 0x3f, 0xbf, 0xff, 0xdf, 0xfd, 0xf8, 0x00, 0x7f, 0x3f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 
	0x00, 0x3f, 0xbf, 0xf8, 0x5f, 0xf9, 0xf8, 0x00, 0x0f, 0x7f, 0xff, 0xff, 0xf9, 0x80, 0x00, 0x00, 
	0x00, 0x7f, 0xbf, 0x81, 0xff, 0xfd, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfd, 0xe0, 0x00, 0x00, 
	0x02, 0x7f, 0xf8, 0x07, 0x1f, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 
	0x00, 0x7f, 0xc0, 0x3f, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 
	0x04, 0xfc, 0x03, 0xfc, 0x3f, 0xff, 0xff, 0x01, 0xb8, 0x3f, 0xff, 0xff, 0xff, 0x20, 0x00, 0x00, 
	0x00, 0xfc, 0x0f, 0xf1, 0x1f, 0xff, 0xff, 0x00, 0x08, 0x3f, 0xff, 0xff, 0xff, 0x20, 0x00, 0x00, 
	0x01, 0xfe, 0x7f, 0xc7, 0x0f, 0xfe, 0x7e, 0x00, 0x08, 0x7f, 0xff, 0xff, 0xff, 0x20, 0x00, 0x00, 
	0x01, 0xff, 0xff, 0x1c, 0x0f, 0xfe, 0x7e, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xa0, 0x00, 0x00, 
	0x03, 0xff, 0xf8, 0x78, 0x17, 0xff, 0x7f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 
	0x03, 0xfc, 0x31, 0xf1, 0x83, 0xff, 0xfe, 0x03, 0x01, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
	0x03, 0xf8, 0x1f, 0xc7, 0x80, 0xff, 0xce, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
	0x07, 0xf0, 0x1f, 0x0f, 0x84, 0x3f, 0x82, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
	0x07, 0xf0, 0x1c, 0x0e, 0xc6, 0x3c, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
	0x07, 0xf0, 0x18, 0x0e, 0x42, 0x38, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
	0x07, 0xf8, 0x1a, 0x04, 0x42, 0x30, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
	0x07, 0xfc, 0x73, 0x00, 0x42, 0x70, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
	0x27, 0xff, 0xfb, 0x80, 0x72, 0x70, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xee, 0x00, 0x00, 
	0x0f, 0xff, 0xf9, 0x80, 0x62, 0x70, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xee, 0x00, 0x00, 
	0x0f, 0xff, 0xf8, 0x80, 0x00, 0x70, 0x00, 0x00, 0x1c, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
	0x0f, 0xff, 0xf8, 0x00, 0x24, 0x30, 0x00, 0x00, 0x1c, 0x7f, 0xff, 0xff, 0xff, 0xf7, 0x00, 0x00, 
	0x0f, 0xff, 0xf8, 0x00, 0x3f, 0x20, 0x00, 0x80, 0x08, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x4f, 0xff, 0xdc, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x1f, 0xff, 0x9f, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x1f, 0xff, 0xbf, 0xc0, 0x0f, 0xc0, 0x00, 0x20, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
	0x0f, 0xff, 0xbf, 0xe0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x03, 0xff, 0xed, 0xff, 0xff, 0x00, 0x00, 
	0x07, 0xff, 0xec, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xec, 0xff, 0xff, 0x00, 0x00, 
	0x07, 0xff, 0xcc, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe8, 0xff, 0xff, 0x00, 0x00, 
	0x07, 0xf3, 0xce, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x7f, 0xff, 0x00, 0x00, 
	0x07, 0xf3, 0xcf, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x80, 0x00, 0x0f, 0xf8, 0x7f, 0xff, 0x00, 0x00, 
	0x07, 0xf3, 0x9f, 0xe0, 0x0e, 0x40, 0x00, 0x01, 0x00, 0x00, 0x03, 0xf0, 0x7f, 0xfe, 0x00, 0x00, 
	0x07, 0xf1, 0x9b, 0xfe, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x7f, 0xfa, 0x00, 0x00, 
	0x07, 0xe3, 0x9a, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x78, 0x00, 0x00, 
	0x07, 0xe1, 0x98, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x78, 0x00, 0x00, 
	0x07, 0xe1, 0xd8, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x30, 0x00, 0x00, 
	0x07, 0x81, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x30, 0x00, 0x00, 
	0x07, 0x81, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x00, 0x00, 
	0x07, 0x81, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x30, 0x00, 0x00, 
	0x02, 0x01, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x82, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xef, 0xe0, 0x00, 0x00
};

// 'bang2', 128x64px
const unsigned char BangBitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0f, 0xc7, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0xfe, 0x0f, 0x9d, 0xdc, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf1, 0x83, 0x1c, 0xd1, 0xf0, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x31, 0x83, 0x18, 0x70, 0xe0, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x1b, 0x03, 0x10, 0x70, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x9f, 0x1f, 0x01, 0x90, 0x70, 0x81, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x9d, 0x1e, 0x01, 0x90, 0x70, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x9f, 0x3a, 0x01, 0x90, 0x70, 0x0e, 0x20, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x9e, 0x7a, 0x31, 0x90, 0x21, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0xf6, 0x3f, 0xd0, 0xc1, 0x9c, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf1, 0xe6, 0x3f, 0xd1, 0xe1, 0xbd, 0xfe, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x64, 0x3b, 0xf0, 0x00, 0x19, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x3c, 0x78, 0xf0, 0x00, 0x19, 0x06, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x0c, 0x0e, 0x08, 0x70, 0xf0, 0x80, 0x19, 0x8e, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x9f, 0x88, 0x30, 0x71, 0x80, 0x0f, 0x8c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x80, 0x30, 0x21, 0x81, 0x8f, 0x8c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x80, 0x78, 0x21, 0xc1, 0x86, 0x0c, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x80, 0x68, 0x01, 0xe3, 0xc0, 0x18, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x00, 0x6d, 0x01, 0xe3, 0xc0, 0x38, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xbe, 0x00, 0xef, 0x31, 0xff, 0xe0, 0x70, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3c, 0xef, 0xf1, 0xff, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0xf1, 0xe7, 0xe7, 0xb8, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x9f, 0xff, 0xc6, 0x7f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x00, 0x38, 0x00, 0xfe, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x03, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x06, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x0c, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x18, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x38, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Array of all bitmaps for convenience. (Total bytes used to store images in PROGMEM = 128)
const int epd_bitmap_allArray_LEN = 4;
const unsigned char* bitmapArray[6] = {
	P1CowboyBitmap,
  P2CowboyBitmap,
  DeadBushBitmap,
  CactusBitmap,
  CowboyAimingBitmap,
  BangBitmap
};

// Keep track of running milliseconds so we can avoid
// using the delay() function for delays.
// unsigned long _lastToggledTimestampMs = 0; // tracks the last time the OLED was updated

#define COWBOY_WIDTH 15  // TODO: Tweak the sizes
#define COWBOY_HEIGHT 23  // TODO: Tweak the sizes

const char STR_LOADSCREEN_CREATOR[] = "Ben Kosa 2024";
const char STR_LOADSCREEN_GAME_NAME[] = "HIGH NOON";
const char STR_PRESS_BUTTON_TO_START[] = "PRESS ANY BUTTON TO START";
const char STR_PLAYER1_WON[] = "PLAYER 1 WINS!";
const char STR_PLAYER2_WON[] = "PLAYER 2 WINS!";
const char STR_GAME_OVER[] = "GAME OVER!";
const char STR_GAME_POINT[] = "GAME POINT!";  // TODO: Remove if not needed

// TODO: change to cowboy sprites
PlayerBitMap _leftCowboy(0, SCREEN_HEIGHT / 2 - COWBOY_HEIGHT / 2, COWBOY_WIDTH, COWBOY_HEIGHT, P1CowboyBitmap);
PlayerBitMap _rightCowboy(SCREEN_WIDTH - COWBOY_WIDTH, SCREEN_HEIGHT / 2 - COWBOY_HEIGHT / 2, COWBOY_WIDTH, COWBOY_HEIGHT, P2CowboyBitmap);

// We'll use Balls as bullets
// Initially draw them outside the screen just over their redraw bounds
Ball _p1Bullet(SCREEN_WIDTH + 15, 20, 2);
Ball _p2Bullet(-15, 20, 2);

// Keep track of both player's scores
int _p1Score = 0;  // p1 = Left Player
int _p2Score = 0;  // p2 = Right Player

// Keep track of whether the p1 or p2 bullet is firing or not
// initially no bullets are firing
bool _p1BulletFiring = false;
bool _p2BulletFiring = false;
int BULLET_SPEED = 5;  // TODO: Modify if needed

const int GAME_OVER_SCORE = 3;

enum GameState {
  NEW_GAME,
  PLAYING,
  GAME_OVER,
};

enum Players {
  P1 = 0,  // P1 = Left Player
  P2 = 1  // P1 = Left Player
};

enum GameState _curGameState = NEW_GAME;
enum Players _gameWinner = P1;

// for tracking fps
float _fps = 0;
unsigned long _frameCount = 0;
unsigned long _fpsStartTimeStamp = 0;
const boolean _drawFps = false; // change to show/hide fps display

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  // For debugging

  // Setup digital output for piezo buzzer
  // and vibromotor
  pinMode(TONE_OUTPUT_PIN, OUTPUT);
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
  pinMode(DEATH_LED_OUTPUT_PIN, OUTPUT);

  // Setup digital joystick input for P2
  pinMode(UP_BUTTON_INPUT_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_INPUT_PIN, INPUT_PULLUP);

  // Setup digital fire button input for p1 and p2
  pinMode(P1_SHOOT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(P2_SHOOT_BUTTON_PIN, INPUT_PULLUP);

  // TODO: Change this to be dynamic with flashing "PRESS ANY BUTTON TO START"
  initializeOledAndShowLoadScreen();

  _leftCowboy.setDrawFill(true);
  _rightCowboy.setDrawFill(true);
  _p1Bullet.setDrawFill(true);
  _p2Bullet.setDrawFill(true);

  // For tracking fps
  _fpsStartTimeStamp = millis();

  // For making sure that p1 and p2 have to wait 2 seconds
  // when they fire a bullet before they can fire another
  // bullet (2 sec cooldown).
  // _p1MsSinceFired = millis();  // Whenever p1 hits fire button, this num must be over 2000ms
  // _p2MsSinceFired = millis();  // Whenever p2 hits fire button, this num must be over 2000ms
}

void loop() {
  // Clear the display first at beginning of loop
  // cycle
  _display.clearDisplay();

  // Read in val of all of our digital inputs
  // P2 Digtial Joystick buttons (up and down)
  //   (read in value and update y-pos of leftCowboy accordingly)
  int upButtonVal = digitalRead(UP_BUTTON_INPUT_PIN);
  int downButtonVal = digitalRead(DOWN_BUTTON_INPUT_PIN);
  // Serial.print("\nupButtonVal: ");
  // Serial.println(upButtonVal);
  // Serial.print("downButtonVal: ");
  // Serial.println(downButtonVal);

  // Fire buttons (P1 and P2)
  int p1FireButtonVal = digitalRead(P1_SHOOT_BUTTON_PIN);
  int p2FireButtonVal = digitalRead(P2_SHOOT_BUTTON_PIN);
  // Serial.print("p1FireButton: ");
  // Serial.println(p1FireButtonVal);
  // Serial.print("p2FireButton: ");
  // Serial.println(p2FireButtonVal);

  // Serial.print("_curGameState: ");
  // Serial.println(_curGameState);

  // TODO: First create the game loop
  if (_curGameState == NEW_GAME) {
    // TODO: Show the "PRESS ANY BUTTON TO START" screen
    // New Game Screen Loop
    //   1. Check to see if any buttons have been pressed
    //      - If yes, then set _curGameState == NEW_STATE
    //      - If no, then just continue to draw Start Screen
    
    // Check val of P2 digital joystick buttons
    if (upButtonVal == LOW || downButtonVal == LOW || p1FireButtonVal == LOW || p2FireButtonVal == LOW) {
      _curGameState = PLAYING;
    }
    
    drawStartScreen();
  } else if (_curGameState == GAME_OVER) {
    // TODO: Draw Game Over scene and delay for 3 seconds (show "Player 1 Wins" or "Player 2 Wins")
    digitalWrite(DEATH_LED_OUTPUT_PIN, HIGH);  // turn LED on to show game over
    drawGameOver();
    delay(2000);
    digitalWrite(DEATH_LED_OUTPUT_PIN, LOW);  // turn LED on to show game over

    // Reset points and states
    _curGameState = NEW_GAME;
    _p1Score = 0;
    _p2Score = 0;

  } else { // GAME_STATE == PLAYING
    // Start game loop
    // Game loop flow:
    //   1. First draw the dueling scene (cacti & dead bushes in middle)
    //   2. Draw cowboys at their current positions
    //   3. Check if the P1 or P2 movemment sticks have been pressed (P1 analog & P2 digital)
    //        - If so, then update the position values of the cowboys accordingly (using .setY)
    //   4. Check if the P1 or P2 fire buttons have been pressed
    //        - If so, toggle _p1BulletFiring or _p2BulletFiring to True and keep drawing bullet
    //          and updating it's position until it goes off screen.
    //        - Once bullet is offscreen, set _p1BulletFiring or _p2BulletFiring to False.
    //        - While p1 or p2 bullet is firing, be checking if collision to opposite cowboy sprite
    //            - If yes, then increase pts of player who bullet belongs to
    //              and check if game is over (by checking points)
    //              - if game over, then set state to game over.

    // Draw the dueling battlefield
    drawDesert();

    // During  loop, read in the different joystick (analog & digital) inputs & fire buttons
    // P1 Analog Joystick y-axis (read in value and update y-pos of leftCowboy accordingly)
    _analogJoystick.read();
    int upDownVal = _analogJoystick.getUpDownVal();
    int yMovementPixels = map(upDownVal, 0, _analogJoystick.getMaxAnalogValue() + 1, -4, 5);
    // Serial.print("\nAnalog Joystick yMovementPixels: ");
    // Serial.println(yMovementPixels);

    _leftCowboy.setY(_leftCowboy.getY() - yMovementPixels);
    _leftCowboy.forceInside(0, 0, _display.width(), _display.height());

    // P2 Digtial Joystick buttons (up and down)
    //   (read in value and update y-pos of leftCowboy accordingly)
    if (upButtonVal == LOW) {
      _rightCowboy.setY(_rightCowboy.getY() - 2);
      //Serial.println((String)"Right paddle: upButtonVal:" + upButtonVal + " downButtonVal:" + downButtonVal);
    } else if (downButtonVal == LOW) {
      _rightCowboy.setY(_rightCowboy.getY() + 2);
      //Serial.println((String)"Right paddle: upButtonVal:" + upButtonVal + " downButtonVal:" + downButtonVal);
    }
    _rightCowboy.forceInside(0, 0, _display.width(), _display.height());

    // Check if fire buttons (P1 and P2) pressed
    // 3 different potential states for both p1 and p2:
    if (p1FireButtonVal == LOW && _p1Bullet.getLeft() > SCREEN_WIDTH) {
      // If P1 hit the fire button AND the bullet is currently out of bounds (meaning
      // it's not currently flying across the screen), then fire bullet.
      _p1Bullet.setLocation(_leftCowboy.getRight(), _leftCowboy.getTop() + _leftCowboy.getHeight() / 2);
      _p1Bullet.setSpeed(BULLET_SPEED, 0);
    }
    
    if (p2FireButtonVal == LOW && _p2Bullet.getRight() < 0) {
      // If P1 hit the fire button AND the bullet is currently out of bounds (meaning
      // it's not currently flying across the screen), then fire bullet.
      _p2Bullet.setLocation(_rightCowboy.getLeft(), _rightCowboy.getTop() + _rightCowboy.getHeight() / 2);
      _p2Bullet.setSpeed(-BULLET_SPEED, 0);
    }

    // Serial.print("What is the position of _p1Bullet?: ");
    // Serial.println(_p1Bullet.getLeft());

    // Check to see if any bullets are firing and keep updating speed
    // In future, could also just check if p1Bullet is beyond right-most bounds of screen
    // instead of checking ms since fired
    if (_p1Bullet.getLeft() <= SCREEN_WIDTH) {
      // Serial.println("Are we updating the p1 bullet???");
      _p1Bullet.update();
      _p1Bullet.draw(_display);
    }

    if (_p2Bullet.getRight() >= 0) {
      _p2Bullet.update();
      _p2Bullet.draw(_display);
    }

    // Check to see if p1Bullet collided with p2 or vice versa
    // and play death animation for the player that's hit
    if (_leftCowboy.overlaps(_p2Bullet)) {
      // If p2 bullet hits p1, then reset position of bullets,
      // update score, and the continue game
      // _leftCowboy.setDrawFill(false);
      // _leftCowboy.draw(_display);
      // delay(200);
      // _leftCowboy.setDrawFill(true);
      // _leftCowboy.draw(_display);
      // delay(200);
      // _leftCowboy.setDrawFill(false);
      // _leftCowboy.draw(_display);
      // delay(200);
      // _leftCowboy.setDrawFill(true);
      // _leftCowboy.draw(_display);

      // If the player gets shot, then vibrate the vibromotor
      // to give haptic feedback & play a tone
      // Play slightly higher tone when bullet hits player
      tone(TONE_OUTPUT_PIN, P1_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
      _vibroMotorStartTimeStamp = millis();

      _p2Bullet.setLocation(-15, 20);
      // Update the score for p2 
      _p2Score += 1;
    }

    if (_rightCowboy.overlaps(_p1Bullet)) {
      // If p1 bullet hits p2, then have p2 blink for now to signify
      // them dying and then continue game
      // _rightCowboy.setDrawFill(false);
      // _rightCowboy.draw(_display);
      // delay(500);
      // _rightCowboy.setDrawFill(true);
      // _rightCowboy.draw(_display);
      // delay(500);
      // _rightCowboy.setDrawFill(false);
      // _rightCowboy.draw(_display);
      // delay(500);
      // _rightCowboy.setDrawFill(true);
      // _rightCowboy.draw(_display);

      // If the player gets shot, then vibrate the vibromotor
      // to give haptic feedback & play a tone
      // Play slightly higher tone when bullet hits player
      tone(TONE_OUTPUT_PIN, P2_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
      _vibroMotorStartTimeStamp = millis();

      _p1Bullet.setLocation(SCREEN_WIDTH + 15, 20);
      // Update the score for p1
      _p1Score += 1;
    }

    // Check for vibromotor output
    if(_vibroMotorStartTimeStamp != -1){
      if(millis() - _vibroMotorStartTimeStamp > VIBROMOTOR_DURATION_MS){
        _vibroMotorStartTimeStamp = -1;
        digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
      }
    }

    // draw scores
    drawScores();

    _leftCowboy.draw(_display);
    _rightCowboy.draw(_display);

    // Check to make sure it isn't the end of the game now
    if (_p1Score >= GAME_OVER_SCORE) {
      // P1 has won
      _gameWinner = P1;
      _curGameState = GAME_OVER;
    }

    if (_p2Score >= GAME_OVER_SCORE) {
      // P2 has won
      _gameWinner = P2;
      _curGameState = GAME_OVER;
    }
  }

  // Centering text on the screen:
  // int16_t x, y;
  // int16_t textWidth, textHeight;
  // const char strHello[] = "Hello Makers!";

  // _display.getTextBounds(strHello, 0, 0, &x, &y, &textWidth, &textHeight);
  // _display.setCursor(_display.width() / 2 - textWidth / 2, _display.height() / 2 - textHeight / 2);
  // _display.print(strHello);
  // _display.display();

  // Draw fps
  if (_drawFps && _curGameState == PLAYING) {
    drawFrameRate();
  }
  calcFrameRate();

  // Render buffer to screen
  _display.display();

  if (DELAY_LOOP_MS > 0) {
    delay(DELAY_LOOP_MS);
  }

}

/************** UTILITY FUNCTIONS **************/

/**
 * Call this from setup() to initialize OLED display and show load screen
 */
void initializeOledAndShowLoadScreen() {
  // SSD1306_SWITCHCAPVCC = generate _display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  
  // Show the start screen
  drawStartScreen();
}

/**
 * Draw the "PRESS ANY BUTTON TO START"
 */
 void drawStartScreen() {
  // Clear the buffer
  _display.clearDisplay();

  // Show load screen
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);

  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds(STR_LOADSCREEN_CREATOR, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, 15);
  _display.print(STR_LOADSCREEN_CREATOR);

  _display.setTextSize(2.5);
  _display.getTextBounds(STR_LOADSCREEN_GAME_NAME, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, 25);
  _display.print(STR_LOADSCREEN_GAME_NAME);

  _display.setTextSize(1);
  _display.getTextBounds(STR_PRESS_BUTTON_TO_START, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, 65);
  _display.print(STR_PRESS_BUTTON_TO_START);

  _display.display();
  // delay(500);
  // _display.clearDisplay();
  // _display.setTextSize(1);
 }

/**
 * Draw the scores to the display
 */
void drawScores(){
  //_display.getTextBounds((String)("" + _leftPlayerScore), 0, 0, &x1, &y1, &w, &h);
  //_display.setCursor(_display.width() / 4 - w / 2, 0);
  _display.setTextSize(2);
  _display.setCursor(_display.width() / 4, 0);
  _display.print(_p1Score);

  //_display.getTextBounds((String)("" + _rightPlayerScore), 0, 0, &x1, &y1, &w, &h);
  _display.setCursor((int)(_display.width() * 0.75), 0);
  _display.print(_p2Score);
}

/**
 * Draw the cowboy duel field (desert patch)
 */
void drawDesert(){
  // Place the two dead bushes and cactus in the proper spots
  // [0] 'cowboy_simple', 15x23px (facing right)
  // [1] 'cowboy_simple_flipped', 15x23px (flipped and facing left)
  // [2] 'dead_bush', 15x23px
  // [3] 'cactus', 24x27px

  // Draw left bush bitmap
  _display.drawBitmap(26, 18,  bitmapArray[2], 15, 23, WHITE);

  // Draw left bush bitmap
  _display.drawBitmap(96, 48,  bitmapArray[2], 15, 23, WHITE);

  //Draw the cactus in the middle
  _display.drawBitmap(SCREEN_WIDTH / 2 - 12, SCREEN_HEIGHT / 2 - 13,  bitmapArray[3], 24, 27, WHITE);

  // _display.display();
}

/**
   Draws game over screen to display
*/
void drawGameOver() {
  int16_t x1, y1;
  uint16_t w, h;

  // Display gameover bitmaps
  _display.clearDisplay();
  _display.drawBitmap(0, 0,  CowboyAimingBitmap, 128, 64, WHITE);
  _display.display();
  delay(1000);

  _display.clearDisplay();
  _display.drawBitmap(0, 0,  BangBitmap, 128, 64, WHITE);
  _display.display();
  delay(1000);

  _display.clearDisplay();

  _display.setTextSize(2);
  _display.getTextBounds(STR_GAME_OVER, 0, 0, &x1, &y1, &w, &h);

  int yGameOverText = 10;
  int yPlayerWonText = yGameOverText + h + 2;
  _display.setCursor(_display.width() / 2 - w / 2, yGameOverText);
  _display.print(STR_GAME_OVER);

  _display.setTextSize(1);
  if (_gameWinner == P1) {
    _display.getTextBounds(STR_PLAYER1_WON, 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(_display.width() / 2 - w / 2, yPlayerWonText);
    _display.print(STR_PLAYER1_WON);
  } else {
    _display.getTextBounds(STR_PLAYER2_WON, 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(_display.width() / 2 - w / 2, yPlayerWonText);
    _display.print(STR_PLAYER2_WON);
  }
  _display.display();
}

/**
   Call this every frame to calculate frame rate
*/
void calcFrameRate() {
  unsigned long elapsedTime = millis() - _fpsStartTimeStamp;
  _frameCount++;
  if (elapsedTime > 1000) {
    _fps = _frameCount / (elapsedTime / 1000.0);
    _fpsStartTimeStamp = millis();
    _frameCount = 0;
  }
}

/**
   Draw frame rate
*/
void drawFrameRate() {
  // Draw frame count
  int16_t x1, y1;
  uint16_t w, h;
  _display.getTextBounds("XX.XX fps", 0, 0, &x1, &y1, &w, &h);

  // Draw fps in bottom right corner
  _display.setCursor(_display.width() - w, _display.height()-h);
  _display.print(_fps);
  _display.print(" fps");
}