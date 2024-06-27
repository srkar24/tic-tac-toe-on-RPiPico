#include "game.hpp"
#include "hardware/gpio.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include <array>
#include <cstdint> 
#include <cstddef>
#include <iostream>
#include <ostream>

using namespace std;

Game::Game() {
    // Initialize game variables such as game board and other variables here
    // Pull-up the buttons to get them to work
    gpio_pull_up(BTN1);
    gpio_pull_up(BTN2);
    gpio_pull_up(BTN3);
}

// void init_gpio(GpioConfig *gpio, size_t len);
void Game::init_gpio(GpioConfig (&gpio)[NUMBER_OF_GPIOS]) {
    // Loop through the GpioConfig array length
    // Initialize the LEDs, BTNs and set respective directions (IP/OP)
    for (size_t i = 0; i < Game::NUMBER_OF_GPIOS; i++) {
        gpio_init(gpio[i].pin_number);
        gpio_set_dir(gpio[i].pin_number, gpio[i].pin_dir);
    }
}

bool Game::debounce(const volatile BtnState &btn) {
    // Checks whether btn state has changed or not
    if (has_changed(btn.prev_state, btn.curr_state)) {
        // Checks whether btn state is stable
        if (is_stable(btn.but_pin, btn.curr_state)) {
            // Return true if btn state has changed and is stable
            return true;
        }
    }
    // Return false if btn state has not changed or is not stable
    return false;
}

bool Game::is_stable(const uint button, const bool prev_state) {
    // Wait for the specified amount of time i.e., 500ms
    sleep_ms(DEBOUNCE_DELAY);
    // Now get the current state of the button
    uint current_state = gpio_get(button);
    // Check if previous and current state of the button is high
    if (prev_state == HIGH && current_state == HIGH) {
        // Optionally print a message if button state is stable if 
        // the preprocessor macro "VERBOSE" is defined
        #ifdef VERBOSE
            printf("Button state is stable");
        #endif

        // Returns true if button state is stable
        return true;
    }
    // Returns false if button state is not stable
    return false;
}

bool Game::has_changed(bool prev_state, bool curr_state) {
    // Edge detector - Checks if previous state is LOW and current state if HIGH    
    bool changed = (prev_state == LOW && curr_state == HIGH);
    // Optionally print a message if button state is stable if 
    // the preprocessor macro "VERBOSE" is defined
    #ifdef VERBOSE
        if(changed) {
            printf("Button state is stable");
        }
    #endif    

    // Returns true if button state has changed, false otherwise
    return changed;
}

void Game::update_btn_state(volatile BtnState *btn) {
    // Check if the current state of the button is 0
    if (btn->curr_state == 0) {
        // If yes, then set previous state of the button to 0
        btn->prev_state = 0;
    }
    // Check if the current state of the button is 1
    else if(btn->curr_state == 1) {
        // If yes, then set previous state of the button to 1
        btn->prev_state = 1;
    }
    // Update current state of the button by reading the button pin
    btn->curr_state = gpio_get(btn->but_pin);
}

void Game::reset_board(char *current_player, uint *moves, char (*board)[COLS], 
                       bool *is_game_over) {
    // Print message indicating board is being reset
    cout << "Reset board" << endl;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // Set each cell value to EMPTY i.e., ' '
            board[i][j] = EMPTY;
        }
    }

    // Reset the number of moves to 0
    *moves = 0;

    // Reset the current player to "X"
    *current_player = X;

    // Reset the game over flag to false
    *is_game_over = false;

    // Call the function "print_board" with the board as an argument
    print_board(board);

    // Call the function "print_player_turn" with current_player as the argument
    print_player_turn(*current_player);

    // Call the function "multicore_fifo_push_blocking" with EMPTY as the arg
    // Push EMPTY to each cell on the board
    multicore_fifo_push_blocking(EMPTY);
}

uint Game::get_curr_row(const uint moves) {
    // Return the current row in the game board
    // Divide the number of moves by ROWS (which is 3) 
    // 0 to 8 represent the cell numbers below: 
    // 0/3 = 0; 1/3 = 0; 2/3 = 0; --> Row 0
    // 3/3 = 1; 4/3 = 1; 5/3 = 1; --> Row 1
    // 6/3 = 2; 7/3 = 2; 8/3 = 2; --> Row 2
    return static_cast<uint>(moves) / ROWS;
}

uint Game::get_next_row(const uint moves) {
    // C -> Cell; R -> ROW
    // (C2 + 1) / 3 = R1; (C5 + 1) / 3 = R2;
    return static_cast<uint>(moves + 1) / ROWS;
}

uint Game::get_next_col(const uint moves) {
    // Return the next column on the board based on:
    // C -> Cell; C -> COLS
    // (C0 + 1) % 3 = C1; (C1 + 1) / 3 = C2;
    return static_cast<uint>(moves + 1) % COLS;
}

uint Game::get_curr_col(const uint moves) {
    // Return the current column in the game board
    // Divide the number of moves by COLS (which is 3) 
    // 0 to 8 represent the cell numbers below: 
    // 0%3 = 0; 1%3 = 1; 2%3 = 2; --> Column 0, 1, 2
    // 3%3 = 0; 4%3 = 1; 5%3 = 2; --> Column 0, 1, 2
    // 6%3 = 0; 7%3 = 1; 8%3 = 2; --> Column 0, 1, 2
    return static_cast<uint>(moves) % COLS;
}

void Game::update_position(uint *moves) {
    // Calculate the next row and column of the board
    uint next_row = get_next_row(*moves);
    uint next_col = get_next_col(*moves);

    // Check if the next position is within the valid range of the board
    if (is_valid_pos(next_row, next_col)) {
        (*moves)++;
    } else {
        #ifdef VERBOSE
            // Prints message to indicate start of new round
            printf("Starting new round");
        #endif

        // Reset the moves counter
        *moves = 0;
    }
}

void Game::print_curr_pos(const uint row, const uint col) {
    cout << "Row: " << row << " Col: " << col << endl;
}

bool Game::is_valid_pos(const uint row, const uint col) {
    // Returns true if both row and column are greater than or equal to 0 and 
    // less than ROWS (3) and COLS (3) respectively
    return (row < ROWS && col < COLS);
}

bool Game::is_empty_pos(uint const row, uint col, const char (*board)[COLS]) {
    // Returns true if cell is empty
    return (board[row][col] == EMPTY);
}

void Game::update_board(const char current_player, const uint moves, char (*board)[COLS]) {
    uint row = get_curr_row(moves);
    uint col = get_curr_col(moves);
    // Print current player's input being entered to the respective row column
    cout << "Entering player " << current_player << "input into row " << row << " col " << col << endl;

    // Update the board at the calculated row and col with the current player's input
    board[row][col] = current_player;
}

void Game::print_board(const char (*board)[COLS]) {
    // ANSI escape coe to clear the terminal screen, effectively erasing all content, 
    // and move the cursor to the top-left corner
    cout << "\e[1;1H\e[2J";

    // Print the vertical separators except for the last column
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            cout << " " << board[i][j] << " ";

            if (j < COLS - 1) {
                cout << "|";
            }
        }

        // Print the horizontal separators except for the last row
        if (i < ROWS - 1) {
            cout << endl << "---+---+---" << endl;
        } else {
            cout << endl;
        }
    }
}

void Game::print_player_turn(const char current_player) {
    // Print the "Player %c turn" message 
    cout << "Player " << current_player << " turn" << endl;
}

void Game::update_player_led(const char current_player) {
    if (current_player == X) {
        gpio_put(LED1, HIGH);
        gpio_put(LED2, LOW);
    }
    else if (current_player == O) {
        gpio_put(LED1, LOW);
        gpio_put(LED2, HIGH);
    }
    else {
        gpio_put(LED1, LOW);
        gpio_put(LED2, LOW);
    }
}

void Game::flash_winner_led(){
    uint32_t winner = static_cast<uint32_t>(EMPTY);
    uint led_pin = ONBOARD_LED;

    while (true) {
        if (multicore_fifo_rvalid()) {
            // Pop a value from "multicore_fifo" and 
            // store it in variable winner
            winner = multicore_fifo_pop_blocking();
        }

        if ((char)winner == X) {
            led_pin = LED1;
        } else if ((char)winner == O) {
            led_pin = LED2;
        } else {
            led_pin = ONBOARD_LED;
        }

        gpio_put(led_pin, HIGH);
        sleep_ms(BLINK_LED_DELAY);
        gpio_put(led_pin, LOW);
        sleep_ms(BLINK_LED_DELAY);
    }
}

void Game::handle_btn1(uint *moves) {
    // Update the position of moves
    update_position(moves);

    uint curr_row = get_curr_row(*moves);

    uint curr_col = get_curr_col(*moves);

    print_curr_pos(curr_row, curr_col);
}

void Game::handle_btn2(char *current_player, uint *moves, char (*board)[COLS], 
                       bool *is_game_over) {
    uint row = get_curr_row(*moves);
    uint col = get_curr_col(*moves);

    // Check if position (row, col) is valid
    if (!is_valid_pos(row, col)) {
        // If position is not valid, print below message
        cout << "Invalid selection row " << row << " col " << col << endl;
        // Return from the function
        return;
    }

    // Check if position (row, col) is empty
    if (!is_empty_pos(row, col, board)) {
        // If position is not empty, print below message
        cout << "Row " << row << " Col " << col << " is not empty" << endl;
        cout << "Please select another location" << endl;
        // Return from the function
        return;
    }

    // Update the board
    update_board(*current_player, *moves, board);
    // Print the board
    print_board(board);

    // Check if there's a win
    if (is_win(*current_player, board)) {
        cout << "Player " << *current_player << " wins!" << endl;
    // Push the winner *current_player
    multicore_fifo_push_blocking(*current_player);
    // Set game over as true
    *is_game_over = true;
    cout << "Please press the reset button to start the game" << endl;
    } else if (is_tie(board)) {
        printf("Tie game! \n");
        reset_board(current_player, moves, board, is_game_over);
    } else {
        *moves = 0;
        *current_player = get_new_player(*current_player);
        print_player_turn(*current_player);
    }
}

char Game::get_new_player(char current_player) {
    // Return the next player symbol
    return (current_player == X) ? O : X;
}

 bool Game::is_win(const char player, const char (*board)[COLS]) {
    for (int i = 0; i < ROWS; i++) {
        // Check if all the element in the row are equal to the player
        if (board[i][0] == player && board[i][1] == player && 
            board[i][2] == player) {
                // Player wins!
                return true;
            }
    }

    for (int j = 0; j < COLS; j++) {
        // Check if all the element in the column are equal to the player
        if (board[0][j] == player && board[1][j] == player &&
            board[2][j] == player) {
                // Player wins!
                return true;
            }
    }

    // Check elements diagonally for winner
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
        return true;
    }

    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
        return true;
    }

    // Player has not won
    return false;
 }

bool Game::is_tie(const char (*board)[COLS]) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == EMPTY) {
                // Return no tie if the cell is empty
                return false;
            }
        }
    }
    // Return tie if all cells are filled
    return true;
}