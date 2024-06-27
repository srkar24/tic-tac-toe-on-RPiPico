#ifndef __GAME_HPP__                                      
#define __GAME_HPP__

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include <stddef.h>
#include <stdio.h>

// Struct for storing GPIO configuration info
typedef struct {
    uint pin_number;
    uint pin_dir;
    // bool pull_down_enabled; 
} GpioConfig;

// Struct for storing button state info
// @field but_pin The number of the button pin
typedef struct {
    uint but_pin;
    bool prev_state;
    bool curr_state;
} BtnState;

class Game {
    public: 
        static const int ROWS = 3;
        static const int COLS = 3;
        static const char EMPTY = ' ';
        static const char X = 'X';                 // Player 1 symbol
        static const char O = 'O';                 // Player 2 symbol
        static const int DEBOUNCE_DELAY = 30;     // Debounce delay
        static const int BLINK_LED_DELAY = 500;    // Blink LED delay
        static const int HIGH = 1;
        static const int LOW = 0;
        
        // Define the pin numbers for the on-board LEDs and BTNs
        static const uint ONBOARD_LED = 25;         
        static const uint LED1 = 19; // RED
        static const uint LED2 = 16; // YELLOW
        static const uint BTN1 = 6; // Traversing Button 
        static const uint BTN2 = 14; // Confirm Selection Button
        static const uint BTN3 = 12; // Reset Button
        static const size_t NUMBER_OF_GPIOS = 6;

        // Constructor
        Game();                                         

        // Initializes the GPIOs
        // @param gpio Pointer to the GpioConfig struct
        // @param NUMBER_OF_GPIOS It's the length of the GpioConfig struct
        // void init_gpio(GpioConfig *gpio, size_t len);
        void init_gpio(GpioConfig (&gpio)[NUMBER_OF_GPIOS]);

        // Debounce function to ensure the button state is stable
        // @param btn Pointer to the BtnState structure
        // @return true If button state is stable
        // @return false If button state is not stable
        bool debounce(const volatile BtnState &btn);

        // Returns true if button state is stable else returns false if not stable
        // @param button Index of button
        // @param prev_state Previous state of the button
        bool is_stable(const uint button, const bool prev_state);

        // Returns whether button state has changed or not
        bool has_changed(bool prev_state, bool curr_state);

        // Updates the button state
        // @param btn Pointer to the BtnState struct
        void update_btn_state(volatile BtnState *btn);

        // Resets the board and updates the current player, moves and is_game_over variables
        // @param current_player Pointer to the current player
        // @param moves Pointer to the number of moves
        // @param board Pointer to the tic-tac-toe board
        // @param is_game_over Pointer to the flag indicating whether the game is over or not
        void reset_board(char *current_player, uint *moves, char (*board)[COLS], bool *is_game_over);

        // Returns the current row based on the number of moves
        // @returns Current row
        // @param moves Number of moves
        uint get_curr_row(const uint moves);

        // Returns next row based on number of moves
        uint get_next_row(const uint moves);

        // Returns next column based on number of moves
        uint get_next_col(const uint moves);

        // Returns current column based on number of moves
        uint get_curr_col(const uint moves);

        // Update the current position by incrementing the number of moves made
        void update_position(uint *moves);

        // Print the current position on the board 
        // @param row Current row
        // @param col Current column
        void print_curr_pos(const uint row, const uint col);

        // Checks if the position on the board is valid
        bool is_valid_pos(const uint row, const uint col);

        // Check if a position on the board is empty
        bool is_empty_pos(uint const row, uint col, const char (*board)[COLS]);

        // Update the board with the current player's move
        // @param moves Number of moves made
        void update_board(const char current_player, const uint moves, char (*board)[COLS]);

        // Print the board
        void print_board(const char (*board)[COLS]);

        // Print the current player's turn
        void print_player_turn(const char current_player);

        // Update the LED indicating the current player
        void update_player_led(const char current_player);

        // Handle btn1 press
        // @param moves The number of moves made
        void handle_btn1(uint *moves);

        // Handle btn2 press
        void handle_btn2(char *current_player, uint *moves, char (*board)[COLS], bool *is_game_over);

        // Returns new player's character
        char get_new_player(char current_player);

        // Check if the given player has won the game, return true if player won else false
        // @param player The current player's character (X or O)
        // @param board The current state of the board
        bool is_win(const char player, const char (*board)[COLS]);

        // Check if the game is tied
        bool is_tie(const char (*board)[COLS]);

        // Flash the LED indicating the winner
        static void flash_winner_led();
};

#endif  // __GAME_HPP__