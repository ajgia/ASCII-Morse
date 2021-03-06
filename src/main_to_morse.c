/// \file

/*
 * ASCII to Morse
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
 * Performs ASCII-to-Morse encoding upon read-in chars
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
 * Constructs a string representation of binary Morse encoding from ASCII chars
 * @param env dc_env 
 * @param err dc_err
 * @param input a pointer to char input
 * @param nread numBytes of input
 * @param dest a pointer to memory to construct at
 */ 
static void constructBinaryMorseRepresentation(char *input, size_t nread, char *dest);
/**
 * Constructs a real binary sequence from a Morse binary string representation
 * @param env dc_env 
 * @param err dc_err
 * @param input a pointer to char input
 * @param binary a pointer to memory to construct at
 */ 
static void constructBinary( char *input, uint8_t *binary);
/**
 * Writes binary sequence to STD_OUT
 * @param env dc_env 
 * @param err dc_err
 * @param binary a pointer to binary input
 * @param nread numBytes to write of input
 */ 

static void writeToFile(const struct dc_posix_env *env, struct dc_error *err, uint8_t *binary, size_t numBytes);

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
    info = dc_application_info_create(&env, &err, "To Morse Application");
    ret_val = dc_application_run(&env, &err, info, create_settings, destroy_settings, run, dc_default_create_lifecycle, dc_default_destroy_lifecycle, NULL, argc, argv);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);
    return ret_val;
}

static struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err)
{
    // static bool default_verbose = false;
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
                            __attribute__((unused)) struct dc_error *err,
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
    char chars[BUF_SIZE];
    char *output;
    uint8_t *binary;
    ssize_t nread = 0;


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

    // allocate memory for maximum possible code length string
    // sequences + EOCs + EOT + a nullbyte
    size_t count = (size_t)nread * MAX_MORSE + (size_t)nread *2 + 4 + 1;
    output = (char*)calloc(count, sizeof(char));

    constructBinaryMorseRepresentation(chars, (size_t)nread, output);

    // calculate resulting size of binary to write
    size_t numBytesBinary = (strlen(output) / 8);
    if ( (strlen(output)%8) != 0 )
        ++numBytesBinary;

    binary = (uint8_t*)calloc(numBytesBinary, sizeof(uint8_t));

    constructBinary(output, binary);
    writeToFile(env, err, binary, numBytesBinary);

    free(output);
    free(binary);
    return ret_val;
}

static void constructBinaryMorseRepresentation(char *input, size_t nread, char *dest) {
    // Process each char in input
    for (size_t i = 0; i < nread; i++) {
        // If not a space, get letter from alphabet
        if (*(input+i) != ' ') {
            letter l = getLetterByChar(*(input+i));
            // Translate dots and dashes of the char to 1s and 0s
            for(size_t j = 0; j < l.length; j++) {
                if (l.morse[j] == '.') 
                    strcat(dest, "10");
                else 
                    strcat(dest, "01");
            }
            strcat(dest, "00");
        }
        // If space, append space
        else 
            strcat(dest, "11");
    }
    // EOT
    strcat(dest, "0000");
    // printf("%s", dest);
}

static void constructBinary(char *input, uint8_t *output) {
    // Counter for input chars
    size_t i = 0;
    // Counter for byte bits (8)
    size_t j = 0;
    // Counter for which byte
    size_t k = 0;

    while( *(input+i) ) {

        // Set bit of current byte
        if (j < 8) {
            // if ( get_mask8(*(input+i), masks_8[j]) ) {
            if ( *(input+i) == '1') {
               *(output+k) = set_bit8(*(output+k), masks_8[j]);
            }
            ++j;
            ++i;
        }

        // New byte
        else {
            j = 0;
            ++k;
        }
    }
}

void writeToFile(const struct dc_posix_env *env, struct dc_error *err, uint8_t *binary, size_t numBytes) {
    dc_write(env, err, STDOUT_FILENO, binary, numBytes);
}

static void error_reporter(const struct dc_error *err) {
    fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number, 0);
    fprintf(stderr, "ERROR: %s\n", err->message);
}

static void trace_reporter(__attribute__((unused))  const struct dc_posix_env *env,
                                                    const char *file_name,
                                                    const char *function_name,
                                                    size_t line_number) {
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}
