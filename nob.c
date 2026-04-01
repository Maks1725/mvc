#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_DIR "build/"
#define SRC_DIR "src/"

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return 1;

    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, SRC_DIR"main.c", SRC_DIR"config.c");
    nob_cmd_append(&cmd, "-o", BUILD_DIR"mvc");
    nob_cmd_append(&cmd, "-Wall", "-Wextra");
    nob_cmd_append(&cmd, "-lraylib", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11");

    if (!nob_cmd_run(&cmd)) return 1;
    
    return 0;
}
