/**************************************************************************
  Check out the links for tutorials and wiring diagrams.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/
//#define ESP32_S3_TFT // for ESP32-S3 TFT
#define HX8357 // for the Adafruit 3.5" TFT (HX8357) FeatherWing

#include <Adafruit_GFX.h>    // Core graphics library
#ifdef ESP32_S3_TFT
  // This is a library for several Adafruit displays based on ST77* drivers.
  // Works with the Adafruit TFT Gizmo
  // ----> http://www.adafruit.com/products/4367
  #include <Adafruit_ST7789.h> // Hardware-specific library for ST7789 // on ESP32-S3 TFT
#endif
#ifdef HX8357
  // This is our library for the Adafruit 3.5" TFT (HX8357) FeatherWing
  // ----> http://www.adafruit.com/products/3651
  #include "Adafruit_HX8357.h" // on 3.5" TFT (HX8357) FeatherWing

  // If using the rev 1 with STMPE resistive touch screen controller uncomment this line:
  //#include <Adafruit_STMPE610.h>
  // If using the rev 2 with TSC2007, uncomment this line:
  #include <Adafruit_TSC2007.h>
#endif

#include <SPI.h>
#include <Wire.h> // for i2c keyboard, DRV2605 haptic feedback
#include "Adafruit_seesaw.h" // mini gamepad (on STEMMA QT)

//#include <Adafruit_VS1053.h> // for mp3 playing from microSD card; wants same pins as TFT? (5, 6, 9, 10)
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv; // haptic

//#define DEBUG_BUTTONS

// ------------------------- TFT setup -------------------------

#ifdef ESP32_S3_TFT
  // Use dedicated hardware SPI pins
  Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

  // on ESP32-S3 TFT
  #define PIXELS_X 240
  #define PIXELS_Y 135
#endif

#ifdef HX8357
  #ifdef ESP8266
    #define STMPE_CS 16
    #define TFT_CS   0
    #define TFT_DC   15
    #define SD_CS    2
  #elif defined(ESP32) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2) && !defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S3)
    #define STMPE_CS 32
    #define TFT_CS   15
    #define TFT_DC   33
    #define SD_CS    14
  #elif defined(TEENSYDUINO)
    #define TFT_DC   10
    #define TFT_CS   4
    #define STMPE_CS 3
    #define SD_CS    8
  #elif defined(ARDUINO_STM32_FEATHER)
    #define TFT_DC   PB4
    #define TFT_CS   PA15
    #define STMPE_CS PC7
    #define SD_CS    PC5
  #elif defined(ARDUINO_NRF52832_FEATHER)  /* BSP 0.6.5 and higher! */
    #define TFT_DC   11
    #define TFT_CS   31
    #define STMPE_CS 30
    #define SD_CS    27
  #elif defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
    #define TFT_DC   P5_4
    #define TFT_CS   P5_3
    #define STMPE_CS P3_3
    #define SD_CS    P3_2
  #else
      // Anything else, defaults!
    #define STMPE_CS 6
    #define TFT_CS   9
    #define TFT_DC   10
    #define SD_CS    5
  #endif

  #define TFT_RST -1

  // Use hardware SPI and the above for CS/DC
  Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// --------------------- touchscreen setup ---------------------

  #if defined(_ADAFRUIT_STMPE610H_)
    Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
  #elif defined(_ADAFRUIT_TSC2007_H)
    // If you're using the TSC2007 there is no CS pin needed, so instead its an IRQ!
    #define TSC_IRQ STMPE_CS
    Adafruit_TSC2007 ts = Adafruit_TSC2007();             // newer rev 2 touch contoller
  #else
    #error("You must have either STMPE or TSC2007 headers included!")
  #endif

  // This is calibration data for the raw touch data to the screen coordinates
  // For STMPE811/STMPE610
  #define STMPE_TS_MINX 3800
  #define STMPE_TS_MAXX 100
  #define STMPE_TS_MINY 100
  #define STMPE_TS_MAXY 3750
  // For TSC2007
  #define TSC_TS_MINX 300
  #define TSC_TS_MAXX 3800
  #define TSC_TS_MINY 185
  #define TSC_TS_MAXY 3700
  // set during setup(); use during loop()
  int16_t touch_minx, touch_maxx;
  int16_t touch_miny, touch_maxy;
#endif

// --------------------- colors ------------------------

enum color_e {
#ifdef ESP32_S3_TFT
  color_black = ST77XX_BLACK,
  color_white = ST77XX_WHITE,
  color_blue = ST77XX_BLUE,
  color_green = ST77XX_GREEN,
  color_yellow = ST77XX_YELLOW,
  color_red = ST77XX_RED,
  color_magenta = ST77XX_MAGENTA,
#endif
#ifdef HX8357
  color_black = HX8357_BLACK,
  color_white = HX8357_WHITE,
  color_blue = HX8357_BLUE,
  color_green = HX8357_GREEN,
  color_yellow = HX8357_YELLOW,
  color_red = HX8357_RED,
  color_magenta = HX8357_MAGENTA,
#endif
  // raw 565 format colors:
  // Source: https://github.com/newdigate/rgb565_colors/tree/main?tab=readme-ov-file#gray
  color_silver = 0xbdf7,
  //color_lt_gray = 0x6B4D, // dim gray 
  color_lt_gray = 0xd69a,
  color_dk_gray = 0x29A7, // gunmetal gray

  color_backgr = color_black,
  color_highlight = color_silver, // color_white,
  //color_shadow = 0x328A, // dark slate gray
  color_shadow = color_dk_gray,
  color_window = color_lt_gray,

  color_cap_shadow = color_black,
  color_cap_bar = color_shadow,
  color_cap_label = color_white,

  color_wndw_text = color_black,
  color_btn_text = color_black,
  color_edit_text = color_black,
  color_edit_backgr = color_white,
  color_edit_cursor = color_black,

  color_focus_border = color_black,

  color_count,
};

// --------------------- input ------------------------

// on-board digital input button
#define PIN_BOOT_BUTTON  0 // on ESP32 S3 TFT

// mini gamepad (on STEMMA QT)
Adafruit_seesaw ss;
#define BUTTON_X         6
#define BUTTON_Y         2
#define BUTTON_A         5
#define BUTTON_B         1
#define BUTTON_SELECT    0
#define BUTTON_START    16
uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) |
                       (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);
//#define IRQ_PIN   5

// ----------------------- window structures ----------------------------

enum border_style_e {
  border_outset = 0,
  border_ridge,
  border_outset_narrow,
  border_inset,
  border_focus,
  border_style_count,
};

struct windw_s {
	int wx, wy, ww, wh; // Window coords.
	int border_style;		//  enum border_style_e
  int buffered;		    // Flag for buffer.
  GFXcanvas16 *canvas; // 16-bit, width x height pixels
};

#define WINDW_MAX_CAP 61
#define CHAR_WIDTH_PIXELS 12
#define CHAR_HEIGHT_PIXELS 16

struct cap_windw_s {
  struct windw_s windw;
  char label[WINDW_MAX_CAP]; // full cap
};

#define WINDW_MAX_LINE 61

struct cap_twindw_s {
  struct cap_windw_s cwindw;
  char line1[WINDW_MAX_LINE];
  char line2[WINDW_MAX_LINE];
  int button;
};

#define BUTTON_WIDTH_PIXELS  64
#define BUTTON_HEIGHT_PIXELS 32
#define BUTTON_MAX_LABEL 20

struct button_s {
  struct windw_s windw;
  char label_str[BUTTON_MAX_LABEL];
	unsigned hotkey;
	int altkey;
  //GFXcanvas16 canvas(BUTTON_WIDTH_PIXELS, BUTTON_HEIGHT_PIXELS); // 16-bit, width x height pixels
  GFXcanvas16 *label_canvas; // 16-bit, width-4 x height-4 pixels
};

#define EDIT_WIDTH_PIXELS   64
#define EDIT_HEIGHT_PIXELS  32
#define EDIT_MAX_TEXT       WINDW_MAX_LINE
#define CURSOR_BLINK_COUNT  5

struct edit_s {
  struct windw_s windw;
  char text_str[EDIT_MAX_TEXT];
  int text_max_chars;
  int cursor_idx;
  bool cursor_blink_on;
  int  cursor_blink_count;
};

#define LIST_MAX_TEXT   WINDW_MAX_LINE
#define LIST_MAX_ITEMS  100
// TODO: implement
struct list_s {
  struct windw_s windw;
  int scroll_idx;
  int item_count;
  char text_str[LIST_MAX_ITEMS][LIST_MAX_TEXT];
};

enum ctrl_type_e {
  ctrl_type_none = 0,
  ctrl_windw,
  ctrl_cap_windw,
  ctrl_cap_twindw,
  ctrl_button,
  ctrl_up_down, // TODO
  ctrl_edit,
  ctrl_scroll,  // TODO
  ctrl_list,    // TODO
  ctrl_type_count,
};

struct tab_ctrl_s {
  int   ctrl_type; // enum ctrl_type_e
  void *ctrl_obj;
};

#define SCREEN_MAX_CTRLS 10
struct ctrl_screen_s {
  struct tab_ctrl_s screen_ctrls[SCREEN_MAX_CTRLS];
  int               focus_idx;
};

// ----------------------- screen functions: change focus to new control ----------------------------

// Change focus to *next* control in screen structure.
//  If the last control had focus, wrap (change focus to the first control).
void ScreenTabNext(struct ctrl_screen_s *scrn) {
  int ctrl_idx;
  struct tab_ctrl_s *focus_old;
  struct tab_ctrl_s *focus_new;

  focus_old = &scrn->screen_ctrls[scrn->focus_idx];
  DrawCtrl (focus_old, /*has_focus*/false);

  ctrl_idx = 0;
  do {
    if (scrn->screen_ctrls[ctrl_idx].ctrl_type == ctrl_type_none) {
      scrn->focus_idx = 0; // go back to the first control
      break;
    }
    if (ctrl_idx == scrn->focus_idx + 1) {
      scrn->focus_idx = ctrl_idx; // we can go to the next one
      break;
    }
    ctrl_idx++;
  } while (1);

  focus_new = &scrn->screen_ctrls[scrn->focus_idx];
  DrawCtrl (focus_new, /*has_focus*/true);
}

// Change focus to *previous* control in screen structure.
//  If the first control had focus, wrap (change focus to the last control).
void ScreenTabPrev(struct ctrl_screen_s *scrn) {
  struct tab_ctrl_s *focus_old;
  struct tab_ctrl_s *focus_new;

  focus_old = &scrn->screen_ctrls[scrn->focus_idx];
  DrawCtrl (focus_old, /*has_focus*/false);

  if (scrn->focus_idx > 0) {
    scrn->focus_idx--;
  } else { // scrn->focus_idx <= 0. We are at the first control in the screen, going backwards.
    int ctrl_idx;

    ctrl_idx = 0;
    while (scrn->screen_ctrls[ctrl_idx].ctrl_type != ctrl_type_none) {
      ctrl_idx++;
    };
    scrn->focus_idx = ctrl_idx-1; // go to the last control
  }

  focus_new = &scrn->screen_ctrls[scrn->focus_idx];
  DrawCtrl (focus_new, /*has_focus*/true);
}

// On touching the screen:
// 1. Change focus to control at that location.
// 2. Click that control.
bool  do_click  = false;
bool was_click  = false;
void ScreenTouch(struct ctrl_screen_s *scrn, int touch_x, int touch_y) {
  int old_idx;
  int ctrl_idx;
  struct tab_ctrl_s *chk_ctrl;
  struct windw_s *chk_windw;
  struct tab_ctrl_s *focus_old;
  struct tab_ctrl_s *focus_new = NULL;

  old_idx = scrn->focus_idx;
  ctrl_idx = 0;
  do {
    if (scrn->screen_ctrls[ctrl_idx].ctrl_type == ctrl_type_none) {
      return; // got to the end of the list, found no control being touched
    }

    chk_ctrl = &scrn->screen_ctrls[ctrl_idx];
    chk_windw = GetCtrlWindow(chk_ctrl); // Get X and Y of control
    if (chk_windw != NULL) {
      if (touch_x >= chk_windw->wx &&
          touch_x <= chk_windw->wx+chk_windw->ww &&
          touch_y >= chk_windw->wy &&
          touch_y <= chk_windw->wy+chk_windw->wh) {
        // touch is within bounds of window
        scrn->focus_idx = ctrl_idx;
        do_click  = true;
        //was_click = true;
        break;
      }
    }
    ctrl_idx++;
  } while (1);

  if (scrn->focus_idx != old_idx) {
    focus_old = &scrn->screen_ctrls[old_idx];
    DrawCtrl (focus_old, /*has_focus*/false);

    focus_new = &scrn->screen_ctrls[scrn->focus_idx];
    DrawCtrl (focus_new, /*has_focus*/true);
  }
}

bool gamepad_found;
bool haptic_found;

// ---------------------------- Define windows + controls. --------------------------
struct windw_s wnd_main;
//struct cap_windw_s cwnd_main;
//struct cap_twindw_s ctwnd_main;
//struct button_s btn_ok;
struct edit_s edit_text;
struct button_s btn_song1;
struct button_s btn_song2;
//struct button_s btn_song3;

struct ctrl_screen_s main_screen = {
  {
    { ctrl_edit,   &edit_text, },
    { ctrl_button, &btn_song1, },
    { ctrl_button, &btn_song2, },
    { ctrl_type_none,  NULL, },
  },
  0, // focus_idx
};

struct ctrl_screen_s *cur_screen = &main_screen; // initial screen at start

// ------------------ setup ------------------
void setup(void) {
  //Serial.begin(9600);
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Hello! Welcome to windw serial port!"));

  // Start TFT display
#ifdef ESP32_S3_TFT
  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  //pinMode(TFT_BACKLIGHT, OUTPUT);
  //digitalWrite(TFT_BACKLIGHT, HIGH); // Backlight on; for CircuitPlayground Express TFT Gizmo

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  tft.init(PIXELS_Y, PIXELS_X);                // Init ST7789 ST7789 240x135 on ESP32-S3 TFT
#endif

  // ----------------------- start touchscreen input ----------------------------
#ifdef HX8357
  #if defined(_ADAFRUIT_STMPE610H_)
    if (!ts.begin()) {
      Serial.println("Couldn't start STMPE touchscreen controller");
      while (1) delay(100);
    }
    touch_minx = STMPE_TS_MINX; touch_maxx = STMPE_TS_MAXX;
    touch_miny = STMPE_TS_MINY; touch_maxy = STMPE_TS_MAXY;
  #else
    if (! ts.begin(0x48, &Wire)) {
      Serial.println("Couldn't start TSC2007 touchscreen controller");
      while (1) delay(100);
    }
    touch_minx = TSC_TS_MINX; touch_maxx = TSC_TS_MAXX;
    touch_miny = TSC_TS_MINY; touch_maxy = TSC_TS_MAXY;
    pinMode(TSC_IRQ, INPUT);
  #endif
  Serial.println("Touchscreen started");

  tft.begin();

  // read TFT display diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(HX8357_RDPOWMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDCOLMOD);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDIM);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(HX8357_RDDSDR);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
#endif
  tft.setRotation(3);//tft.setRotation(2);
  tft.fillScreen(color_backgr);

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(color_backgr);
  time = millis() - time;

  Wire.begin();        // join i2c bus (address optional for master)

  // ------------------------ Start mini gamepad input ---------------------------
  gamepad_found = ss.begin(0x50);
  if (!gamepad_found) {
    Serial.println("seesaw not found - no gamepad will be used.");
  } else { // gamepad_found
    Serial.println("gamepad started");

    uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
    if (version != 5743) {
      Serial.print("Wrong firmware loaded? ");
      Serial.println(version);
      while(1) delay(10);
    }
    Serial.println("Found Product 5743");
    
    ss.pinModeBulk(button_mask, INPUT_PULLUP);
    ss.setGPIOInterrupts(button_mask, 1);

  #if defined(IRQ_PIN)
    pinMode(IRQ_PIN, INPUT);
  #endif
  }
  
  Serial.println(time, DEC);
  tft.fillScreen(color_backgr);

  // --------------------------------- Start haptic output ----------------------------
  haptic_found = drv.begin();
  if (!haptic_found) {
    Serial.println("DRV2605 not found - no haptic feedback will be used.");
  } else { // haptic_found
    drv.selectLibrary(1);
  
    // I2C trigger by sending 'go' command 
    // default, internal trigger when sending GO command
    drv.setMode(DRV2605_MODE_INTTRIG); 
  }

  // ------------------------  Set up on-board digital input button -------------------------
  pinMode     (PIN_BOOT_BUTTON, INPUT_PULLUP);
  //digitalWrite(PIN_BOOT_BUTTON, HIGH); // enable pull-up

  // ---------------------------- Set up screen controls -----------------------------------
  SetupWindow (&wnd_main, /*x0*/0, /*y0*/0, tft.width()-1, tft.height()-1, /*border*/1, /*buffered*/0);
  DrawWindow (&wnd_main);
  //SetupCapWindow (&cwnd_main, /*x0*/0, /*y0*/0, tft.width()-1, tft.height()-1, /*border*/1, /*buffered*/0, "MP3 Jukebox");
  //DrawCapWindow (&cwnd_main);
  //SetupCapTWindow (&ctwnd_main, "MP3 Jukebox", "radio_gaga.mp3", "top_gun_theme.mp3");
  //SetupCapTWindow (&ctwnd_main, "MP3 Jukebox", "", "");
  //DrawCapTWindow (&ctwnd_main);

  //SetupButton (&btn_ok, /*x*/(tft.width()/2)-BUTTON_WIDTH_PIXELS/2, /*y*/tft.height()-BUTTON_HEIGHT_PIXELS-6, "OK");
  //DrawButton (&btn_ok);
  SetupEdit (&edit_text, /*x*/20, /*y*/12, /*text_max_chars*/16, "My Radio");
  DrawEdit (&edit_text, /*has_focus*/1);
  
  //SetupButton (&btn_song1, /*x*/20, /*y*/tft.height()-BUTTON_HEIGHT_PIXELS*2-18, "radio_gaga.mp3");
  SetupButton (&btn_song1, /*x*/20, /*y*/12+EDIT_HEIGHT_PIXELS+6, "radio_gaga.mp3");
  DrawButton (&btn_song1, /*has_focus*/0);
  //SetupButton (&btn_song2, /*x*/20, /*y*/tft.height()-BUTTON_HEIGHT_PIXELS-12, "top_gun.mp3");
  SetupButton (&btn_song2, /*x*/20, /*y*/12+EDIT_HEIGHT_PIXELS+6+BUTTON_HEIGHT_PIXELS+6, "top_gun.mp3");
  DrawButton (&btn_song2, /*has_focus*/0);
  //SetupButton (&btn_song3, /*x*/10, /*y*/tft.height()-BUTTON_HEIGHT_PIXELS*2-12, "starships.mp3");
  //DrawButton (&btn_song3, /*has_focus*/1);
}

// ---------------------- CardKB mini keyboard constants ------------------
#define KB_ADDR 0x5f
#define MIN_PRINTABLE ' '
#define MAX_PRINTABLE '~'

// When you press Fn+A thru Fn+Z on cardkb,
//  the cardkb will return these values.
#define FUNC_KEY_COUNT ('Z'-'A'+1)
unsigned func_keys[FUNC_KEY_COUNT] =  // Fn+
{	0x9a, 0xaa, 0xa8, 0x9c, 0x8f, 0x9d, // abc def
	0x9e, 0x9f, 0x94, 0xa0, 0xa1, 0xa2, // ghi jkl
	0xac, 0xab, 0x95, 0x96, 0x8d, 0x90, // mno pqr
	0x9b, 0x91, 0x93, 0xa9, 0x8e, 0xa7, // stu vwx
	0x92, 0xa6 }; // yz
#define OKALT      '\n'
#define CANCELALT  0x1b
#define KEY_BACKSPACE 0x8
#define KEY_LEFT  0xb4 // <--
#define KEY_UP    0xb5 // ^
#define KEY_DOWN  0xb6 // v
#define KEY_RIGHT 0xb7 //  -->

// -------------------------------- loop --------------------------------

int last_x = 0, last_y = 0; // old joystick position
void loop() {
  int x_move;
  int y_move;
  static int  y_old = 0;
  char        kb_in = 0; // no key pressed
  bool        do_unclick = false;

  // do_click can be set by touch, kb_in Enter press, or digital input button.
  do_click = false;

  // --------------------- Check touchscreen input -------------------------
#if defined(TSC_IRQ)
  if (!digitalRead(TSC_IRQ)) {
    // IRQ pin is low, we can read from touchscreen!

    TS_Point p_raw = ts.getPoint();
    int touch_x, touch_y;

    Serial.print("X = "); Serial.print(p_raw.x);
    Serial.print("\tY = "); Serial.print(p_raw.y);
    Serial.print("\tPressure = "); Serial.print(p_raw.z);
    if (((p_raw.x == 0) && (p_raw.y == 0)) || (p_raw.z < 10)) return; // no pressure, no touch
  
    // Scale from ~0->4000 to tft.width using the calibration #'s
    //OK with some TFT rotations:
    //p.x = map(p.x, touch_minx, touch_maxx, 0, tft.width());
    //p.y = map(p.y, touch_miny, touch_maxy, 0, tft.height());
    //Serial.print(" -> "); Serial.print(p.x); Serial.print(", "); Serial.println(p.y);
    //our TFT rotation:
    touch_x = map(p_raw.y, touch_miny, touch_maxy, tft.width(), 0); // screen x: raw y scaled, and left-right mirrored
    touch_y = map(p_raw.x, touch_minx, touch_maxx, 0, tft.height());  // screen y: raw x scaled
    Serial.print(" -> "); Serial.print(touch_x); Serial.print(", "); Serial.println(touch_y);

    ScreenTouch(cur_screen, touch_x, touch_y);
  }
#endif

   // ----------------------  Check CardKB mini keyboard input --------------------------
  Wire.requestFrom(KB_ADDR, 1);    // request 1 byte from peripheral device KB_ADDR

  x_move = 0;
  y_move = 0;

  if (Wire.available()) { // peripheral may send less than requested
    kb_in = Wire.read(); // receive a byte as character
    switch (kb_in) {
    case 0x00: // nothing pressed. do nothing.
      break;
    case 0xd: // Enter
      kb_in = '\n';
      do_click = true;
      //was_click = true;
      break;
    case 0x1b: // Escape
      break;
    case KEY_BACKSPACE: // Backspace
      break;
    case 0x9: // Tab
      break;
    case KEY_LEFT: // <- Left
      x_move = -1;
      break;
    case KEY_UP: // ^ Up
      y_move = -1;
      break;
    case KEY_DOWN: // v Down
      y_move = 1;
      break;
    case KEY_RIGHT: // Right ->
      x_move = 1;
      break;
    default:
      if (kb_in >= MIN_PRINTABLE && kb_in <= MAX_PRINTABLE) {
        // A-Z, 1-9, punctuation, space
      } else {
        // Someone probably pressed "Function" + key, got 0x80 thru 0xAF
        // See https://docs.m5stack.com/en/unit/cardkb_1.1
      }
    }
    //Serial.print(kb_in);         // print the character
  }

#ifdef DEBUG_BUTTONS
  Serial.print("Left Button: ");
  if (left_btn) {
    Serial.print("DOWN");
  } else {
    Serial.print("  UP");
  }
  Serial.print("   Right Button: ");
  if (right_btn) {
    Serial.print("DOWN");
  } else {
    Serial.print("  UP");    
  }
  Serial.println();
#endif

  // ------------------------ Check mini gamepad input ---------------------------
  if (gamepad_found) {
    // Reverse x/y values to match joystick orientation
    int joy_x = 1023 - ss.analogRead(14);
    int joy_y = 1023 - ss.analogRead(15);
    
    if ( (abs(joy_x - last_x) > 3)  ||  (abs(joy_y - last_y) > 3)) {
      Serial.print("joy_x: "); Serial.print(joy_x); Serial.print(", "); Serial.print("joy_y: "); Serial.println(joy_y);
      last_x = joy_x;
      last_y = joy_y;
    }
    // X joystick axis: be more sensitive, as it is harder to push
    if (joy_x < 400) {
      x_move = -1;
    }
    if (joy_x > 600) {
      x_move = 1;
    }
    // Y joystick axis: be less sensitive, as it is so easy to push (don't override X motions)
    if (joy_y < 128) {
      y_move = 1;
    }
    if (joy_y > 900) {
      y_move = -1;
    }
#if defined(IRQ_PIN)
    if(!digitalRead(IRQ_PIN)) {
      return;
    }
#endif

    uint32_t buttons = ss.digitalReadBulk(button_mask);
    if (! (buttons & (1UL << BUTTON_X))) {
      y_move = -1;
    }
    if (! (buttons & (1UL << BUTTON_B))) {
      y_move = 1;
    }
    if (! (buttons & (1UL << BUTTON_Y))) {
      x_move = -1;
    }
    if (! (buttons & (1UL << BUTTON_A))) {
      x_move = 1;
    }
  } // end if(gamepad_found)

  // move the direction that was tilted the fastest,
  //  or if none tilted fast, the way we are most tilted.
  if (y_move > 0 && y_old == 0) {
    ScreenTabNext(cur_screen);
  }
  if (y_move < 0 && y_old == 0) {
    ScreenTabPrev(cur_screen);
  }
  if (x_move > 0) {
    do_click = true;
    //was_click = true;
  }
  if (was_click && !do_click) {
    do_unclick = true;
  }

  // ------------------------ Check onboard digital input button ---------------------------
  int btn_in = digitalRead(PIN_BOOT_BUTTON);
  static int old_btn_in = 1;

  if (!btn_in && old_btn_in) { // b/c pull-up
    do_click = true;
  }
  if (btn_in && !old_btn_in) { // un-click
    do_unclick = true;
  }

  // ------------------------- Send input to control that has focus --------------------------
  struct tab_ctrl_s *focus_ctrl;
  focus_ctrl = &cur_screen->screen_ctrls[cur_screen->focus_idx];

  if (do_click) {
    OnClickCtrl (focus_ctrl);
  }
  if (do_unclick) {
    haptic_click ();
    DrawCtrl (focus_ctrl, /*has_focus*/true);
    was_click = false;
  }

  UpdateCtrl (focus_ctrl); // e.g. for cursor blink, etc.
  if (kb_in != 0) {
    OnKeyCtrl (focus_ctrl, kb_in);
  }
  old_btn_in = btn_in;
  y_old = y_move;

  delay(100);
}

// ---------------- make a haptic rumble "click" ------------
void haptic_click(void) {
  if (!haptic_found) {
    return;
  }
  int effect = 1; // Strong Click - 100%

  // set the effect to play
  drv.setWaveform(0, effect);  // play effect 
  drv.setWaveform(1, 0);       // end waveform

  // play the effect!
  drv.go();
}

// ----------------------- draw Dashed lines -----------------------------

#define DOT_LINE_LEN 10
#define DOT_SPACE_LEN 3
#define FOCUS_MARGIN_PIXELS 4

// to TFT
#ifdef ESP32_S3_TFT
  void DrawDashedHLine(Adafruit_ST7789 *scrn, int line_x, int line_y, int line_w) {
#endif
#ifdef HX8357
  void DrawDashedHLine(Adafruit_HX8357 *scrn, int line_x, int line_y, int line_w) {
#endif
    // top and left sides
    int start_px, end_px;
    int width_px;
    int limit_px;

    start_px = line_x;
    limit_px = line_x + line_w;

    width_px = DOT_LINE_LEN;
    while (start_px < limit_px) {
      end_px = start_px + width_px;
      if (end_px > limit_px) {
        end_px = limit_px;
        width_px = end_px - limit_px;
      }

      scrn->drawFastHLine(start_px, line_y, width_px, color_focus_border);
      start_px += DOT_LINE_LEN + DOT_SPACE_LEN;
    }
}

#ifdef ESP32_S3_TFT
  void DrawDashedVLine(Adafruit_ST7789 *scrn, int line_x, int line_y, int line_w) {
#endif
#ifdef HX8357
  void DrawDashedVLine(Adafruit_HX8357 *scrn, int line_x, int line_y, int line_w) {
#endif
    // top and left sides
    int start_py, end_py;
    int height_py;
    int limit_py;

    start_py = line_y;
    limit_py = line_y + line_w;

    height_py = DOT_LINE_LEN;
    while (start_py < limit_py) {
      end_py = start_py + height_py;
      if (end_py > limit_py) {
        end_py = limit_py;
        height_py = end_py - limit_py;
      }
      scrn->drawFastVLine(line_x, start_py, height_py, color_focus_border);
      start_py += DOT_LINE_LEN + DOT_SPACE_LEN;
    }
}

// to Canvas
void DrawDashedHLineToCanvas(GFXcanvas16 *canvas, int line_x, int line_y, int line_w) {
    // top and left sides
    int start_px, end_px;
    int width_px;
    int limit_px;

    start_px = line_x;
    limit_px = line_x + line_w;

    width_px = DOT_LINE_LEN;
    while (start_px < limit_px) {
      end_px = start_px + width_px;
      if (end_px > limit_px) {
        end_px = limit_px;
        width_px = end_px - limit_px;
      }
      canvas->drawFastHLine(start_px, line_y, width_px, color_focus_border);
      start_px += DOT_LINE_LEN + DOT_SPACE_LEN;
    }
}

void DrawDashedVLineToCanvas(GFXcanvas16 *canvas, int line_x, int line_y, int line_w) {
    // top and left sides
    int start_py, end_py;
    int height_py;
    int limit_py;

    start_py = line_y;
    limit_py = line_y + line_w;

    height_py = DOT_LINE_LEN;
    while (start_py < limit_py) {
      end_py = start_py + height_py;
      if (end_py > limit_py) {
        end_py = limit_py;
        height_py = end_py - limit_py;
      }
      canvas->drawFastVLine(line_x, start_py, height_py, color_focus_border);
      start_py += DOT_LINE_LEN + DOT_SPACE_LEN;
    }
}

// Adapted from: Borland C++ Power Programming by Clayton Walnum
// https://fabiensanglard.net/reverse_engineering_strike_commander/docs/Borland_C___Power_Programming_Book_and_Disk__Programming_.pdf
// pg. 129 (PDF pg. 143)

// ------------------- draw Border ------------------------

#ifdef ESP32_S3_TFT
  void DrawBorder(Adafruit_ST7789 *scrn, struct windw_s *wnd, int brd_style) {
#endif
#ifdef HX8357
  void DrawBorder(Adafruit_HX8357 *scrn, struct windw_s *wnd, int brd_style) {
#endif
  int bx, by, bw, bh;

  bx = wnd->wx;
  by = wnd->wy;
  bw = wnd->ww;
  bh = wnd->wh;
  
  switch (brd_style) {
  case border_outset:
    // highlight - top and left sides, 2 pixels wide each
    scrn->drawFastHLine(bx,   by,   bw,   color_highlight); //  top edge - outer
    scrn->drawFastVLine(bx,   by,   bh,   color_highlight); // left edge - outer
    scrn->drawFastHLine(bx+1, by+1, bw-2, color_highlight); //  top edge - inner
    scrn->drawFastVLine(bx+1, by+1, bh-2, color_highlight); // left edge - innder

    // shadow - bottom and right sides, 2 pixels wide each
    scrn->drawFastHLine(bx+1,    by+bh-1, bw-1, color_shadow); // bottom edge - outer
    scrn->drawFastVLine(bx+bw-1, by,      bh,   color_shadow); //  right edge - outer
    scrn->drawFastHLine(bx+2,    by+bh-2, bw-3, color_shadow); // bottom edge - inner
    scrn->drawFastVLine(bx+bw-2, by+1,    bh-2, color_shadow); //  right edge - inner
    break;
  case border_ridge:
    scrn->drawFastHLine(bx+10,      by+10,      bw-20, color_shadow); //  top margin line
    scrn->drawFastVLine(bx+10,      by+10,      bh-20, color_shadow); // left margin line
    scrn->drawFastHLine(bx+10,      by+bh - 10, bw-20, color_highlight); // bottom margin line    
    scrn->drawFastVLine(bx+bw - 10, 10,         bh-20, color_highlight); //   left margin line
    break;
  case border_outset_narrow: // Draw caption bar
    scrn->drawFastVLine(bx + 20,      by+20, 20,      color_highlight);  //   left edge
    scrn->drawFastHLine(bx + 20,      by+20, bw - 40, color_highlight);  //    top edge
    scrn->drawFastVLine(bx + bw - 20, by+20, 21,      color_cap_shadow); //  right edge
    scrn->drawFastHLine(bx + 20,      by+40, bw - 40, color_cap_shadow); // bottom edge
    break;
  case border_inset:
    // shadow - top and left sides, 2 pixels wide each
    scrn->drawFastHLine(bx,   by,   bw-1, color_shadow); //  top edge - outer
    scrn->drawFastVLine(bx,   by,   bh-1, color_shadow); // left edge - outer
    scrn->drawFastHLine(bx+1, by+1, bw-3, color_shadow); //  top edge - inner
    scrn->drawFastVLine(bx+1, by+1, bh-3, color_shadow); // left edge - innder

    // highlight - bottom and right sides, 2 pixels wide each
    scrn->drawFastHLine(bx+1,      by + bh-1, bw-1, color_highlight); // bottom edge - outer
    scrn->drawFastVLine(bx + bw-1, by,        bh-1, color_highlight); //  right edge - outer
    scrn->drawFastHLine(bx+2,      by + bh-2, bw-3, color_highlight); // bottom edge - inner
    scrn->drawFastVLine(bx + bw-2, by+1,      bh-3, color_highlight); //  right edge - inner
    break;
  case border_focus:
    // for some reason using the scrn parameter does not work here?
    DrawDashedHLine(scrn, bx+FOCUS_MARGIN_PIXELS, by+FOCUS_MARGIN_PIXELS,      bw-FOCUS_MARGIN_PIXELS*2); // top edge outer
    DrawDashedHLine(scrn, bx+FOCUS_MARGIN_PIXELS, by+FOCUS_MARGIN_PIXELS+1,    bw-FOCUS_MARGIN_PIXELS*2); // top edge inner
    DrawDashedHLine(scrn, bx+FOCUS_MARGIN_PIXELS, by+bh-FOCUS_MARGIN_PIXELS-1, bw-FOCUS_MARGIN_PIXELS*2); // bottom edge inner
    DrawDashedHLine(scrn, bx+FOCUS_MARGIN_PIXELS, by+bh-FOCUS_MARGIN_PIXELS,   bw-FOCUS_MARGIN_PIXELS*2); // bottom edge outer

    DrawDashedVLine(scrn, bx+FOCUS_MARGIN_PIXELS,      by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // left edge outer
    DrawDashedVLine(scrn, bx+FOCUS_MARGIN_PIXELS+1,    by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // left edge inner
    DrawDashedVLine(scrn, bx+bw-FOCUS_MARGIN_PIXELS-1, by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // right edge inner
    DrawDashedVLine(scrn, bx+bw-FOCUS_MARGIN_PIXELS,   by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // right edge outer
    break;
  default:
    break;
  }
}

void DrawBorderToCanvas(GFXcanvas16 *canvas, struct windw_s *wnd, int brd_style) {
  int bx, by, bw, bh;
  
  bx = 0;
  by = 0;
  bw = wnd->ww;
  bh = wnd->wh;

  switch (brd_style) {
  case border_outset:
    // highlight - top and left sides, 2 pixels wide each
    canvas->drawFastHLine(bx,   by,   bw,   color_highlight); //  top edge - outer
    canvas->drawFastVLine(bx,   by,   bh,   color_highlight); // left edge - outer
    canvas->drawFastHLine(bx+1, by+1, bw-2, color_highlight); //  top edge - inner
    canvas->drawFastVLine(bx+1, by+1, bh-2, color_highlight); // left edge - innder

    // shadow - bottom and right sides, 2 pixels wide each
    canvas->drawFastHLine(bx+1,    by+bh-1, bw-1, color_shadow); // bottom edge - outer
    canvas->drawFastVLine(bx+bw-1, by,      bh,   color_shadow); //  right edge - outer
    canvas->drawFastHLine(bx+2,    by+bh-2, bw-3, color_shadow); // bottom edge - inner
    canvas->drawFastVLine(bx+bw-2, by+1,    bh-2, color_shadow); //  right edge - inner
    break;
  case border_ridge:
    canvas->drawFastHLine(bx+10,      by+10,      bw-20, color_shadow); //  top margin line
    canvas->drawFastVLine(bx+10,      by+10,      bh-20, color_shadow); // left margin line
    canvas->drawFastHLine(bx+10,      by+bh - 10, bw-20, color_highlight); // bottom margin line    
    canvas->drawFastVLine(bx+bw - 10, 10,         bh-20, color_highlight); //   left margin line
    break;
  case border_outset_narrow: // Draw caption bar
    canvas->drawFastVLine(bx + 20,      by+20, 20,      color_highlight);  //   left edge
    canvas->drawFastHLine(bx + 20,      by+20, bw - 40, color_highlight);  //    top edge
    canvas->drawFastVLine(bx + bw - 20, by+20, 21,      color_cap_shadow); //  right edge
    canvas->drawFastHLine(bx + 20,      by+40, bw - 40, color_cap_shadow); // bottom edge
    break;
  case border_inset:
    // shadow - top and left sides, 2 pixels wide each
    canvas->drawFastHLine(bx,   by,   bw-1, color_shadow); //  top edge - outer
    canvas->drawFastVLine(bx,   by,   bh-1, color_shadow); // left edge - outer
    canvas->drawFastHLine(bx+1, by+1, bw-3, color_shadow); //  top edge - inner
    canvas->drawFastVLine(bx+1, by+1, bh-3, color_shadow); // left edge - innder

    // highlight - bottom and right sides, 2 pixels wide each
    canvas->drawFastHLine(bx+1,      by + bh-1, bw-1, color_highlight); // bottom edge - outer
    canvas->drawFastVLine(bx + bw-1, by,        bh-1, color_highlight); //  right edge - outer
    canvas->drawFastHLine(bx+2,      by + bh-2, bw-3, color_highlight); // bottom edge - inner
    canvas->drawFastVLine(bx + bw-2, by+1,      bh-3, color_highlight); //  right edge - inner
    break;
  case border_focus:
    DrawDashedHLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS, by+FOCUS_MARGIN_PIXELS,      bw-FOCUS_MARGIN_PIXELS*2); // top edge outer
    DrawDashedHLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS, by+FOCUS_MARGIN_PIXELS+1,    bw-FOCUS_MARGIN_PIXELS*2); // top edge inner
    DrawDashedHLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS, by+bh-FOCUS_MARGIN_PIXELS-1, bw-FOCUS_MARGIN_PIXELS*2); // bottom edge inner
    DrawDashedHLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS, by+bh-FOCUS_MARGIN_PIXELS,   bw-FOCUS_MARGIN_PIXELS*2); // bottom edge outer

    DrawDashedVLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS,      by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // left edge outer
    DrawDashedVLineToCanvas(canvas, bx+FOCUS_MARGIN_PIXELS+1,    by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // left edge inner
    DrawDashedVLineToCanvas(canvas, bx+bw-FOCUS_MARGIN_PIXELS-1, by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // right edge inner
    DrawDashedVLineToCanvas(canvas, bx+bw-FOCUS_MARGIN_PIXELS,   by+FOCUS_MARGIN_PIXELS, bh-FOCUS_MARGIN_PIXELS*2); // right edge outer
    break;
  default:
    break;
  }
}

// ---------------------- basic Window  --------------------

void DrawWindowToCanvas(struct windw_s *wnd, GFXcanvas16 *canvas) {
  // in buffer, origin is (0,0)
  // on screen, origin is (wx,wy)
  DrawBorderToCanvas (canvas, wnd, border_outset);

  // center
  canvas->fillRect(2, 2, wnd->ww-4, wnd->wh-4, color_window);

  //Draw border, if requested.
  if (wnd->border_style == border_ridge) {
    DrawBorderToCanvas (canvas, wnd, border_ridge);
  }
}

void SetupWindow(struct windw_s *wnd, int x, int y, int w, int h, int brd_style, int buf) {
  wnd->wx = x;
  wnd->wy = y;
  wnd->ww = w;
  wnd->wh = h;
  wnd->border_style = brd_style;
  wnd->buffered = buf;
  Serial.print("SetupWindow(x=");
  Serial.print(x);
  Serial.print(", y=");
  Serial.print(y);
  Serial.print(", w=");
  Serial.print(w);
  Serial.print(", h=");
  Serial.print(h);
  Serial.print(", brd_style=");
  Serial.print(brd_style);
  Serial.print(", buf=");
  Serial.print(buf);
  Serial.println(")");
  if (buf) {
    wnd->canvas = new GFXcanvas16(w, h);
    DrawWindowToCanvas (wnd, wnd->canvas);
  } else {
    wnd->canvas = NULL;
  }
}

void DrawWindow(struct windw_s *wnd) {
  //Draw basic 3-D window.

  if (wnd->buffered) {
    tft.drawRGBBitmap(wnd->wx,   wnd->wy, wnd->canvas->getBuffer(), wnd->canvas->width(), wnd->canvas->height());
  } else { // !wnd->buffered
    DrawBorder (&tft, wnd, border_outset);

    // center
    tft.fillRect(wnd->wx+2, wnd->wy+2, wnd->ww-3, wnd->wh-3, color_window);

    //Draw border, if requested.
    if (wnd->border_style == border_ridge) {
      DrawBorder (&tft, wnd, border_ridge);
    }
  }
}



// ---------------------- Captioned Window  --------------------

void SetupCapWindow(struct cap_windw_s *cwnd, int x, int y, int w, int h, int brd, int buf, char *cap) {
  SetupWindow(&cwnd->windw, x, y, w, h, brd, buf);
  if (cap != NULL) {
    strncpy (cwnd->label, cap, WINDW_MAX_CAP);
  } else { // cap == NULL
    cwnd->label[0] = '\x0';
  }
  cwnd->label[WINDW_MAX_CAP-1] = '\x0'; // ensure null term.
}

void DrawCapWindow(struct cap_windw_s *cwnd) {
  struct windw_s *wnd;
  wnd = &cwnd->windw;

  DrawWindow (wnd);

  // Draw caption bar
  DrawBorder (&tft, wnd, border_outset_narrow);  

  // center
  tft.fillRect(wnd->wx+21, wnd->wy+21, wnd->ww-41, 19, color_cap_bar);

  // label
  tft.setTextSize(2);
	int x = (wnd->wx + wnd->ww / 2) - (strlen(cwnd->label) * CHAR_WIDTH_PIXELS / 2);
  tft.setCursor(x, wnd->wy+22);
  tft.setTextColor(color_cap_label);
  tft.setTextWrap(true);
  tft.print(cwnd->label);
}

// ---------------------- Captioned Text Window --------------------
void SetupCapTWindow(struct cap_twindw_s *ctwnd, char *cap, char *ln1, char *ln2) {
  int str_w;
  int wndw_w = 230; // minumum width
  int wndw_x;
  int wndw_y = 164; // default y
  int wndw_h = 150; // default height

  if (cap != NULL) {
    str_w = strlen(cap) * CHAR_WIDTH_PIXELS + 60;
    if (wndw_w < str_w) {
      wndw_w = str_w;
    }
  }
  if (ln1 != NULL) {
    str_w = strlen(ln1) * CHAR_WIDTH_PIXELS + 60;
    if (wndw_w < str_w) {
      wndw_w = str_w;
    }
  }
  if (ln2 != NULL) {
    str_w = strlen(ln2) * CHAR_WIDTH_PIXELS + 60;
    if (wndw_w < str_w) {
      wndw_w = str_w;
    }
  }
  if (wndw_w > tft.width()-1) { // limit to screen bounds
    wndw_w = tft.width()-1;
  }
  wndw_x = (tft.width()/2) - wndw_w/2;

  if (wndw_h > tft.height()-1) {
    wndw_h = tft.height()-1;
  }
  if (wndw_y + wndw_h > tft.height()) {
    wndw_y = (tft.height()/2) - wndw_h/2;
  }

  if (ln1 != NULL) {
    strncpy (ctwnd->line1, ln1, WINDW_MAX_LINE);
  } else { // ln1 == NULL
    ctwnd->line1[0] = '\x0';
  }
  ctwnd->line1[WINDW_MAX_LINE-1] = '\x0'; // ensure null term.

  if (ln2 != NULL) {
    strncpy (ctwnd->line2, ln2, WINDW_MAX_LINE);
  } else { // ln2 == NULL
    ctwnd->line2[0] = '\x0';
  }
  ctwnd->line2[WINDW_MAX_LINE-1] = '\x0';
  
  SetupCapWindow(&ctwnd->cwindw, wndw_x, wndw_y, wndw_w, wndw_h, 0/*border*/, 1/*buffered*/, cap);
}

void DrawCapTWindow(struct cap_twindw_s *ctwnd) {
  struct cap_windw_s *cwnd;
  struct windw_s *wnd;
  cwnd = &ctwnd->cwindw;
  wnd = &cwnd->windw;

  DrawCapWindow (cwnd);
	// Position and draw window body text.
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setTextColor(color_wndw_text);
	int x = (wnd->wx + wnd->ww / 2) - (strlen(ctwnd->line1) * CHAR_WIDTH_PIXELS) / 2;
	if (strlen(ctwnd->line2) == 0) {
    tft.setCursor(x, wnd->wy+68);
    tft.print(ctwnd->line1);
  } else { // strlen(line2) != 0
    tft.setCursor(x, wnd->wy+56);
    tft.print(ctwnd->line1);
	  x = (wnd->wx + wnd->ww / 2) - (strlen(ctwnd->line2) * CHAR_WIDTH_PIXELS) / 2;
    tft.setCursor(x, wnd->wy+71);
    tft.print(ctwnd->line2);
  }
}



// ---------------------- Button functions --------------------

void DrawButtonLabelToCanvas(struct button_s *btn) {
	int pos = -1;                  // position in 'btn->label_str' where '^' was found
	char tlabel[BUTTON_MAX_LABEL]; // copy of 'btn->label_str' with no '^' character
  GFXcanvas16 *lc;
  struct windw_s *wnd;
  wnd = &btn->windw;

  lc = btn->label_canvas;
  lc->fillScreen(color_window); // Clear button label canvas (not tft)

	// Find and remove the ^ character and
	// set the appropriate hot key.
	strncpy(tlabel, btn->label_str, BUTTON_MAX_LABEL);
  tlabel[BUTTON_MAX_LABEL-1] = '\x0';
	for (int i = 0; i < strlen(tlabel); ++i) {
		if (tlabel[i] == '^') {
      char func_letter;
      
			pos = i;
      func_letter = toupper(tlabel[i + 1]);
      if (func_letter >= 'A' && func_letter < 'Z') {
			  btn->hotkey = func_keys[func_letter - 'A'];
      }
			for (int j = i; j < strlen(tlabel); ++j) {
				tlabel[j] = tlabel[j + 1];
      }
		}
	}
	if      (strcmp(tlabel, "OK"    ) == 0) btn->altkey = OKALT;
	else if (strcmp(tlabel, "CANCEL") == 0) btn->altkey = CANCELALT;

	// Center and draw text on button.
  lc->setTextSize(2);
  lc->setTextWrap(true);
  lc->setTextColor(color_btn_text);
  int x = (wnd->ww / 2) - (strlen(tlabel) * CHAR_WIDTH_PIXELS / 2);
  lc->setCursor(x, 8);
  lc->print(tlabel);

	// Underline the hot-key character.
	if (pos >= 0) {
    lc->drawFastHLine(x + pos * CHAR_WIDTH_PIXELS, 18, CHAR_WIDTH_PIXELS-1, color_btn_text);
  }
}

void SetupButton(struct button_s *btn, int x, int y, const char *lbl) {
  int w;

  // make button wider to fit label
  w = strlen(lbl) * CHAR_WIDTH_PIXELS + 12;
  if (w < BUTTON_WIDTH_PIXELS) {
    w = BUTTON_WIDTH_PIXELS;
  }
  //SetupWindow(&btn->windw, x, y, w, BUTTON_HEIGHT_PIXELS/*h*/, 0/*brd*/, 1/*buf*/); // buffer not working on HX8357?
  SetupWindow(&btn->windw, x, y, w, BUTTON_HEIGHT_PIXELS/*h*/, 0/*brd*/, 0/*buf*/);

  if (lbl != NULL) {
    strncpy (btn->label_str, lbl, BUTTON_MAX_LABEL);
  } else { // lbl == NULL
    btn->label_str[0] = '\x0';
  }
  btn->label_str[BUTTON_MAX_LABEL-1] = '\x0'; // ensure null term.

  btn->altkey = 0;
  btn->hotkey = 0;

  btn->label_canvas = new GFXcanvas16(w-4, BUTTON_HEIGHT_PIXELS-4);
  DrawButtonLabelToCanvas (btn);
}

void DrawButton(struct button_s *btn, bool has_focus) {
  struct windw_s *wnd;
  wnd = &btn->windw;

  DrawWindow (wnd);

  if (wnd->buffered) {
    tft.drawRGBBitmap(wnd->wx+2,   wnd->wy+2, btn->label_canvas->getBuffer(),
                                              btn->label_canvas->width(),
                                              btn->label_canvas->height());
  } else { // !wnd->buffered
	  int pos = -1;                  // position in 'btn->label_str' where '^' was found
  	char tlabel[BUTTON_MAX_LABEL]; // copy of 'btn->label_str' with no '^' character

    // Find and remove the ^ character and
    // set the appropriate hot key.
    strncpy(tlabel, btn->label_str, BUTTON_MAX_LABEL);
    tlabel[BUTTON_MAX_LABEL-1] = '\x0';
    for (int i = 0; i < strlen(tlabel); ++i) {
      if (tlabel[i] == '^') {
        char func_letter;
        
        pos = i;
        func_letter = toupper(tlabel[i + 1]);
        if (func_letter >= 'A' && func_letter < 'Z') {
          btn->hotkey = func_keys[func_letter - 'A'];
        }
        for (int j = i; j < strlen(tlabel); ++j) {
          tlabel[j] = tlabel[j + 1];
        }
      }
    }
    if      (strcmp(tlabel, "OK"    ) == 0) btn->altkey = OKALT;
    else if (strcmp(tlabel, "CANCEL") == 0) btn->altkey = CANCELALT;

    // Center and draw text on button.
    tft.setTextSize(2);
    tft.setTextWrap(true);
    tft.setTextColor(color_btn_text);
    int x = (wnd->wx + wnd->ww / 2) - (strlen(tlabel) * CHAR_WIDTH_PIXELS / 2);
    tft.setCursor(x, wnd->wy+12);
    tft.print(tlabel);

    // Underline the hot-key character.
    if (pos >= 0) {
      tft.drawFastHLine(x + pos * CHAR_WIDTH_PIXELS, wnd->wy+20, CHAR_WIDTH_PIXELS-1, color_btn_text);
    }
  } // end else (!wnd->buffered)
  if (has_focus) {
    DrawBorder (&tft, wnd, border_focus);
  }
}

void ClickButton (struct button_s *btn) {
  struct windw_s *wnd;
  wnd = &btn->windw;

  // Shift the image on the button down and right
	// to simulate button movement.
  tft.drawRGBBitmap(wnd->wx+3,   wnd->wy+3, btn->label_canvas->getBuffer(),
                                            btn->label_canvas->width(),
                                            btn->label_canvas->height());

	// Draw the button’s borders so the
	// button appears to be pressed.
  DrawBorder (&tft, wnd, border_inset);
}



// ---------------------- Event to Control functions --------------------

void DrawCtrl(struct tab_ctrl_s *ctrl, bool has_focus) {
  switch (ctrl->ctrl_type) {
  case ctrl_button:
    DrawButton ((struct button_s *)ctrl->ctrl_obj, has_focus);
    break;
  case ctrl_edit:
    DrawEdit ((struct edit_s *)ctrl->ctrl_obj, has_focus);
    break;
  default: // other kind of control.
    break; //  do nothing.
  }
}

void OnKeyCtrl(struct tab_ctrl_s *ctrl, char kb_in) {
  switch (ctrl->ctrl_type) {
  case ctrl_button:
    break;
  case ctrl_edit:
    OnKeyEdit ((struct edit_s *)ctrl->ctrl_obj, kb_in);
    break;
  default: // other kind of control.
    break; //  do nothing.
  }
}

void UpdateCtrl(struct tab_ctrl_s *ctrl) {
  switch (ctrl->ctrl_type) {
  case ctrl_button:
    break;
  case ctrl_edit:
    UpdateEdit ((struct edit_s *)ctrl->ctrl_obj);
    break;
  default: // other kind of control.
    break; //  do nothing.
  }
}

struct windw_s *GetCtrlWindow(struct tab_ctrl_s *ctrl) {
  struct windw_s *ret = NULL;

  switch (ctrl->ctrl_type) {
  case ctrl_windw:
    ret = (struct windw_s *)ctrl->ctrl_obj;
    break;
  case ctrl_cap_windw:
    ret = &((struct cap_windw_s *)ctrl->ctrl_obj)->windw;
    break;
  case ctrl_cap_twindw:
    ret = &((struct cap_twindw_s *)ctrl->ctrl_obj)->cwindw.windw;
    break;
  case ctrl_button:
    ret = &((struct button_s *)ctrl->ctrl_obj)->windw;
    break;
  case ctrl_edit:
    ret = &((struct edit_s *)ctrl->ctrl_obj)->windw;
    break;
  default: // other kind of control.
    break; //  do nothing.
  }
  return ret;
}

void OnClickCtrl(struct tab_ctrl_s *ctrl) {
  if (!was_click) {
    haptic_click ();
  }
  was_click = true;

  switch (ctrl->ctrl_type) {
  case ctrl_button:
    ClickButton ((struct button_s *)ctrl->ctrl_obj);
    break;
  default: // other kind of control.
    break; //  do nothing.
  }
}



// ------------------- Edit control -------------------------

void SetupEdit (struct edit_s *edt, int x, int y, int text_max_chars, const char *text_default) {
  int w;

  if (text_max_chars > EDIT_MAX_TEXT-1) {
    text_max_chars = EDIT_MAX_TEXT-1;
  }
  edt->text_max_chars = text_max_chars;

  // make control wider to fit allowed chars (Alternative: horizontal scrolling)
  w = text_max_chars * CHAR_WIDTH_PIXELS + 15;
  if (w < EDIT_WIDTH_PIXELS) {
    w = EDIT_WIDTH_PIXELS;
  }
  //SetupWindow(&edt->windw, x, y, w, EDIT_HEIGHT_PIXELS/*h*/, 0/*brd*/, 1/*buf*/);
  SetupWindow(&edt->windw, x, y, w, EDIT_HEIGHT_PIXELS/*h*/, 0/*brd*/, 0/*buf*/); // no use for buffering this

  if (text_default != NULL) {
    strncpy (edt->text_str, text_default, EDIT_MAX_TEXT);
  } else { // text_default == NULL
    edt->text_str[0] = '\x0';
  }
  edt->text_str[EDIT_MAX_TEXT-1] = '\x0'; // ensure null term.

  edt->cursor_idx = strlen(edt->text_str);
  edt->cursor_blink_on = false;
  edt->cursor_blink_count = 0;
}

void DrawEdit (struct edit_s *edt, bool has_focus) {
  struct windw_s *wnd;
  wnd = &edt->windw;
  int tx, ty;

  tx = wnd->wx+9;
  ty = wnd->wy+9;

  DrawBorder(&tft, wnd, border_inset);

  tft.fillRect(wnd->wx+2, wnd->wy+2, wnd->ww-4, wnd->wh-4, color_edit_backgr);

  // Draw text in control.
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.setTextColor(color_edit_text);
  tft.setCursor(tx, wnd->wy+9);
  tft.print(edt->text_str);
 
  if (has_focus) {
    DrawBorder(&tft, wnd, border_focus);
  }
}

void DrawEditCursor (struct edit_s *edt) {
  struct windw_s *wnd;
  wnd = &edt->windw;
  int tx, ty;
  uint16_t draw_erase_color; // cursor color or background color

  tx = wnd->wx+9;
  ty = wnd->wy+9;

  if (edt->cursor_blink_on) {
    draw_erase_color = color_edit_cursor; // draw
  } else { // !cursor_blink_on
    draw_erase_color = color_edit_backgr; // erase
  }
  tft.drawFastVLine(tx + edt->cursor_idx*CHAR_WIDTH_PIXELS-2, ty-2, CHAR_HEIGHT_PIXELS+2, draw_erase_color);
  tft.drawFastVLine(tx + edt->cursor_idx*CHAR_WIDTH_PIXELS-1, ty-2, CHAR_HEIGHT_PIXELS+2, draw_erase_color);

  edt->cursor_blink_count = 0; // wait before blinking again
}

void UpdateEdit (struct edit_s *edt) {
  edt->cursor_blink_count++;
  if (edt->cursor_blink_count > CURSOR_BLINK_COUNT) {
    edt->cursor_blink_on = !edt->cursor_blink_on;
    DrawEditCursor (edt);
  }
}

void DebugPrintChars(char *str) {
  int idx;

  idx = 0;
  while (str[idx] != '\x0') {
    Serial.print(idx);
    Serial.print(": '");
    Serial.print(str[idx]);
    Serial.print("', ");
    idx++;
  }
  Serial.print(idx);
  Serial.println(": NULL");
}

void OnKeyEdit (struct edit_s *edt, char kb_in) {
  int text_len;

  text_len = strlen (edt->text_str);
  if (kb_in == KEY_BACKSPACE) {
    if (edt->cursor_idx > 0) {
      //edt->text_str[text_len-1] = '\x0';

      // Move all characters
      //  FROM range: cursor_idx   ... text_len+1
      //    TO range: cursor_idx-1 ... text_len
      int src_idx, dst_idx;
      dst_idx = edt->cursor_idx-1;
      while (dst_idx <= text_len) {
        src_idx = dst_idx+1;
        edt->text_str[dst_idx] = edt->text_str[src_idx];
        dst_idx++;
      }
      edt->cursor_idx--;
      DrawEdit (edt, true);
      edt->cursor_blink_on = true;
      DrawEditCursor (edt);
    }
  }

  // insert a character
  if (kb_in >= MIN_PRINTABLE && kb_in <= MAX_PRINTABLE) {
    if (text_len < edt->text_max_chars) { // yes, enough room for more
      // Move all characters
      //  FROM range: cursor_idx   ... text_len
      //    TO range: cursor_idx+1 ... text_len+1
      int src_idx, dst_idx;
      src_idx = text_len;
      while (src_idx >= edt->cursor_idx) {
        dst_idx = src_idx+1;
        edt->text_str[dst_idx] = edt->text_str[src_idx];
        src_idx--;
      }
      edt->text_str[edt->cursor_idx] = kb_in; // user's character
      edt->cursor_idx++;
      DrawEdit (edt, true);
      edt->cursor_blink_on = true;
      DrawEditCursor (edt);
    }
  }

  if (kb_in == KEY_LEFT) {
    if (edt->cursor_idx > 0) {
      edt->cursor_blink_on = false;
      DrawEditCursor (edt);
      
      edt->cursor_idx--;

      edt->cursor_blink_on = true;
      DrawEditCursor (edt);
    }
  }
  if (kb_in == KEY_RIGHT) {
    if (edt->cursor_idx < text_len) {
      edt->cursor_blink_on = false;
      DrawEditCursor (edt);

      edt->cursor_idx++;

      edt->cursor_blink_on = true;
      DrawEditCursor (edt);
    }
  }
}