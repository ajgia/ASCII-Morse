/// \file

#ifndef TEMPLATE_COMMON_H
#define TEMPLATE_COMMON_H

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * This file is part of dc_dump.
 *
 *  dc_dump is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Foobar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with dc_dump.  If not, see <https://www.gnu.org/licenses/>.
 */


/**
 * Sets mask bit of byte argument 
 */ 
uint8_t set_bit8(uint8_t byte, uint8_t mask);
/**
 * Prints masked byte argument
 */ 
void print_mask8(uint8_t byte, uint8_t mask);
/**
 * Gets masked byte
 */ 
uint8_t get_mask8(uint8_t byte, uint8_t mask);


/**
 * Prints to std_out the string argument followed by a newline
 *
 * @param str a string pointer
 * @return an int
 */
int display(const char *str);

/**
 * Defines a letter with both ASCII value and morse value represented as a bool array
 * Bool array: dot = true, dash = false
 */ 
typedef struct letter {
    char c;
    bool sequence[15];
    char *morse;
    size_t length;
} letter;

/**
 * Array of all morse letters
 */ 
static letter alphabet[26 + 10 + 16 + 1] = {
    { .c = 'A', .sequence = {1, 0}, .morse = ".-", .length = 2},
    { .c = 'B', .sequence = {0,1,1,1}, .morse = "-...", .length = 4},
    { .c = 'C', .sequence = {0,1,0,1}, .morse = "-.-.", .length = 4},
    { .c = 'D', .sequence = {0,1,1}, .morse = "-..", .length = 3},
    { .c = 'E', .sequence = {1}, .morse = ".", .length = 1},
    { .c = 'F', .sequence = {1,1,0,1}, .morse = "..-.", .length = 4},
    { .c = 'G', .sequence = {0,0,1}, .morse = "--.", .length = 3},
    { .c = 'H', .sequence = {1,1,1,1}, .morse = "....", .length = 4},
    { .c = 'I', .sequence = {1,1}, .morse = "..", .length = 2},
    { .c = 'J', .sequence = {1,0,0,0}, .morse = ".---", .length = 4},
    { .c = 'K', .sequence = {0,1,0}, .morse = "-.-", .length = 3},
    { .c = 'L', .sequence = {1,0,1,1}, .morse = ".-..", .length = 4},
    { .c = 'M', .sequence = {0,0}, .morse = "--", .length = 2},
    { .c = 'N', .sequence = {0,1}, .morse = "-.", .length = 2},
    { .c = 'O', .sequence = {0,0,0}, .morse = "---", .length = 3},
    { .c = 'P', .sequence = {1,0,0,1}, .morse = ".--.", .length = 4},
    { .c = 'Q', .sequence = {0,0,1,0}, .morse = "--.-", .length = 4},
    { .c = 'R', .sequence = {1,0,1}, .morse = ".-.", .length = 3},
    { .c = 'S', .sequence = {1,1,1}, .morse = "...", .length = 3},
    { .c = 'T', .sequence = {0}, .morse = "-", .length = 1},
    { .c = 'U', .sequence = {1,1,0}, .morse = "..-", .length = 3},
    { .c = 'V', .sequence = {1,1,1,0}, .morse = "...-", .length = 4},
    { .c = 'W', .sequence = {1,0,0}, .morse = ".--", .length = 3},
    { .c = 'X', .sequence = {0,1,1,0}, .morse = "-..-", .length = 4},
    { .c = 'Y', .sequence = {0,1,0,0}, .morse = "-.--", .length = 4},
    { .c = 'Z', .sequence = {0,0,1,1}, .morse = "--..", .length = 4},

    { .c = '0', .sequence = {0,0,0,0,0}, .morse = "-----", .length = 5},
    { .c = '1', .sequence = {1,0,0,0,0}, .morse = ".----", .length = 5},
    { .c = '2', .sequence = {1,1,0,0,0}, .morse = "..---", .length = 5},
    { .c = '3', .sequence = {1,1,1,0,0}, .morse = "...--", .length = 5},
    { .c = '4', .sequence = {1,1,1,1,0}, .morse = "....-", .length = 5},
    { .c = '5', .sequence = {1,1,1,1,1}, .morse = ".....", .length = 5},
    { .c = '6', .sequence = {0,1,1,1,1}, .morse = "-....", .length = 5},
    { .c = '7', .sequence = {0,0,1,1,1}, .morse = "--...", .length = 5},
    { .c = '8', .sequence = {0,0,0,1,1}, .morse = "---..", .length = 5},
    { .c = '9', .sequence = {0,0,0,0,1}, .morse = "----.", .length = 5},

// Error punctuation. Skipped
    { .c = '&', .sequence = {1,0,1,1,1}, .morse = ".-...", .length = 5},
    { .c = '\'', .sequence = {1,0,0,0,0,1}, .morse = ".----.", .length = 6},
    { .c = '@', .sequence = {1,0,0,1,0,1}, .morse = ".--.-.", .length = 6},
    { .c = ')', .sequence = {0,1,0,0,1,0}, .morse = "-.--.-", .length = 6},
    { .c = '(', .sequence = {0,1,0,0,1}, .morse = "-.--.", .length = 5},
    { .c = ':', .sequence = {0,0,0,1,1,1}, .morse = "---...", .length = 6},
    { .c = ',', .sequence = {0,0,1,1,0,0}, .morse = "--..--", .length = 6},
    { .c = '=', .sequence = {0,1,1,1,0}, .morse = "-...-", .length = 5},
    { .c = '!', .sequence = {0,1,0,1,0,0}, .morse = "-.-.--", .length = 6},
    { .c = '.', .sequence = {1,0,1,0,1,0}, .morse = ".-.-.-", .length = 6},
    { .c = '-', .sequence = {0,1,1,1,1,0}, .morse = "-....-", .length = 6},
 // Is the same as 'X'   { .c = '*', .sequence = {0,1,1,0}, .length = 4},
    { .c = '%', .sequence = {0,0,0,0,0, 0,1,1,0,1, 0,0,0,0,0}, .morse = "------..-.-----", .length = 15},
    { .c = '+', .sequence = {1,0,1,0,1}, .morse = ".-.-.", .length = 5},
    { .c = '\"', .sequence = {1,0,1,1,0,1}, .morse = ".-..-.", .length = 6},
    { .c = '\?', .sequence = {1,1,0,0,1,1}, .morse = "..--..", .length = 6},
    { .c = '/', .sequence = {0,1,1,0,1}, .morse = "-..-.", .length = 5},
    { .c = '\n', .sequence = {1,0,1,0}, .morse = ".-.-", .length = 4}
};

/**
 * Returns Morse struct letter by char argument
 */ 
letter getLetterByChar(char c);

/**
 * Returns Morse struct letter by morse sequence
 */ 
letter getLetterByMorse(char *morse);

/**
 * Prints letter's members
 */ 
void printLetter(letter l);

#endif // TEMPLATE_COMMON_H
