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

int display(const char *str)
{
    printf("%s\n", str);

    return 0;
}

letter getLetterByChar(char c) {
    if ( isalpha(c) ) {
        int a = toupper(c) - 65;
        return alphabet[a];
    } else if (isdigit(c) ) {
        int a = c - 48 + 26;
        return alphabet[a];
    } 

    switch (c) {
        case '&':
            return alphabet[36];
            break;
        case '\'':
            return alphabet[37];
            break;
        case '@':
            return alphabet[38];
            break;
        case ')':
            return alphabet[39];
            break;
        case '(':
            return alphabet[40];
            break;
        case ':':
            return alphabet[41];
            break;
        case ',':
            return alphabet[42];
            break;
        case '=':
            return alphabet[43];
            break;
        case '!':
            return alphabet[44];
            break;
        case '.':
            return alphabet[45];
            break;
        case '-':
            return alphabet[46];
            break;
        case '%':
            return alphabet[47];
            break;
        case '+':
            return alphabet[48];
            break;
        case '\"':
            return alphabet[49];
            break;
        case '\?':
            return alphabet[50];
            break;
        case '/':
            return alphabet[51];
            break;
        case '\n':
            return alphabet[52];
            break;
        default:
            return alphabet[25];
            break;
    }
}

uint8_t set_bit8(uint8_t byte, uint16_t mask) {
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