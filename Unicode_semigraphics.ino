
// Arduino code for transcoding serial stream from small TV terminal format to VT100 format
// Based on information from https://serasidis.gr/circuits/TV_terminal/ 

// Define constants for control characters
const byte SOH = 0x01; // Set mode "CR or LF for new line" (Unix, Apple like)
const byte STX = 0x02; // Set mode "CR and LF for new line" (Microsoft like)
const byte BS = 0x08; // Backspace
const byte TAB = 0x09; // Horizontal TAB - move cursor to position modulo 8
const byte LF = 0x0A; // Line feed (see SOH and STX as well)
const byte FF = 0x0C; // Form feed (clear screen)
const byte CR = 0x0D; // Carriage return (see SOH and STX as well)
const byte DC1 = 0x11; // Cursor ON
const byte DC2 = 0x12; // Cursor OFF
const byte DC3 = 0x13; // Next byte will specify X position of cursor
const byte DC4 = 0x14; // Next byte will specify Y position of cursor

// Define variables for serial stream mode and cursor position
bool crlfMode = false; // True if CR and LF are needed for new line, false if only CR or LF is enough
byte cursorX = 1; // Current X position of cursor (1-40)
byte cursorY = 1; // Current Y position of cursor (1-25)

void setup() {
  // Initialize serial ports
  Serial.begin(9600); // Serial port connected to small TV terminal
  Serial1.begin(9600); // Serial port connected to VT100 terminal
}

void loop() {
  // Check if data is available from small TV terminal
  if (Serial.available() > 0) {
    // Read one byte from serial stream
    byte data = Serial.read();
    // Check if data is a control character or a printable character
    if (data < 32) {
      // Data is a control character, handle it accordingly
      switch (data) {
        case SOH:
          // Set mode to CR or LF for new line
          crlfMode = false;
          break;
        case STX:
          // Set mode to CR and LF for new line
          crlfMode = true;
          break;
        case BS:
          // Move cursor one position back and erase character
          if (cursorX > 1) {
            cursorX--;
            Serial1.print("\x1B[D"); // VT100 escape sequence for cursor left
            Serial1.print(" "); // Erase character with space
            Serial1.print("\x1B[D"); // VT100 escape sequence for cursor left
          }
          break;
        case TAB:
          // Move cursor to next multiple of 8 position
          while (cursorX % 8 != 0 && cursorX < 40) {
            cursorX++;
            Serial1.print(" "); // Fill with spaces until next tab stop
          }
          break;
        case LF:
          // Move cursor to next line
          if (cursorY < 25) {
            cursorY++;
            Serial1.print("\x1B[B"); // VT100 escape sequence for cursor down
          } else {
            Serial1.print("\n"); // Scroll up one line if at bottom of screen
          }
          if (!crlfMode) {
            // Reset cursor to first column if CR is not needed
            cursorX = 1;
            Serial1.print("\r"); 
          }
          break;
        case FF:
          // Clear screen and move cursor to home position
          Serial1.print("\x1B[2J"); // VT100 escape sequence for clear screen
          Serial1.print("\x1B[H"); // VT100 escape sequence for cursor home
          cursorX = 1;
          cursorY = 1;
          break;
        case CR:
          // Move cursor to first column of current line
          Serial1.print("\r");
          cursorX = 1;
          if (crlfMode) {
            // Move cursor to next line if LF is needed
            if (cursorY < 25) {
              cursorY++;
              Serial1.print("\x1B[B"); // VT100 escape sequence for cursor down
            } else {
              Serial1.print("\n"); // Scroll up one line if at bottom of screen
            }
          }
          break;
        case DC1:
          // Turn cursor on
          Serial1.print("\x1B[?25h"); // VT100 escape sequence for show cursor
          break;
        case DC2:
          // Turn cursor off
          Serial1.print("\x1B[?25l"); // VT100 escape sequence for hide cursor
          break;
        case DC3:
          // Set cursor X position
          if (Serial.available() > 0) {
            // Read next byte as X position
            byte x = Serial.read();
            if (x >= 1 && x <= 40) {
              // Valid X position, move cursor to it
              cursorX = x;
              Serial1.print("\x1B["); // VT100 escape sequence for cursor position
              Serial1.print(cursorY); // Current Y position
              Serial1.print(";");
              Serial1.print(cursorX); // New X position
              Serial1.print("H");
            }
          }
          break;
        case DC4:
          // Set cursor Y position
          if (Serial.available() > 0) {
            // Read next byte as Y position
            byte y = Serial.read();
            if (y >= 1 && y <= 25) {
              // Valid Y position, move cursor to it
              cursorY = y;
              Serial1.print("\x1B["); // VT100 escape sequence for cursor position
              Serial1.print(cursorY); // New Y position
              Serial1.print(";");
              Serial1.print(cursorX); // Current X position
              Serial1.print("H");
            }
          }
          break;
      }
    } else {
      // Data is a printable character, check if it is a semigraphics character or not
      if (data >= 128 && data <= 191) {
        // Data is a semigraphics character, convert it to unicode and send it as UTF-8 encoded sequence
        byte unicode = data - 128 + 0xB0; // Convert semigraphics code to unicode code in Symbols for Legacy Computing block 
        byte utf8[3]; // Array to store UTF-8 encoded bytes
        utf8[0] = 0xF0; // First byte of UTF-8 encoding for U+10000 to U+10FFFF range 
        utf8[1] = 0x9F; // Second byte of UTF-8 encoding for U+10000 to U+10FFFF range 
        utf8[2] = unicode; // Third byte of UTF-8 encoding for U+10000 to U+10FFFF range 
        Serial.write(utf8, 3); // Write UTF-8 encoded bytes to VT100 terminal 
      } else {
        // Data is a normal character, write it to VT100 terminal
        Serial.write(data);
      }
      // Update cursor X position and wrap to next line if needed
      cursorX++;
      if (cursorX > 40) {
        cursorX = 1;
        if (cursorY < 25) {
          cursorY++;
          Serial1.print("\x1B[B"); // VT100 escape sequence for cursor down
        } else {
          Serial1.print("\n"); // Scroll up one line if at bottom of screen
}
      }
    }
  }
}
//Source: Conversation with Bing, 4/23/2023
//(1) UTF-8 - Wikipedia. https://en.wikipedia.org/wiki/UTF-8.
//(2) UTF8 Encode/Decode [Online Tool]. https://textool.io/utf8-encode-decode.
//(3) HTML UTF-8 Reference - W3School. https://www.w3schools.com/charsets/ref_html_utf8.asp.
//(4) UTF-8 Encode - Convert Text to UTF-8 - Online - Browserling. https://www.browserling.com/tools/utf8-encode.

//Source: Conversation with Bing, 4/23/2023
//(1) Semigraphics - Wikipedia. https://en.wikipedia.org/wiki/Semigraphics.
//(2) GitHub - mobluse/semigraphics: Functions for semigraphics similar to .... https://github.com/mobluse/semigraphics.
//(3) L2/17- - Unicode. https://www.unicode.org/L2/L2017/17435-terminals-prop.pdf.
