/// \file

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

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int display(const char *str)
{
    printf("%s\n", str);

    return 0;
}

letter getLetterByChar(char c) {
    // Abusing ASCII order to find letters faster
    if ( isalpha(c) ) {
        int a = toupper(c) - 65;
        return alphabet[a];
    } else if (isdigit(c) ) {
        int a = c - 48 + 26;
        return alphabet[a];
    } 

    // Punctuation is not in such a nice order. Need to search alphabet array
    for (size_t i = 0; i < ALPHABET_LENGTH; ++i) {
        if ( alphabet[i].c == c)
            return alphabet[i];
        
    }
    return alphabet[ALPHABET_LENGTH - 1];    // If letter not found return last letter in alphabet '\', the protocol error value
}

letter getLetterByMorse(char *morse) {
    for (size_t i = 0; i < ALPHABET_LENGTH; ++i) {
        if (strcmp(morse, alphabet[i].morse) == 0) {

            // Special case. X and * have the same Morse value {0,1,1,0}
            // Program would have to use context to choose which one to interpret as. Will not do so.
            // Instead, everything will be 'X'. X is [23] in alphabet
            if (alphabet[i].c == '*') {
                i = 23;
            }

            return alphabet[i];
        }

    }
    // If letter not found return last letter in alphabet '\', the protocol error value
    return alphabet[ALPHABET_LENGTH - 1];
}

uint8_t set_bit8(uint8_t byte, uint8_t mask) {
    uint8_t set;
    set = byte | mask;
    return set;
}


void print_mask8(uint8_t byte, uint8_t mask) {
    uint8_t masked;
    masked = byte & mask;
    printf("%u\n", masked);
}


uint8_t get_mask8(uint8_t byte, uint8_t mask) {
    uint8_t masked;
    masked = byte & mask;
    return masked;
}


void printLetter(letter l) {
    printf("%c, %zu\n", l.c, l.length);
    for(size_t i = 0; i < l.length; i++) {
        printf("%d ", *((l.sequence)+i));
    }
    display("");
    printf("%s\n", l.morse);
}
