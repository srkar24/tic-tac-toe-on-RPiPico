#include "game.hpp"
#include "hardware/gpio.h"
#include "pico/stdio.h"

int main() {

    // Set the board values (3x3 board) to EMPTY
    char board[Game::ROWS][Game::COLS] = {
        {Game::EMPTY, Game::EMPTY, Game::EMPTY},
        {Game::EMPTY, Game::EMPTY, Game::EMPTY},
        {Game::EMPTY, Game::EMPTY, Game::EMPTY}
    };

    // Set current player to X
    char current_player = Game::X;

    // Set current number of moves initialized to 0
    uint moves = 0;

    bool is_game_over = false;

    // Set the array of structs of GPIO configuration
    GpioConfig my_gpio[Game::NUMBER_OF_GPIOS] = {
        {Game::LED1, GPIO_OUT}, 
        {Game::LED2, GPIO_OUT}, 
        {Game::BTN1, GPIO_IN}, 
        {Game::BTN2, GPIO_IN}, 
        {Game::BTN3, GPIO_IN}, 
        {Game::ONBOARD_LED, GPIO_OUT}, 
    };

    // Struct for button 1 state
    volatile BtnState btn1 = {
        .but_pin = Game::BTN1,
        .prev_state = false,
        .curr_state = false
    };

    // Struct for button 2 state
    volatile BtnState btn2 = {
        .but_pin = Game::BTN2,
        .prev_state = false,
        .curr_state = false
    };   

    // Struct for button 3 state
    volatile BtnState btn3 = {
        .but_pin = Game::BTN3,
        .prev_state = false,
        .curr_state = false
    }; 

    // Create an instance of the Game class
    Game game; 

    // Initialize the standard input/output library
    stdio_init_all();   

    // Run code on core1 
    multicore_launch_core1(Game::flash_winner_led); 

    game.init_gpio(my_gpio);
    
    game.reset_board(&current_player, &moves, board, &is_game_over);

    while (true) {
        // Update player status led if game is not over
        if (!is_game_over) {
            game.update_player_led(current_player);

            // Pull-down, active-low for btn1
            if (!gpio_get(Game::BTN1)) {
                // Check and handle button 1 press event
                game.update_btn_state(&btn1);
                if (game.debounce(btn1)) {
                    game.handle_btn1(&moves);
                }
            }
            
            // Pull-down, active-low for btn2
            if (!gpio_get(Game::BTN2)) {
                // Check and handle button 2 press event
                game.update_btn_state(&btn2);
                if (game.debounce(btn2)) {
                    game.handle_btn2(&current_player, &moves, board, &is_game_over);
                }
            }
        }

        // Pull-down, active-low for btn3
        if (!gpio_get(Game::BTN3)) {
            // Check and handle button 3 press event 
            game.update_btn_state(&btn3);
            if (game.debounce(btn3)) {
                game.reset_board(&current_player, &moves, board, &is_game_over);
            }
        }

    }

    return 0;
}