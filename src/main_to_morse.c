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

static void error_reporter(const struct dc_error *err);
static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number);
static void will_change_state(const struct dc_posix_env *env,
                struct dc_error *err,
                const struct dc_fsm_info *info,
                int from_state_id,
                int to_state_id);
static void did_change_state(const struct dc_posix_env *env,
                         struct dc_error *err,
                         const struct dc_fsm_info *info,
                         int from_state_id,
                         int to_state_id,
                         int next_id);
static void bad_change_state(const struct dc_posix_env *env,
                         struct dc_error *err,
                         const struct dc_fsm_info *info,
                         int from_state_id,
                         int to_state_id);
static int state_error(const struct dc_posix_env *env, struct dc_error *err, void *arg);

static int readInput(const struct dc_posix_env *env, struct dc_error *err);
static int convertToMorse(const struct dc_posix_env *env, struct dc_error *err, void *arg);
static int writeToFile(const struct dc_posix_env *env, struct dc_error *err, void *arg);


enum application_states
{
    READ = DC_FSM_USER_START,    // 2
    CONVERT,
    WRITE,
    ERROR,
};

int main()
{
    struct dc_error err;
    struct dc_posix_env env;
    int ret_val;
    struct dc_fsm_info *fsm_info;
    static struct dc_fsm_transition transitions[] = {
            {DC_FSM_INIT,   READ,           readInput},
            {READ,          CONVERT,        convertToMorse},
            {CONVERT,       WRITE,          writeToFile},
            {WRITE,         DC_FSM_EXIT,    NULL},
            {READ,          ERROR,          state_error},
            {CONVERT,       ERROR,          state_error},
            {WRITE,         ERROR,          state_error},
            {ERROR,         DC_FSM_EXIT,    NULL}
    };

    dc_error_init(&err, error_reporter);
    dc_posix_env_init(&env, NULL /*trace_reporter*/);
    ret_val = EXIT_SUCCESS;
    fsm_info = dc_fsm_info_create(&env, &err, "toMorse");
//    dc_fsm_info_set_will_change_state(fsm_info, will_change_state);
//    dc_fsm_info_set_did_change_state(fsm_info, did_change_state);
//    dc_fsm_info_set_bad_change_state(fsm_info, bad_change_state);


    if(dc_error_has_no_error(&err))
    {
        int from_state;
        int to_state;

        ret_val = dc_fsm_run(&env, &err, fsm_info, &from_state, &to_state, NULL, transitions);
        dc_fsm_info_destroy(&env, &fsm_info);
    }

    return ret_val;
}

static int readInput(const struct dc_posix_env *env, struct dc_error *err) {
    char chars[BUF_SIZE];
    ssize_t nread;
    int ret_val;
    int next_state;

    ret_val = EXIT_SUCCESS;

    if (dc_error_has_no_error(err)) {
        while ( (nread = dc_read(env, err, STDIN_FILENO, chars, BUF_SIZE)) > 0) {
            
            if (dc_error_has_error(err)) {
                ret_val = 1;
            }

            dc_write(env, err, STDOUT_FILENO, chars, (size_t)nread);
        }
    }

    next_state = CONVERT;
    return next_state;
}

static int convertToMorse(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    int next_state;
    display("convert");
    next_state = WRITE;
    return next_state;
}
static int writeToFile(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    int next_state;
    display("write");
    next_state = DC_FSM_EXIT;
    return next_state;
}


static void error_reporter(const struct dc_error *err)
{
    fprintf(stderr, "Error: \"%s\" - %s : %s @ %zu\n", err->message, err->file_name, err->function_name, err->line_number);
}

static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number)
{
    fprintf(stderr, "Entering: %s : %s @ %zu\n", file_name, function_name, line_number);
}

static void will_change_state(const struct dc_posix_env *env,
                              struct dc_error *err,
                              const struct dc_fsm_info *info,
                              int from_state_id,
                              int to_state_id)
{
    printf("%s: will change %d -> %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id);
}

static void did_change_state(const struct dc_posix_env *env,
                             struct dc_error *err,
                             const struct dc_fsm_info *info,
                             int from_state_id,
                             int to_state_id,
                             int next_id)
{
    printf("%s: did change %d -> %d moving to %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id, next_id);
}

static void bad_change_state(const struct dc_posix_env *env,
                             struct dc_error *err,
                             const struct dc_fsm_info *info,
                             int from_state_id,
                             int to_state_id)
{
    printf("%s: bad change %d -> %d\n", dc_fsm_info_get_name(info), from_state_id, to_state_id);
}

static int state_error(const struct dc_posix_env *env, struct dc_error *err, void *arg)
{
    printf("ERROR\n");

    return DC_FSM_EXIT;
}

