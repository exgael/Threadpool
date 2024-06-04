#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "processing.h"
#include "list.h"

#define SOURCE_DIR "/root/inf3173-tp3-h23"
#define BINARY_DIR "/root/inf3173-tp3-h23/build/bin"
int main() {
    fprintf(stderr, "Main start!\n");

    const char *img_serial_prefix = "img_s_";
    const char *img_multithread_prefix = "img_m_";
    const char *img_extension = ".png";

    struct list *work_list_serial = list_new(NULL, free_work_item);
    struct list *work_list_multihead = list_new(NULL, free_work_item);

    for (int i = 0; i < 101; i++) {
        char input_file_path[256];
        char output_file_path_serial[256];
        char output_file_path_multithread[256];

        snprintf(input_file_path, sizeof(input_file_path), "../../data/img_%04d.png", i);
        snprintf(output_file_path_serial, sizeof(output_file_path_serial), "%s%04d%s", img_serial_prefix, i, img_extension);
        snprintf(output_file_path_multithread, sizeof(output_file_path_multithread), "%s%04d%s", img_multithread_prefix, i, img_extension);

        struct work_item *item = (struct work_item *)calloc(1, sizeof(struct work_item));
        item->input_file = strdup(input_file_path);
        item->output_file = strdup(output_file_path_serial);
        struct list_node *n = list_node_new(item);
        list_push_back(work_list_serial, n);

        struct work_item *item2 = (struct work_item *)calloc(1, sizeof(struct work_item));
        item2->input_file = strdup(input_file_path);
        item2->output_file = strdup(output_file_path_multithread);
        struct list_node *n2 = list_node_new(item2);
        list_push_back(work_list_multihead, n2);
    }

    fprintf(stderr, "work item created and added to list!\n");

    struct timeval start_serial, end_serial, start_multithread, end_multithread;

    fprintf(stderr, "Serial!\n");

    gettimeofday(&start_serial, NULL);
    process_serial(work_list_serial);
    gettimeofday(&end_serial, NULL);


    fprintf(stderr, "Multihead!\n");
    gettimeofday(&start_multithread, NULL);
    process_multithread(work_list_multihead, sysconf(_SC_NPROCESSORS_ONLN));
    gettimeofday(&end_multithread, NULL);

    long duration_serial = (end_serial.tv_sec - start_serial.tv_sec) * 1000 + (end_serial.tv_usec - start_serial.tv_usec) / 1000;
    long duration_multithread = (end_multithread.tv_sec - start_multithread.tv_sec) * 1000 + (end_multithread.tv_usec - start_multithread.tv_usec) / 1000;

    printf("Temps d'exécution en série (secondes): %ld\n", duration_serial / 1000);
    printf("Temps d'exécution en multithread (secondes): %ld\n", duration_multithread / 1000);
    printf("Ratio d'accélération: %.2f\n", (double)duration_serial / duration_multithread);

    list_free(work_list_serial);
    list_free(work_list_multihead);
    return 0;
}

