#ifndef LLP1_FILE_H
#define LLP1_FILE_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#define BUFFER_SIZE (1024 * 32) // фиксированный размер буфера для работы с файлом


extern size_t memory_used;

enum file_record_type {
    REC_EMPTY,
    REC_STRING, // для удобства хранения строк произвольной длины
    REC_NODE
};


void free_memory_counter(size_t size);

void *allocate_memory(size_t size);

size_t get_memory_used();

void db_fflush(graph_db *db);

void db_fwrite(void *buf, uint64_t item_size, int n_items, graph_db *db);

void db_fread(void *buf, int item_size, int n_items, graph_db *db);

void db_fseek(graph_db *db, long int offset, int whence);

long int db_ftell(graph_db *db);

void db_fclose(graph_db *db);

void write_buffer(char *buffer, int *n_buffer, float what);

void stop_edit_node(node_type_def *node);

#endif //LLP1_FILE_H