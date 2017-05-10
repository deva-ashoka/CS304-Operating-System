#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    FILE *file = fopen(argv[1], "r");
    int n;
    fscanf(file, "%d", &n);
    int integers[n];

    for (int i = 0; i < n; i++) {
        int num;
        (fscanf(file, "%d", &num));
        integers[i] = num;
    }
    fclose(file);

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (integers[i] > integers[j]) {
                int a = integers[i];
                integers[i] = integers[j];
                integers[j] = a;
            }
        }
    }

    FILE *outputFile = fopen("output.txt", "w");
    for (int i = 0; i < n; i++) {
        printf("%d\n", integers[i]);
        fprintf(outputFile, "%d\n", integers[i]);
    }
    fclose(outputFile);

    sleep(30);

    return 1;
}