/**
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

const uint8_t MASK_00000001 = UINT8_C(0x00000001);
const uint8_t MASK_00000010 = UINT8_C(0x00000002);
const uint8_t MASK_00000100 = UINT8_C(0x00000004);
const uint8_t MASK_00001000 = UINT8_C(0x00000008);
const uint8_t MASK_00010000 = UINT8_C(0x00000010);
const uint8_t MASK_00100000 = UINT8_C(0x00000020);
const uint8_t MASK_01000000 = UINT8_C(0x00000040);
const uint8_t MASK_10000000 = UINT8_C(0x00000080);

static const uint8_t masks_8[] = {
    MASK_00000001,
    MASK_00000010,
    MASK_00000100,
    MASK_00001000,
    MASK_00010000,
    MASK_00100000,
    MASK_01000000,
    MASK_10000000
};

struct application_settings
{
    struct dc_opt_settings opts;
};

static struct dc_application_settings *create_settings( const struct dc_posix_env *env,
                                                        struct dc_error *err);

static int destroy_settings(const struct dc_posix_env *env,
                            struct dc_error *err,
                            struct dc_application_settings **psettings);
static int run( const struct dc_posix_env *env,
                struct dc_error *err,
                struct dc_application_settings *settings);
static void error_reporter(const struct dc_error *err);
static void trace_reporter(const struct dc_posix_env *env,
                          const char *file_name,
                          const char *function_name,
                          size_t line_number);
static void readInput(const struct dc_posix_env *env, struct dc_error *err, char *dest);
static void convertToMorse(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest);
static void writeToFile(const struct dc_posix_env *env, struct dc_error *err, char *input);


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
    char * output;

    ret_val = EXIT_SUCCESS;
    app_settings = (struct application_settings *)settings;


    readInput(env, err, chars);
    convertToMorse(env, err, chars, output);
    writeToFile(env, err, output);

    // Prints out a letter's members
    // printf("%c, %d\n", alphabet[0].c, alphabet[0].sequenceLength);
    // for(size_t i = 0; i < alphabet[0].sequenceLength; i++) {
    //     printf("%d\n", *((alphabet[0].sequence)+i));
    // }

    return ret_val;
}

static void readInput(const struct dc_posix_env *env, struct dc_error *err, char *dest) {
    ssize_t nread;
    
    display("read");
    if (dc_error_has_no_error(err)) {
        nread = dc_read(env, err, STDIN_FILENO, dest, BUF_SIZE);
    }
}

static void convertToMorse(const struct dc_posix_env *env, struct dc_error *err, char *input, char *dest) {
    char * output;
    display("convert");
    
    printf("%s", input);
}

static void writeToFile(const struct dc_posix_env *env, struct dc_error *err, char *output) {

    display("write");

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
