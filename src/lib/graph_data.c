#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "structures.h"
#include "file.h"

void write_buffer(char *buffer, int *n_buffer, float what)
{
    char *w = (char *) &what;
    int i;
    buffer += *n_buffer;
    for (i = 0; i < sizeof(float); i++, buffer++, (*n_buffer)++)
    {
        *buffer = w[i];
    }
}

void stop_edit_node(node_type_def *node) {
    node->buffer_occupied_size = 0;
    node->adding = 0;
}

void create_node(graph_db *db, node_type_def *node) {
    attribute_def *cur_attribute_def = node->attribute_def_first;
    int i;
    stop_edit_node(node);
    node->buffer_occupied_size = 0;
    node->adding = 1;
    while (cur_attribute_def != NULL) {
        write_buffer(node->buffer, &node->buffer_occupied_size, 0.0);
        cur_attribute_def = cur_attribute_def->next;
    }
    write_buffer(node->buffer, &node->buffer_occupied_size, 0.0);
}

void post_node(graph_db *db, node_type_def *node) {
    if (node->buffer_occupied_size > 0) {
        if (node->adding) {
            int n = sizeof(int) + sizeof(unsigned char) + sizeof(int) + node->buffer_occupied_size;
            unsigned char type = REC_NODE;
            int empty_offset = 0;
            db_fseek(db, 0, SEEK_END);
            node->file_current_element = db_ftell(db);
            db_fwrite(&n, sizeof(n), 1, db);
            db_fwrite(&type, sizeof(type), 1, db);
            db_fwrite(&empty_offset, sizeof(empty_offset), 1, db);
            db_fwrite(node->buffer, node->buffer_occupied_size, 1, db);
            node->file_previous_element = node->file_last_element;
            if (node->file_first_element == 0 && node->file_last_element == 0) {
                db_fseek(db, node->file_root, SEEK_SET);
                db_fwrite(&node->file_current_element, sizeof(int), 1, db);
                db_fwrite(&node->file_current_element, sizeof(int), 1, db);
                node->file_first_element = node->file_current_element;
                node->file_last_element = node->file_current_element;
            } else {
                db_fseek(db, node->file_previous_element + sizeof(int) + sizeof(unsigned char), SEEK_SET);
                db_fwrite(&node->file_current_element, sizeof(int), 1, db);
                db_fseek(db, node->file_root + sizeof(int), SEEK_SET);
                db_fwrite(&node->file_current_element, sizeof(int), 1, db);
                node->file_last_element = node->file_current_element;
            }
            node->adding = 0;
        } else {
            db_fseek(db, node->file_current_element + sizeof(int) + sizeof(unsigned char) + sizeof(int), SEEK_SET);
            db_fwrite(node->buffer, node->buffer_occupied_size, 1, db);
        }
        db_fflush(db);
    }
}

void set_value_for_attribute_of_node(graph_db *db, node_type_def *node, char *attribute_def_name, float value) {
    int n;
    if (node->buffer_occupied_size > 0 && search_attribute_def(node, attribute_def_name, &n)) {
        n *= sizeof(float);
        write_buffer(node->buffer, &n, value);
    }
}

float get_attribute_value_of_node(node_type_def *node, char *attribute_def_name) {
    int n;
    if (node->buffer_occupied_size > 0 && search_attribute_def(node, attribute_def_name, &n)) {
        float *buf = (float *) node->buffer;
        return buf[n];
    } else
        return 0.0;
}

uint64_t string_init(graph_db *db, char *s) {
    unsigned char Type = REC_STRING; // обозначение типа записи
    uint64_t length = strlen(s); // длина строки
    uint64_t n = sizeof(uint64_t) + sizeof(unsigned char) + 1 + length; // размер записи, включаем туда длину строки, тип записи и нулевой символ
    uint64_t result; // смещение от начала файла до записи строки
    db_fseek(db, 0, SEEK_END); // переходим в конец файла
    result = db_ftell(db); // запоминаем смещение от начала файла до конца
    db_fwrite(&n, sizeof(n), 1, db); // записываем размер записи
    db_fwrite(&Type, sizeof(Type), 1, db); // записываем тип записи
    db_fwrite(s, length + 1, 1, db); // записываем строку
    db_fflush(db);
    return result;
}

char *string_get(graph_db *db, uint64_t offset) {
    unsigned char type;
    char *result;
    uint64_t length; // длина строки
    uint64_t n; // размер записи
    db_fseek(db, offset, SEEK_SET); // переходим по указанному смещению
    db_fread(&n, sizeof(n), 1, db); // считываем размер записи
    db_fread(&type, sizeof(type), 1, db);
    if (type != REC_STRING)
        return NULL;
    length = n - sizeof(int) - sizeof(unsigned char); // вычисляем длину строки
    result = allocate_memory(length); // выделяем память под строку
    db_fread(result, length, 1, db); // считываем строку
    return result;
}

int get_node(graph_db *db, node_type_def *node) {
    unsigned char type;
    int dummy;
    int n;
    stop_edit_node(node);
    if (node->file_current_element == 0)
        return 0;
    node->buffer_occupied_size = 0;
    node->adding = 0;
    db_fseek(db, node->file_current_element, SEEK_SET);
    db_fread(&n, sizeof(n), 1, db);
    db_fread(&type, sizeof(type), 1, db);
    if (type != REC_NODE)
        return 0;
    db_fread(&dummy, sizeof(int), 1, db);
    node->buffer_occupied_size = n - sizeof(n) - sizeof(type) - sizeof(int);
    db_fread(node->buffer, node->buffer_occupied_size, 1, db);
    return 1;
}

int link_nodes(graph_db *db, node_type_def *from_node, node_type_def *to_node) {
    attribute_def *list_attribute_defs = from_node->attribute_def_first;
    relationship_def *list_rel = from_node->relationship_def_first; // список связей
    float *buf;
    int n, i;
    int offset = 0;
    while (list_rel != NULL && list_rel->node_connect != to_node) {
        list_rel = list_rel->next;
    }
    if (list_rel == NULL)
        return 0;

    while (list_attribute_defs != NULL) {
        offset += sizeof(float);
        list_attribute_defs = list_attribute_defs->next;
    }

    buf = (float *) (from_node->buffer + offset); // буфер для хранения списка связей
    n = *buf;
    for (i = 0; i < n; i++) { // проверяем, нет ли уже такой связи
        if (buf[2 * i + 1] == to_node->file_root && buf[2 * i + 2] == to_node->file_current_element) {
            return 1;
        }
    }
    (*buf)++;
    buf[2 * n + 1] = to_node->file_root; // записываем в буфер связь
    buf[2 * n + 2] = to_node->file_current_element;
    return 1;
}

void restart_node_pointer(graph_db *db, node_type_def *node) {
    stop_edit_node(node);
    node->file_current_element = node->file_first_element;
    node->file_previous_element = 0;
}

bool next_node(graph_db *db, node_type_def *node) {
    stop_edit_node(node);
    if (node->file_first_element == 0 || node->file_last_element == 0 || node->file_current_element == 0)
        return false;
    db_fseek(db, node->file_current_element + sizeof(int) + sizeof(unsigned char), SEEK_SET);
    node->file_previous_element = node->file_current_element;
    db_fread(&node->file_current_element, sizeof(int), 1, db);
    return true;
}

int delete_node(graph_db *db, node_type_def *node) {
    unsigned char Type = REC_EMPTY; // Type of record
    int after_deleted_offset; // Offset of the next node
    stop_edit_node(node); // Cancel editing the node
    if (node->file_first_element == 0 || node->file_last_element == 0 || node->file_current_element == 0)
        return 0; // If the node is empty, return 0
    db_fseek(db, node->file_current_element + sizeof(int), SEEK_SET); // Seek to the beginning of the node
    db_fwrite(&Type, sizeof(Type), 1, db); // Write the type of record
    db_fflush(db); // Flush the database
    db_fread(&after_deleted_offset, sizeof(after_deleted_offset), 1, db); // Read the offset of the next node
    if (node->file_first_element == node->file_last_element) // If the node is the only node
    {
        int empty_offset = 0; // Offset of the next node
        db_fseek(db, node->file_root, SEEK_SET); // Seek to the beginning of the node
        db_fwrite(&empty_offset, sizeof(empty_offset), 1, db); // Write the offset of the next node
        node->file_first_element = 0; // Set the first element to 0
        db_fwrite(&empty_offset, sizeof(empty_offset), 1, db);
        node->file_last_element = 0;
        node->file_previous_element = 0;
        node->file_current_element = 0;
    } else if (node->file_first_element == node->file_current_element) // If the node is the first node
    {
        db_fseek(db, node->file_root, SEEK_SET); // Seek to the beginning of the node
        db_fwrite(&after_deleted_offset, sizeof(after_deleted_offset), 1, db); // Write the offset of the next node
        node->file_first_element = after_deleted_offset; // Set the first element to the offset of the next node
        node->file_previous_element = 0;
        node->file_current_element = after_deleted_offset;
    } else if (node->file_last_element == node->file_current_element) // If the node is the last node
    {
        int empty_offset = 0; // Offset of the next node
        db_fseek(db, node->file_previous_element + sizeof(int) + sizeof(unsigned char),
                 SEEK_SET); // Seek to the beginning of the node
        db_fwrite(&empty_offset, sizeof(empty_offset), 1, db); // Write the offset of the next node
        db_fseek(db, node->file_root + sizeof(int), SEEK_SET); // Seek to the beginning of the node
        db_fwrite(&node->file_previous_element, sizeof(int), 1, db); // Write the offset of the next node
        node->file_last_element = node->file_previous_element; // Set the last element to the offset of the previous node
        node->file_previous_element = 0; // Set the previous element to 0
        node->file_current_element = 0; // Set the current element to 0
    } else // If the node is in the middle
    {
        db_fseek(db, node->file_previous_element + sizeof(int) + sizeof(unsigned char),
                 SEEK_SET); // Seek to the beginning of the node
        db_fwrite(&after_deleted_offset, sizeof(after_deleted_offset), 1, db); // Write the offset of the next node
        node->file_current_element = after_deleted_offset; // Set the current element to the offset of the next node
    }
    db_fflush(db);
    return 1;
}
