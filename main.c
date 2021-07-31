#include <stdlib.h>
#include <stdio.h>
#include "blin.h"
#include "linenoise/linenoise.h"

i32 result[] = { 0, 0, 0, 0 };  // where to put result

int main() {
    char *line;

    linenoiseHistoryLoad(".blin_history"); /* Load the history at startup */
    while((line = linenoise("> ")) != NULL) {
        if (line[0] == '\\' || line[0] == '\0') break;
        if (line[0] != '\0') {
            linenoiseHistoryAdd(line);
            linenoiseHistorySave(".blin_history");

            run(compile((u8*)line), result);
            printf("result: %d %d %d %d\n", result[0], result[1], result[2], result[3]);
        }
        free(line);
    }
    return 0;
}
