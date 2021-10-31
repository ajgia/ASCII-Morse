/// \file

/*
 * Morse to ASCII
 * Alex Giasson
 * A00982145 
*/

// 00 - eoc
// 01 - dash
// 10 - dot
// 11 - "\s"
// 00 00 - eot

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <dc_posix/dc_unistd.h>
#include <dc_application/command_line.h>
#include <dc_application/config.h>
#include <dc_application/defaults.h>
#include <dc_application/environment.h>
#include <dc_application/options.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_unistd.h>
#include <dc_util/bits.h>
#include <dc_util/dump.h>
#include <dc_util/types.h>
#include <dc_fsm/fsm.h>
#include <dc_posix/dc_time.h>

#define BUF_SIZE 1024

/**
 * Bit masks
 */ 
const uint8_t MASK_00000001 = UINT8_C(0x00000001);
const uint8_t MASK_00000010 = UINT8_C(0x00000002);
const uint8_t MASK_00000100 = UINT8_C(0x00000004);
const uint8_t MASK_00001000 = UINT8_C(0x00000008);
const uint8_t MASK_00010000 = UINT8_C(0x00000010);
const uint8_t MASK_00100000 = UINT8_C(0x00000020);
const uint8_t MASK_01000000 = UINT8_C(0x00000040);
const uint8_t MASK_10000000 = UINT8_C(0x00000080);

/**
 * Bit mask array
 */ 
static const uint8_t masks_8[] = {
    MASK_10000000,
    MASK_01000000,
    MASK_00100000,
    MASK_00010000,
    MASK_00001000,
    MASK_00000100,
    MASK_00000010,
    MASK_00000001
};


struct application_settings
{
    struct dc_opt_settings opts;
};

/*!
 * Creates application settings 
 */ 
static struct dc_application_settings *create_settings( const struct dc_posix_env *env,
                                                        struct dc_error *err);

/**
 * Destroys application settings
 */ 
static int destroy_settings(const struct dc_posix_env *env,
                            struct dc_error *err,
                            struct dc_application_settings **psettings);
/**
 * Performs Morse-to-ASCII encoding upon read-in binary
 */ 
static int run( const struct dc_posix_env *env,
                struct dc_error *err,
                struct dc_application_settings *settings);
/**
 * Error reporter
 */ 
static void error_reporter(const struct dc_error *err);
/**
 * Trace reporter
 */ 
static void trace_reporter(const struct dc_posix_env *env,
                          const char *file_name,
                          const char *function_name,
                          size_t line_number);

/**
 * Construct 0 and 1 string representation of binary input.
 * @param env dc_env 
 * @param err dc_err
 * @param input a pointer to char input
 * @param nread numBytes of input
 * @param dest a pointer to memory to construct at
 */ 
static void constructStringBinary(const struct dc_posix_env *env, struct dc_error *err, char *input, size_t nread, char *dest);

/**
 * Converts a string of binary morse to dot/dash morse
 * @param env dc_env
 * @param err dc_err
 * @param input a pointer to char input
 * @param dest a pointer to memory to construct at
 */ 
static void convertToMorse(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest);

/**
 * Converts dot/dash morse to ASCII
 * @param env dc_env
 * @param err dc_err
 * @param input a pointer to char input
 * @param dest a pointer to memory to construct at
 */ 
static void convertToAscii(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest);

/**
 * Main
 */ 
int main(int argc, char * argv[])
{
    dc_posix_tracer tracer;
    dc_error_reporter reporter;
    struct dc_posix_env env;
    struct dc_error err;
    struct dc_application_info *info;
    int ret_val;

    reporter = error_reporter;
    tracer = NULL;
    // tracer = trace_reporter;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);
    info = dc_application_info_create(&env, &err, "To ASCII Application");
    ret_val = dc_application_run(&env, &err, info, create_settings, destroy_settings, run, dc_default_create_lifecycle, dc_default_destroy_lifecycle, NULL, argc, argv);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);
    return ret_val;
}

static struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err)
{
    static bool default_verbose = false;
    struct application_settings *settings;

    DC_TRACE(env);
    settings = dc_malloc(env, err, sizeof(struct application_settings));

    if(settings == NULL)
    {
        return NULL;
    }

    settings->opts.parent.config_path = dc_setting_path_create(env, err);

    struct options opts[] = {
            {(struct dc_setting *)settings->opts.parent.config_path,
                    dc_options_set_path,
                    "config",
                    required_argument,
                    'c',
                    "CONFIG",
                    dc_string_from_string,
                    NULL,
                    dc_string_from_config,
                    NULL}
    };

    // note the trick here - we use calloc and add 1 to ensure the last line is all 0/NULL
    settings->opts.opts_count = (sizeof(opts) / sizeof(struct options)) + 1;
    settings->opts.opts_size = sizeof(struct options);
    settings->opts.opts = dc_calloc(env, err, settings->opts.opts_count, settings->opts.opts_size);
    dc_memcpy(env, settings->opts.opts, opts, sizeof(opts));
    settings->opts.flags = "m:";
    settings->opts.env_prefix = "DC_EXAMPLE_";

    return (struct dc_application_settings *)settings;
}

static int destroy_settings(const struct dc_posix_env *env,
                            struct dc_error *err,
                            struct dc_application_settings **psettings) 
{
    struct application_settings *app_settings;

    DC_TRACE(env);
    app_settings = (struct application_settings *)*psettings;
    dc_free(env, app_settings->opts.opts, app_settings->opts.opts_count);
    dc_free(env, *psettings, sizeof(struct application_settings));

    if(env->null_free)
    {
        *psettings = NULL;
    }

    return 0;
}

static int run(const struct dc_posix_env *env, struct dc_error *err, struct dc_application_settings *settings) {
    struct application_settings *app_settings;
    int ret_val;
    DC_TRACE(env);
    ssize_t nread = 0;
    char chars[BUF_SIZE];
    char stringBinary[BUF_SIZE*8] = "";
    char morseMessage[BUF_SIZE*8] = "";
    char asciiMessage[BUF_SIZE*8] = "";

    ret_val = EXIT_SUCCESS;
    app_settings = (struct application_settings *)settings;

    // Read
    if (dc_error_has_no_error(err)) {
        nread = dc_read(env, err, STDIN_FILENO, chars, BUF_SIZE);
    }
    
    if (dc_error_has_error(err)) {
        error_reporter(err);
        return EXIT_FAILURE;
    }

    constructStringBinary(env, err, chars, (size_t)nread, stringBinary);
    convertToMorse(env, err, stringBinary, morseMessage);
    convertToAscii(env, err, morseMessage, asciiMessage);

    dc_write(env, err, STDOUT_FILENO, asciiMessage, strlen(asciiMessage));
    display("");

    return ret_val;
}

static void constructStringBinary(const struct dc_posix_env *env, struct dc_error *err, char *input, size_t nread, char *dest) {
    // loop each byte
    for (size_t i = 0; i < nread; ++i) {

        // loop inside the byte
        for (size_t j = 0; j < 8; ++j) {
            if ( get_mask8(*(input+i), masks_8[j]) )
                strcat(dest, "1");
            else
                strcat(dest, "0");
        }
    }
}

static void convertToMorse(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest) {
    // loop through *input 
    // remember *input ends in a null byte
    // store the previous char. if last char == 0 and thisChar == 0, then end of character.
    size_t i = 0;
    char c;
    char prevC;

    while ( *(input+i) ) {

        prevC = c;
        c = *(input+i);
        // printf("%c", c);

        if ( i%2 == 1 && i != 0 ) {
            // translate c and prevC into dot, dash or space
            if ( prevC == '0' && c == '1') {
                strcat(dest, "-");
            } else if ( prevC == '1' && c == '0') {
                strcat(dest, ".");
            } else if ( prevC == '1' && c == '1') {
                strcat(dest, " ");
            }
            else {
                strcat(dest, "0"); // EOC. Internally make this a single 0 instead of 2 to make our lives easier later
            }
        }
        ++i;
    }
}

static void convertToAscii(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest) {
    size_t i = 0;
    char morse[MAX_MORSE] = "";
    size_t morseLength = 0;
    char c;
    char prevC;

    while ( *(input+i) ) {
        
        prevC = c;
        c = *(input+i);

        if ( c != '0' && c != ' ' ) {
            morse[morseLength] = c;
            ++morseLength;

        } else if ( c == '0') {
            if ( prevC == '0') {
                return;
            } 
            else if (prevC != ' ') {
                letter l = getLetterByMorse(morse);
                // Concatenate *dest with new char. Need to initialize new char into array to do so
                char arr[2] = {l.c, '\0'};
                strcat(dest, arr);

                // reset morse
                morseLength = 0;
                for (size_t j = 0; j < MAX_MORSE; ++j) {
                    morse[j] = '\0';
                }
            }
        }
        else if ( c == ' ' ) {
            strcat(dest, " ");
        }

        ++i;
    }
}

static void error_reporter(const struct dc_error *err) {
    fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number, 0);
    fprintf(stderr, "ERROR: %s\n", err->message);
}

static void trace_reporter( const struct dc_posix_env *env,
                            const char *file_name,
                            const char *function_name,
                            size_t line_number) {
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}

