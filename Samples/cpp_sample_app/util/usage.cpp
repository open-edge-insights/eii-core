#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "eis/msgbus/msgbus.h"
#include "eis/utils/logger.h"
#include "eis/utils/json_config.h"
#include <eis/config_manager/env_config.h>

void usage(const char* name) {
    fprintf(stderr, "usage: %s [-h|--help] <json-config> [topic]\n", name);
    fprintf(stderr, "\t-h|--help   - Show this help\n");
    fprintf(stderr, "\tjson-config - Path to JSON configuration file\n");
    fprintf(stderr, "\ttopic       - (Optional) Topic string "\
                    "(df: publish_test)\n");
}
