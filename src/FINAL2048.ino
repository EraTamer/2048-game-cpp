#include <TimerOne.h>

#define CLK 11
#define OE 9
#define LAT 10
#define A 54  // Arduino A0 -> PF0
#define B 55  // Arduino A1 -> PF1
#define C 56  // Arduino A2 -> PF2
#define D 57  // Arduino A3 -> PF3

// PORTA mapping:
#define R1 2  // Arduino 24 -> PA2 RED upper half
#define G1 4  // Arduino 26 -> PA3 GREEN upper half
#define B1 3  // Arduino 25 -> PA4 BLUE upper half
#define R2 5  // Arduino 27 -> PA5 RED lower half
#define B2 6  // Arduino 28 -> PA6 GREEN lower half
#define G2 7  // Arduino 29 -> PA7 BLUE lower half

#define CLK_HIGH (PORTB |= (1 << PB5))
#define CLK_LOW (PORTB &= ~(1 << PB5))
#define LAT_HIGH (PORTB |= (1 << PB4))
#define LAT_LOW (PORTB &= ~(1 << PB4))
#define OE_HIGH (PORTH |= (1 << PH6))
#define OE_LOW (PORTH &= ~(1 << PH6))

#define interruptPin18 18
#define interruptPin19 19
#define interruptPin2 2
#define interruptPin3 3

const long debounce_delay = 100;

volatile bool button1pressed = false;
volatile long last_interrupt1 = 0;

volatile bool button2pressed = false;
volatile long last_interrupt2 = 0;

volatile bool button3pressed = false;
volatile long last_interrupt3 = 0;

volatile bool button4pressed = false;
volatile long last_interrupt4 = 0;

volatile int displayingCycle = 0;  // Modulation bit: 0-3
volatile int currentRow = 0;

/*
Color format: 1 byte per pixel. 2 bits per channel (Black, Red, Green, Blue).
Binary position determines intensity/brightness (exponential scaling).
*/

#define BLACK 0    // 0b00000000
#define RED 85     // 0b01010101
#define GREEN 170  // 0b10101010
#define BLUE 255   // 0b11111111
#define BLUE2 0b11111100
#define AQUA 0b10101011
#define PURPLE 0b01010111
#define YELLOW 0b01010110
#define LIGHT_GREEN 0b10100110
#define DARK_GREEN 0b00101010
#define ORANGE 0b01011001
#define WHITE 0b11011001
#define VIOLET 0b11000001

// ---------------- 2048 GAME DATA ----------------
int board[4][4];
int score = 0;

struct TilePos {
  uint8_t row;
  uint8_t col;
};

TilePos tilePos[4][4] = { { { 0, 0 }, { 0, 16 }, { 0, 32 }, { 0, 48 } },
                          { { 8, 0 }, { 8, 16 }, { 8, 32 }, { 8, 48 } },
                          { { 16, 0 }, { 16, 16 }, { 16, 32 }, { 16, 48 } },
                          { { 24, 0 }, { 24, 16 }, { 24, 32 }, { 24, 48 } } };

void init_board();
void add_random_tile();
void draw_board_2048();
void print_board_serial();
bool move_left();
bool move_right();
bool move_up();
bool move_down();

uint8_t grid[32][64] = { 0 };

/*
Buffer: 4 bitplanes. Each holds color info for 2 rows (n and n+16).
Structure: 4 planes x 16 double-rows x 64 columns.
*/
volatile uint8_t buffer[4][16][64] = { 0 };
volatile uint8_t current_row = 0;
volatile uint8_t current_col = 0;

void setup() {
  DDRA = 0xFF;

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(OE, OUTPUT);

  digitalWrite(OE, 1);

  draw2(0, 0, WHITE);
  draw4(0, 16, PURPLE);
  draw8(0, 32, VIOLET);
  draw16(0, 48, AQUA);
  draw32(8, 0, BLUE2);
  draw64(8, 16, BLUE);
  draw128(8, 32, YELLOW);
  draw256(8, 48, LIGHT_GREEN);
  draw512(16, 0, ORANGE);
  draw1024(16, 16, RED);
  draw2048(16, 32, WHITE);

  draw_grid();

  update_buffer();

  Serial.begin(9600);
  randomSeed(analogRead(0));
  init_board();

  pinMode(interruptPin18, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin18), handleInterruptB1,
                  RISING);
  pinMode(interruptPin19, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin19), handleInterruptB2,
                  RISING);
  pinMode(interruptPin2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), handleInterruptB3,
                  RISING);
  pinMode(interruptPin3, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin3), handleInterruptB4,
                  RISING);

  Timer1.initialize(100);
  Timer1.attachInterrupt(display);
}

void display() {
  // Allow other interrupts (buttons) to execute
  interrupts();

  OE_HIGH;  // Disable output

  // Bit-plane management
  if (currentRow == 0) {
    displayingCycle = (displayingCycle + 1) % 4;

    // Increase timing to free CPU for buttons. Lower refresh rate.
    long newPeriod = 60 + (78 * (1 << displayingCycle));
    Timer1.setPeriod(newPeriod);
  }

  // --- Data Transmission ---
  uint8_t *ptr = buffer[displayingCycle][currentRow];
  set_row(currentRow);

  for (uint8_t col = 0; col < 64; col++) {
    PORTA = *ptr;
    ptr++;
    CLK_HIGH;
    CLK_LOW;
  }

  LAT_HIGH;
  LAT_LOW;

  OE_LOW; // Enable output

  currentRow = (currentRow + 1) % 16;
}

void draw_pixel(uint8_t row, uint8_t col, uint8_t color) {
  grid[row][col] = color;
}

void draw_row(uint8_t row, uint8_t color) {
  for (int col = 0; col < 64; col++) {
    grid[row][col] = color;
  }
}

void freeBuffer() {
  memset(grid, 0, sizeof(grid));
  update_buffer();
}

void draw_col(uint8_t col, uint8_t color) {
  for (int row = 0; row < 32; row++) {
    grid[row][col] = color;
  }
}

void set_row(uint8_t row_number) {
  // Maintain top 4 bits, update lower 4 bits for row selection
  PORTF = (PORTF & 0xF0) | (row_number & 0x0F);
}

void draw_grid() {
  draw_row(7, GREEN);
  draw_row(15, GREEN);
  draw_row(23, GREEN);
  draw_col(15, GREEN);
  draw_col(31, GREEN);
  draw_col(47, GREEN);
}

void draw0(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 5] = color;
  grid[row + 1][col + 4] = color;
  grid[row + 1][col + 6] = color;
  grid[row + 2][col + 4] = color;
  grid[row + 2][col + 6] = color;
  grid[row + 3][col + 4] = color;
  grid[row + 3][col + 6] = color;
  grid[row + 4][col + 4] = color;
  grid[row + 4][col + 6] = color;
  grid[row + 5][col + 5] = color;
}

void draw1(int8_t row, int8_t col, int8_t color) {
  grid[row + 2][col + 6] = color;
  grid[row + 1][col + 7] = color;
  grid[row + 0][col + 8] = color;
  grid[row + 1][col + 8] = color;
  grid[row + 2][col + 8] = color;
  grid[row + 3][col + 8] = color;
  grid[row + 4][col + 8] = color;
  grid[row + 5][col + 8] = color;
}

void draw2(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 1 + 6] = color;
  grid[row + 1][col + 0 + 6] = color;
  grid[row + 1][col + 2 + 6] = color;
  grid[row + 2][col + 2 + 6] = color;
  grid[row + 3][col + 1 + 6] = color;
  grid[row + 4][col + 0 + 6] = color;
  grid[row + 5][col + 0 + 6] = color;
  grid[row + 5][col + 1 + 6] = color;
  grid[row + 5][col + 2 + 6] = color;
}

void draw3(int8_t row, int8_t col, int8_t color) {
  grid[row + 1][col + 6] = color;
  grid[row + 0][col + 7] = color;
  grid[row + 1][col + 8] = color;
  grid[row + 2][col + 8] = color;
  grid[row + 3][col + 7] = color;
  grid[row + 4][col + 8] = color;
  grid[row + 5][col + 8] = color;
  grid[row + 5][col + 7] = color;
  grid[row + 5][col + 6] = color;
}

void draw4(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 8] = color;
  grid[row + 1][col + 7] = color;
  grid[row + 2][col + 6] = color;
  grid[row + 3][col + 6] = color;
  grid[row + 3][col + 7] = color;
  grid[row + 3][col + 8] = color;
  grid[row + 4][col + 7] = color;
  grid[row + 5][col + 7] = color;
}

void draw5(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 8] = color;
  grid[row + 0][col + 7] = color;
  grid[row + 0][col + 6] = color;
  grid[row + 1][col + 6] = color;
  grid[row + 2][col + 6] = color;
  grid[row + 2][col + 7] = color;
  grid[row + 3][col + 8] = color;
  grid[row + 4][col + 8] = color;
  grid[row + 5][col + 7] = color;
  grid[row + 5][col + 6] = color;
}

void draw6(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 8] = color;
  grid[row + 0][col + 7] = color;
  grid[row + 1][col + 6] = color;
  grid[row + 2][col + 6] = color;
  grid[row + 3][col + 6] = color;
  grid[row + 4][col + 6] = color;
  grid[row + 5][col + 6] = color;
  grid[row + 5][col + 7] = color;
  grid[row + 5][col + 8] = color;
  grid[row + 4][col + 8] = color;
  grid[row + 3][col + 8] = color;
  grid[row + 3][col + 7] = color;
}

void draw8(int8_t row, int8_t col, int8_t color) {
  grid[row + 0][col + 13 - 6] = color;
  grid[row + 1][col + 12 - 6] = color;
  grid[row + 1][col + 14 - 6] = color;
  grid[row + 3][col + 12 - 6] = color;
  grid[row + 3][col + 14 - 6] = color;
  grid[row + 4][col + 12 - 6] = color;
  grid[row + 4][col + 14 - 6] = color;
  grid[row + 5][col + 13 - 6] = color;
  grid[row + 3][col + 13 - 6] = color;
  grid[row + 2][col + 13 - 6] = color;
}

void draw9(int8_t row, int8_t col, int8_t color) {
  grid[row + 2][col + 7] = color;
  grid[row + 2][col + 6] = color;
  grid[row + 1][col + 6] = color;
  grid[row + 0][col + 6] = color;
  grid[row + 0][col + 7] = color;
  grid[row + 0][col + 8] = color;
  grid[row + 1][col + 8] = color;
  grid[row + 2][col + 8] = color;
  grid[row + 3][col + 8] = color;
  grid[row + 4][col + 8] = color;
  grid[row + 5][col + 8] = color;
  grid[row + 5][col + 7] = color;
  grid[row + 5][col + 6] = color;
}

void draw16(int8_t row, int8_t col, int8_t color) {
  draw1(row, col - 2, color);
  draw6(row, col + 2, color);
}

void draw32(int8_t row, int8_t col, int8_t color) {
  draw3(row, col - 2, color);
  draw2(row, col + 2, color);
}

void draw64(int8_t row, int8_t col, int8_t color) {
  draw6(row, col - 2, color);
  draw4(row, col + 2, color);
}

void draw128(int8_t row, int8_t col, int8_t color) {
  draw1(row, col - 4, color);
  draw2(row, col, color);
  draw8(row, col + 4, color);
}

void draw256(int8_t row, int8_t col, int8_t color) {
  draw2(row, col - 4, color);
  draw5(row, col, color);
  draw6(row, col + 4, color);
}

void draw512(int8_t row, int8_t col, int8_t color) {
  draw5(row, col - 4, color);
  draw1(row, col + 0, color);
  draw2(row, col + 4, color);
}

void draw1024(int8_t row, int8_t col, int8_t color) {
  draw1(row, col - 6, color);
  draw0(row, col, color);
  draw2(row, col + 2, color);
  draw4(row, col + 6, color);
}

void draw2048(int8_t row, int8_t col, int8_t color) {
  draw2(row, col - 6, color);
  draw0(row, col, color);
  draw4(row, col + 2, color);
  draw8(row, col + 6, color);
}

// ---------------- 2048 GAME LOGIC ----------------

void print_board_serial() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      Serial.print(board[r][c]);
      Serial.print('\t');
    }
    Serial.println();
  }
  Serial.println("----");
}

void add_random_tile() {
  int empty_r[16];
  int empty_c[16];
  int count = 0;

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 0) {
        empty_r[count] = r;
        empty_c[count] = c;
        count++;
      }
    }
  }

  if (count == 0)
    return;

  int randomIndex = random(0, count);
  int r = empty_r[randomIndex];
  int c = empty_c[randomIndex];

  int value = (random(0, 10) == 0) ? 4 : 2;
  board[r][c] = value;
}

void init_board() {
  score = 0;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      board[r][c] = 0;
    }
  }
  add_random_tile();
  add_random_tile();
  print_board_serial();
  draw_board_2048();
}

bool slide_and_merge_row_left(int row[4]) {
  int tmp[4] = { 0, 0, 0, 0 };
  int t = 0;
  bool moved = false;

  // pack nonzero values to left
  for (int i = 0; i < 4; i++) {
    if (row[i] != 0) {
      tmp[t++] = row[i];
    }
  }

  // merge adjacent matching tiles
  for (int i = 0; i < 3; i++) {
    if (tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
      tmp[i] *= 2;
      score += tmp[i];
      tmp[i + 1] = 0;
      moved = true;
    }
  }

  // pack again
  int tmp2[4] = { 0, 0, 0, 0 };
  t = 0;
  for (int i = 0; i < 4; i++) {
    if (tmp[i] != 0) {
      tmp2[t++] = tmp[i];
    }
  }

  // detect change
  for (int i = 0; i < 4; i++) {
    if (row[i] != tmp2[i])
      moved = true;
    row[i] = tmp2[i];
  }
  return moved;
}

bool move_left() {
  bool moved = false;
  for (int r = 0; r < 4; r++) {
    int row[4];
    for (int c = 0; c < 4; c++) {
      row[c] = board[r][c];
    }
    if (slide_and_merge_row_left(row))
      moved = true;
    for (int c = 0; c < 4; c++) {
      board[r][c] = row[c];
    }
  }
  if (moved) {
    add_random_tile();
    print_board_serial();

    if (check_win()) {
      display_win();
      while (1)
        ;  // Halt execution
    }

    if (!can_move()) {
      display_game_over();
      while (1)
        ;  // Halt execution
    }

    draw_board_2048();
  }
  return moved;
}

bool move_right() {
  bool moved = false;
  for (int r = 0; r < 4; r++) {
    int row[4];
    // reverse
    for (int c = 0; c < 4; c++) {
      row[c] = board[r][3 - c];
    }
    if (slide_and_merge_row_left(row))
      moved = true;
    // restore order
    for (int c = 0; c < 4; c++) {
      board[r][3 - c] = row[c];
    }
  }
  if (moved) {
    add_random_tile();
    print_board_serial();

    if (check_win()) {
      display_win();
      while (1)
        ;  // Halt execution
    }

    if (!can_move()) {
      display_game_over();
      while (1)
        ;  // Halt execution
    }

    draw_board_2048();
  }
  return moved;
}

bool move_up() {
  bool moved = false;
  for (int c = 0; c < 4; c++) {
    int col[4];
    for (int r = 0; r < 4; r++) {
      col[r] = board[r][c];
    }
    if (slide_and_merge_row_left(col))
      moved = true;
    for (int r = 0; r < 4; r++) {
      board[r][c] = col[r];
    }
  }
  if (moved) {
    add_random_tile();
    print_board_serial();

    if (check_win()) {
      display_win();
      while (1)
        ;  // Halt execution
    }

    if (!can_move()) {
      display_game_over();
      while (1)
        ;  // Halt execution
    }

    draw_board_2048();
  }
  return moved;
}

bool move_down() {
  bool moved = false;
  for (int c = 0; c < 4; c++) {
    int col[4];
    // reverse
    for (int r = 0; r < 4; r++) {
      col[r] = board[3 - r][c];
    }
    if (slide_and_merge_row_left(col))
      moved = true;
    // restore order
    for (int r = 0; r < 4; r++) {
      board[3 - r][c] = col[r];
    }
  }
  if (moved) {
    add_random_tile();
    print_board_serial();

    if (check_win()) {
      display_win();
      while (1)
        ;  // Halt execution
    }

    if (!can_move()) {
      display_game_over();
      while (1)
        ;  // Halt execution
    }

    draw_board_2048();
  }
  return moved;
}

bool can_move() {
  // 1. Check empty cells
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 0)
        return true;
    }
  }

  // 2. Check horizontal merge
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 3; c++) {
      if (board[r][c] == board[r][c + 1])
        return true;
    }
  }

  // 3. Check vertical merge
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == board[r + 1][c])
        return true;
    }
  }

  return false;
}

bool check_win() {
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (board[r][c] == 2048)
        return true;
    }
  }
  return false;
}

void display_game_over() {
  clear_board_pixels();
  // Fill screen with RED
  for (int r = 0; r < 32; r++) {
    for (int c = 0; c < 64; c++) {
      grid[r][c] = RED;
    }
  }
  update_buffer();
  Serial.print("GAME OVER - Final Score: ");
  Serial.println(score);
}

void display_win() {
  clear_board_pixels();
  // Fill screen with YELLOW
  for (int r = 0; r < 32; r++) {
    for (int c = 0; c < 64; c++) {
      grid[r][c] = YELLOW;
    }
  }
  update_buffer();
  Serial.print("YOU WIN! Final Score: ");
  Serial.println(score);
}

void clear_board_pixels() {
  for (int r = 0; r < 32; r++) {
    for (int c = 0; c < 64; c++) {
      grid[r][c] = BLACK;
    }
  }
}

void draw_tile_value(int value, uint8_t row, uint8_t col) {
  switch (value) {
    case 2:
      draw2(row, col, WHITE);
      break;
    case 4:
      draw4(row, col, AQUA);
      break;
    case 8:
      draw8(row, col, BLUE2);
      break;
    case 16:
      draw16(row, col, BLUE);
      break;
    case 32:
      draw32(row, col, VIOLET);
      break;
    case 64:
      draw64(row, col, PURPLE);
      break;
    case 128:
      draw128(row, col, RED);
      break;
    case 256:
      draw256(row, col, ORANGE);
      break;
    case 512:
      draw512(row, col, DARK_GREEN);
      break;
    case 1024:
      draw1024(row, col, LIGHT_GREEN);
      break;
    case 2048:
      draw2048(row, col, WHITE);
      break;
    default:
      break;
  }
}

void draw_board_2048() {
  clear_board_pixels();
  draw_grid();

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int val = board[r][c];
      if (val != 0) {
        TilePos tp = tilePos[r][c];
        draw_tile_value(val, tp.row, tp.col);
      }
    }
  }
  update_buffer();
}

void update_buffer() {
  uint8_t bits_mask = 0b11000000;
  uint8_t port;
  uint8_t color1;
  uint8_t color2;

  for (uint8_t plane = 0; plane < 4; plane++) {

    for (uint8_t row = 0; row < 16; row++) {
      for (uint8_t col = 0; col < 64; col++) {
        port = 0;
        color1 = (grid[row][col] & bits_mask) >> (6 - plane * 2);
        color2 = (grid[row + 16][col] & bits_mask) >> (6 - plane * 2);

        if (color1 == 0b01) {  // RED
          port |= (1 << R1);
        } else if (color1 == 0b10) {  // GREEN
          port |= (1 << G1);
        } else if (color1 == 0b11) {  // BLUE
          port |= (1 << B1);
        }

        if (color2 == 0b01) {  // RED
          port |= (1 << R2);
        } else if (color2 == 0b10) {  // GREEN
          port |= (1 << G2);
        } else if (color2 == 0b11) {  // BLUE
          port |= (1 << B2);
        }

        buffer[plane][row][col] = port;
      }
    }

    bits_mask = bits_mask >> 2;
  }
}

void handleInterruptB1() {
  long time = millis();
  if ((last_interrupt1 + debounce_delay) < time) {
    button1pressed = true;
    last_interrupt1 = millis();
  }
}

void handleInterruptB2() {
  long time = millis();

  if ((last_interrupt2 + debounce_delay) < time) {
    button2pressed = true;
    last_interrupt2 = millis();
  }
}

void handleInterruptB3() {
  long time = millis();

  if ((last_interrupt3 + debounce_delay) < time) {
    button3pressed = true;
    last_interrupt3 = millis();
  }
}

void handleInterruptB4() {
  long time = millis();

  if ((last_interrupt4 + debounce_delay) < time) {
    button4pressed = true;
    last_interrupt4 = millis();
  }
}

void loop() {
  if (button1pressed) {
    button1pressed = false;
    move_up();
  }
  if (button2pressed) {
    button2pressed = false;
    move_left();
  }
  if (button3pressed) {
    button3pressed = false;
    move_right();
  }
  if (button4pressed) {
    button4pressed = false;
    move_down();
  }
}