#include <stdio.h>

#include "structures.h"
#include "file.h"

void db_fflush(graph_db *db) {
    if (db->read_buf_pos < db->read_buf_occupied_size) {
        fseek(db->file, db->read_buf_pos - db->read_buf_occupied_size, SEEK_CUR);
        db->read_buf_pos = 0;
        db->read_buf_occupied_size = 0;
    }
    if (db->write_buf_occupied_size > 0) {
        fwrite(db->write_buf, 1, db->write_buf_occupied_size, db->file);
        fflush(db->file);
        db->write_buf_occupied_size = 0;
    }
}

void db_fwrite(void *buf, uint64_t item_size, int n_items, graph_db *db) {
    char *_buf = (char *) buf;
    int n_free = BUFFER_SIZE - db->write_buf_occupied_size;
    int n_bytes = item_size * n_items;
    int i = db->write_buf_occupied_size;
    if (db->read_buf_pos < db->read_buf_occupied_size) {
        fseek(db->file, db->read_buf_pos - db->read_buf_occupied_size, SEEK_CUR);
        db->read_buf_pos = 0;
        db->read_buf_occupied_size = 0;
    }
    if (n_free > 0) {
        int to_write = n_free < n_bytes ? n_free : n_bytes;
        db->write_buf_occupied_size += to_write;
        n_bytes -= to_write;
        for (; to_write > 0; to_write--, i++)
            db->write_buf[i] = *_buf++;
    }
    if (db->write_buf_occupied_size == BUFFER_SIZE) {
        fwrite(db->write_buf, 1, BUFFER_SIZE, db->file);
        fwrite(_buf, 1, n_bytes, db->file);
        db->write_buf_occupied_size = 0;
    }
}

void db_fread(void *buf, int item_size, int n_items, graph_db *db) {
    char *_buf = (char *) buf;
    int n_have = db->read_buf_occupied_size - db->read_buf_pos;
    int n_bytes = item_size * n_items;
    for (; n_bytes > 0 && n_have > 0; n_have--, n_bytes--)
        *_buf++ = db->read_buf[db->read_buf_pos++];
    if (n_bytes > 0) {
        fread(_buf, 1, n_bytes, db->file);
        db->read_buf_pos = 0;
        db->read_buf_occupied_size = fread(db->read_buf, 1, BUFFER_SIZE, db->file);
    }
}

void db_fseek(graph_db *db, long int offset, int whence) {
    db_fflush(db);
    fseek(db->file, offset, whence);
}

long int db_ftell(graph_db *db) {
    db_fflush(db);
    return ftell(db->file);
}

void db_fclose(graph_db *db) {
    db_fflush(db);
    fclose(db->file);
}
