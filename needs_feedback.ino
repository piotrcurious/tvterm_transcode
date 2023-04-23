
// Arduino code for transcoding serial stream in format of the small TV terminal by Stan Pechal and Vassilis Serasidis
// to VT100 compatible serial stream using VT100 library

// Include VT100 library
#include <VT100.h>

// Define serial baud rate
#define BAUD_RATE 9600

// Create a VT100 object
VT100 vt;

// Define a flag for new line mode
bool nl_mode = false; // false = CR or LF for new line, true = CR and LF for new line

// Define a flag for cursor position mode
bool cp_mode = false; // false = normal mode, true = next byte will specify X or Y position of cursor

// Define a variable for cursor position byte
char cp_byte = '\x00'; // 0x00 = no byte received, 0x13 = next byte will specify X position, 0x14 = next byte will specify Y position

void setup() {
  // Initialize serial communication
  Serial.begin(BAUD_RATE);

  // Initialize VT100 object
  vt.begin(&Serial);

  // Clear screen and move cursor to home position
  vt.clearScreen();
  vt.setCursorPosition(1, 1);
}

void loop() {
  // Check if there is data available on serial port
  if (Serial.available() > 0) {
    // Read one byte from serial port
    char c = Serial.read();

    // Check if cursor position mode flag is true
    if (cp_mode) {
      if (cp_byte == '\x13') { // If previous byte was DC3
        vt.setCursorPosition(c, vt.getCursorY()); // Set cursor X position to current byte and keep Y position unchanged
      } else if (cp_byte == '\x14') { // If previous byte was DC4
        vt.setCursorPosition(vt.getCursorX(), c); // Set cursor Y position to current byte and keep X position unchanged
      }
      cp_mode = false; // Reset cursor position mode flag to false
      cp_byte = '\x00'; // Reset cursor position byte to 0x00
    } else {
      // Check if the byte is a control character for the small TV terminal
      switch (c) {
        case '\x01': // SOH - set mode "CR or LF for new line"
          nl_mode = false; // Set new line mode flag to false
          break;
        case '\x02': // STX - set mode "CR and LF for new line"
          nl_mode = true; // Set new line mode flag to true
          break;
        case '\x08': // BS - backspace
          vt.cursorBackward(1); // Move cursor left one position
          break;
        case '\x09': // HT - horizontal tab - move cursor to position modulo 8
          vt.horizontalTab(); // Move cursor to next tab stop
          break;
        case '\x0A': // LF - line feed (see SOH and STX as well)
          vt.lineFeed(); // Move cursor down one line
          if (nl_mode) { // If new line mode flag is true
            vt.carriageReturn(); // Move cursor to beginning of current line as well
          }
          break;
        case '\x0C': // FF - form feed (clear screen)
          vt.clearScreen(); //
          vt.setCursorPosition(1, 1); // Move cursor to home position
          break;
        case '\x0D': // CR - carriage return (see SOH and STX as well)
          vt.carriageReturn(); // Move cursor to beginning of current line
          if (nl_mode) { // If new line mode flag is true
            vt.lineFeed(); // Move cursor down one line as well
          }
          break;
        case '\x11': // DC1 - cursor ON
          vt.showCursor(); // Show cursor
          break;
        case '\x12': // DC2 - cursor OFF
          vt.hideCursor(); // Hide cursor
          break;
        case '\x13': // DC3 - next byte will specify X position of cursor
          cp_mode = true; // Set cursor position mode flag to true
          cp_byte = c; // Set cursor position byte to current byte
          break;
        case '\x14': // DC4 - next byte will specify Y position of cursor
          cp_mode = true; // Set cursor position mode flag to true
          cp_byte = c; // Set cursor position byte to current byte
          break;
        default: // Any other byte
          vt.print(c); // Print current byte to screen
      }
    }
  }
}

