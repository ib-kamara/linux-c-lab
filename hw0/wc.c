#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    int c;
    int lines = 0;
    int words = 0;
    int chars = 0;
    int in_word = 0;

    if (argc == 1) {
        fp = stdin;
    } else {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            return 1;
        }
    }

    while ((c = fgetc(fp)) != EOF) {
        chars++;

        if (c == '\n') {
            lines++;
        }

        if (c == ' ' || c == '\n' || c == '\t') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }

    printf("%d %d %d\n", lines, words, chars);

    if (fp != stdin) {
        fclose(fp);
    }

    return 0;
}
