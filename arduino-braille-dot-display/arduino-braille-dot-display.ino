// Include libraries for Arduino and MD_MAX7219 chip.
#include <SPI.h>
#include <MD_MAX72xx.h>

// Hardware type for MD_MAX72xx library.
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

// Defining size of dot display and the pins for the dot display.
#define MAX_DEVICES 4
#define CS_PIN 12
#define DIN_PIN 11
#define CLK_PIN 13

// Object for dot display
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DIN_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Button array with pins
const uint8_t button_pin[8] = {9, 8, 7, 6, 5, 4, 3, 2};

// 2D Array for Braille Dot Display
// 0 - Off
// 1 - On
// Binary representation of braille:
// 5 2
// 4 1
// 3 0
uint8_t braille[127];

// Current character position for Braille.
uint8_t current_pos = 0;

// Current mode
// 0 - Arduino standalone input mode
// 1 - Arduino NVDA driver mode
const uint8_t mode = 1;

// 32 byte buffer
byte buf[32];

// array for time last released and button was previous low
bool button_was_low[8];
unsigned long last_button_release_time[8];
unsigned long last_input;
int button_released = -1;

// Initialize braille array with correct binary representations of braille.
void init_braille_array()
{
  for (uint8_t i = 0; i < 32; i++)
  {
    braille[i] = 0b000000;
  }
  braille[33] = 0b011101;
  braille[34] = 0b000010;
  braille[35] = 0b001111;
  braille[36] = 0b110101;
  braille[37] = 0b100101;
  braille[38] = 0b111101;
  braille[39] = 0b001000;
  braille[40] = 0b111011;
  braille[41] = 0b011111;
  braille[42] = 0b100001;
  braille[43] = 0b001101;
  braille[44] = 0b000001;
  braille[45] = 0b001001;
  braille[46] = 0b000101;
  braille[47] = 0b001100;
  braille[48] = 0b001011;
  braille[49] = 0b010000;
  braille[50] = 0b011000;
  braille[51] = 0b010010;
  braille[52] = 0b010011;
  braille[53] = 0b010001;
  braille[54] = 0b011010;
  braille[55] = 0b011011;
  braille[56] = 0b011001;
  braille[57] = 0b001010;
  braille[58] = 0b100011;
  braille[59] = 0b000011;
  braille[60] = 0b110001;
  braille[61] = 0b111111;
  braille[62] = 0b001110;
  braille[63] = 0b100111;
  braille[64] = 0b000100;
  braille[65] = 0b100000;
  braille[66] = 0b110000;
  braille[67] = 0b100100;
  braille[68] = 0b100110;
  braille[69] = 0b100010;
  braille[70] = 0b110100;
  braille[71] = 0b110110;
  braille[72] = 0b110010;
  braille[73] = 0b010100;
  braille[74] = 0b010110;
  braille[75] = 0b101000;
  braille[76] = 0b111000;
  braille[77] = 0b101100;
  braille[78] = 0b101110;
  braille[79] = 0b101010;
  braille[80] = 0b111100;
  braille[81] = 0b111110;
  braille[82] = 0b111010;
  braille[83] = 0b011100;
  braille[84] = 0b011110;
  braille[85] = 0b101001;
  braille[86] = 0b111001;
  braille[87] = 0b010111;
  braille[88] = 0b101101;
  braille[89] = 0b101111;
  braille[90] = 0b101011;
  braille[91] = 0b010101;
  braille[92] = 0b110011;
  braille[93] = 0b110111;
  braille[94] = 0b000110;
  braille[95] = 0b000111;
}

// Output braille character to dot matrix.
void output_braille(char c)
{
  uint8_t braille_char = braille[c];
  uint8_t current_x;
  uint8_t x_reset;
  uint8_t current_y = (2 * current_pos) % 32;
  uint8_t x_count = 0;
  if (current_pos < 16)
  {
    x_reset = 7;
  }
  else
  {
    x_reset = 3;
  }
  current_x = x_reset;
  for (uint8_t i = 6; i > 0; i--)
  {
    mx.setPoint(current_x, current_y, bitRead(braille_char, i - 1));
    x_count++;
    if (x_count == 3)
    {
      current_x = x_reset;
      current_y++;
    }
    else
    {
      current_x--;
    }
  }
  current_pos++;
}

// Output character to dot matrix using NVDA style binary representation of braille.
// 5 2
// 4 1
// 3 0
// 6 7
void output_braille_nvda(uint8_t c)
{
  uint8_t current_x;
  uint8_t x_reset;
  uint8_t current_y = (2 * current_pos) % 32;
  uint8_t x_count = 0;
  uint8_t eight_dot_count = 7;
  if (current_pos < 16)
  {
    x_reset = 7;
  }
  else
  {
    x_reset = 3;
  }
  current_x = x_reset;
  for (uint8_t i = 0; i < 6; i++)
  {
    mx.setPoint(current_x, current_y, bitRead(c, i));
    x_count++;
    if (x_count == 3)
    {
      current_x = x_reset;
      current_y++;
    }
    else
    {
      current_x--;
    }
  }
  // account for 6 and 7 bit now
  current_y = (2 * current_pos) % 32;
  mx.setPoint(x_reset - 3, current_y, bitRead(c, 6));
  mx.setPoint(x_reset - 3, current_y + 1, bitRead(c, 7));
  current_pos++;
}

void backspace()
{
  if (current_pos != 0)
  {
    current_pos--;
    output_braille_nvda(0);
    current_pos--;
  }
}

void new_line()
{
  if (current_pos < 16)
  {
    current_pos = 16;
  }
  else
  {
    mx.clear();
    current_pos = 0;
  }
}

// Print given string in braille.
void print_braille_str(const char *str)
{
  uint8_t i = 0;
  while (str[i] != 0)
  {
    output_braille(str[i]);
    i++;
  }
}

void serial_flush()
{
  while (Serial.available() > 0)
  {
    char t = Serial.read();
  }
}

byte keyboard_rearrange(byte keyboard_input)
{
  byte output = 0;
  if (bitRead(keyboard_input, 1) == 1)
  {
    bitSet(output, 2);
  }
  if (bitRead(keyboard_input, 2) == 1)
  {
    bitSet(output, 1);
  }
  if (bitRead(keyboard_input, 3) == 1)
  {
    bitSet(output, 0);
  }
  if (bitRead(keyboard_input, 4) == 1)
  {
    bitSet(output, 3);
  }
  if (bitRead(keyboard_input, 5) == 1)
  {
    bitSet(output, 4);
  }
  if (bitRead(keyboard_input, 6) == 1)
  {
    bitSet(output, 5);
  }
  return output;
}

void setup()
{
  // Initialize Serial bus.
  Serial.begin(115200);
  Serial.setTimeout(1);
  // Initialize the braille array.
  init_braille_array();
  // Initialize pins for buttons.
  for (int i = 0; i < 8; i++)
  {
    pinMode(button_pin[i], INPUT);
  }
  // Start dot display.
  mx.begin();
}
void loop()
{
  // Check mode first
  if (mode == 1)
  {
    if (Serial.available() >= 32)
    {
      // Output braille binary if 32 bytes found.
      current_pos = 0;
      size_t len = Serial.readBytes(buf, 32);
      for (int i = 0; i < len; i++)
      {
        output_braille_nvda(buf[i]);
      }
      serial_flush();
    }
  }
  // First we update the appropriate arrays
  bool button_pressed = false;
  byte button_input = 0;
  for (int i = 0; i < 8; i++)
  {
    if (digitalRead(button_pin[i]) == LOW)
    {
      button_was_low[i] = true;
    }
    if (digitalRead(button_pin[i]) == HIGH && button_was_low[i])
    {
      button_was_low[i] = false;
      last_button_release_time[i] = millis();
      if (button_released == -1)
      {
        button_released = i;
      }
    }
  }
  if (button_released != -1 && millis() > last_button_release_time[button_released] + 100)
  {
    for (int i = 0; i < 8; i++)
    {
      if (last_button_release_time[i] < last_button_release_time[button_released] + 100 && last_button_release_time[i] > last_button_release_time[button_released] - 100)
      {
        bitSet(button_input, i);
      }
    }
    if (last_input == 0 || millis() - last_input > 250)
    {
      if (mode == 0)
      {
        last_input = millis();
        if (bitRead(button_input, 0) == 1 && bitRead(button_input, 7) == 1)
        {
          output_braille_nvda(0);
        }
        else if (bitRead(button_input, 0) == 1)
        {
          backspace();
        }
        else if (bitRead(button_input, 7) == 1)
        {
          new_line();
        }
        else
        {
          output_braille_nvda(keyboard_rearrange(button_input));
        }
      }
      else
      {
        Serial.write(keyboard_rearrange(button_input));
      }
      button_released = -1;
      button_input = 0;
    }
  }
}
