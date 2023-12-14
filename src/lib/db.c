#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "db.h"
#include "file.h"


void *allocate_memory(size_t size) {
    memory_used += size;
    return calloc(1, size);
}


void free_memory_counter(size_t size) {
    memory_used -= size;
}

size_t get_memory_used() {
    return memory_used;
}

void delete_node_type_def(node_type_def *node) {
    memory_used -= strlen(node->name) + 1;
    free(node->name);

    attribute_def *attr_def = node->attribute_def_first;
    while (attr_def) {
        attribute_def *attr = attr_def;
        attr_def = attr_def->next;
        delete_attribute_def(attr);
    }

    relationship_def *rel_def = node->relationship_def_first;
    while (rel_def) {
        relationship_def *rel = rel_def;
        rel_def = rel_def->next;
        delete_relationship_def(rel);
    }

    memory_used -= BUFFER_SIZE;
    free(node->buffer);
    memory_used -= sizeof(node_type_def);
    free(node);
}

schema *create_schema() {
    schema *sch = allocate_memory(sizeof(schema));
    sch->first = NULL;
    sch->last = NULL;
    return sch;
}


void delete_schema(schema *sch) {
    node_type_def *i = sch->first;
    while (i) {
        node_type_def *node = i;
        i = i->next;
        delete_node_type_def(node);
    }
    memory_used -= sizeof(schema);
    free(sch);
}

void store_attribute_def_list_in_file(graph_db *db, attribute_def *attr) {
    uint64_t name_length = 0;
    while (attr) {
        name_length = 1 + strlen(attr->name);
        db_fwrite(&name_length, sizeof(name_length), 1, db);
        db_fwrite(attr->name, name_length, 1, db);
        db_fwrite(&attr->type, sizeof(attr->type), 1, db);
        attr = attr->next;
    }
    name_length = 0;
    db_fwrite(&name_length, sizeof(name_length), 1, db);
}

void store_relationship_def_list_in_file(graph_db *db, relationship_def *rel) {
    while (rel) {
        db_fwrite(&rel->node_connect, sizeof(rel->node_connect), 1, db);
        rel = rel->next; // get the next relationship definition
    }
    db_fwrite(&rel, sizeof(rel), 1, db);
}

void store_node_type_def_in_file(graph_db *db, node_type_def *node) {
    uint64_t name_length = 1 + strlen(node->name);
    db_fwrite(&node, sizeof(node), 1, db);
    db_fwrite(&name_length, sizeof(name_length), 1, db);
    db_fwrite(node->name, name_length, 1, db);
    store_attribute_def_list_in_file(db, node->attribute_def_first);
    store_relationship_def_list_in_file(db, node->relationship_def_first);
}


void store_schema_in_file(graph_db *db, schema *sch) {
    node_type_def *i = sch->first; // получаем первый элемент списка
    while (i) {
        store_node_type_def_in_file(db, i); // записываем в файл
        i = i->next;
    }
    db_fwrite(&i, sizeof(i), 1, db);
    i = sch->first;
    while (i) {
        int offset_empty = 0;
        i->file_root = db_ftell(db);
        db_fwrite(&offset_empty, sizeof(offset_empty), 1, db);
        i = i->next;
    }
}


bool does_node_type_def_name_exist(schema *sch, char *name) {
    node_type_def *i = sch->first;
    while (i) {
        if (strcmp(name, i->name) == 0)
            return true;
        i = i->next;
    }
    return false;
}


node_type_def *create_node_type_def(schema *sch, char *name) {
    if (does_node_type_def_name_exist(sch, name))
        return NULL;

    node_type_def *ret = allocate_memory(sizeof(node_type_def));

    if (!(sch->first) || !(sch->last))
        sch->first = ret;
    else
        sch->last->next = ret;
    sch->last = ret;

    ret->name = allocate_memory(1 + strlen(name) * sizeof(char));
    strcpy(ret->name, name);

    ret->next = NULL;
    ret->attribute_def_first = NULL;
    ret->attribute_def_last = NULL;
    ret->relationship_def_first = NULL;
    ret->relationship_def_last = NULL;

    ret->file_root = 0;
    ret->file_first_element = 0;
    ret->file_last_element = 0;
    ret->buffer = allocate_memory(BUFFER_SIZE * sizeof(char));
    ret->buffer_occupied_size = 0;
    ret->adding = 0;
    ret->file_current_element = 0;
    ret->file_previous_element = 0;

    return ret;
}


attribute_def *search_attribute_def(node_type_def *node, char *name, int *num) {
    attribute_def *i = node->attribute_def_first;
    *num = 0;
    while (i) {
        if (strcmp(name, i->name) ==
            0)
            return i;
        (*num)++;
        i = i->next;
    }
    *num = -1;
    return NULL;
}


attribute_def *create_attribute_def(node_type_def *node, char *name, char type) {
    int num;
    if (search_attribute_def(node, name, &num))
        return NULL;

    attribute_def *ret = allocate_memory(sizeof(attribute_def));

    if (node->attribute_def_first == NULL || node->attribute_def_last == NULL)
        node->attribute_def_first = ret;
    else
        node->attribute_def_last->next = ret;
    node->attribute_def_last = ret;

    ret->name = allocate_memory(1 + strlen(name) * sizeof(char));
    strcpy(ret->name, name);
    ret->type = type;
    ret->next = NULL;

    return ret;
}


void delete_attribute_def(attribute_def *attr) {
    memory_used -= strlen(attr->name) + 1;
    free(attr->name);
    memory_used -= sizeof(attribute_def);
    free(attr);
}


bool does_relationship_def_exist(node_type_def *connect_from, node_type_def *connect_to) {
    relationship_def *i = connect_from->relationship_def_first;
    while (i) {
        if (connect_to == i->node_connect)
            return true;
        i = i->next;
    }
    return false;
}


relationship_def *create_relationship_def(node_type_def *connect_from, node_type_def *connect_to) {
    if (does_relationship_def_exist(connect_from, connect_to))
        return NULL;

    relationship_def *ret = allocate_memory(sizeof(relationship_def));
    ret->next = NULL;
    ret->node_connect = connect_to;

    if (connect_from->relationship_def_first == NULL ||
        connect_from->relationship_def_last == NULL)
        connect_from->relationship_def_first = ret;
    else
        connect_from->relationship_def_last->next = ret;
    connect_from->relationship_def_last = ret;

    return ret;
}


void delete_relationship_def(relationship_def *rel) {
    memory_used -= sizeof(relationship_def);
    free(rel);
}


graph_db *create_graph_db(schema *sch, char *filename) {
    graph_db *graph = allocate_memory(sizeof(graph_db));
    graph->file = fopen(filename, "w+b");

    if (!(graph->file)) {
        memory_used -= sizeof(graph_db);
        free(graph);
        return NULL;
    }

    graph->schema = sch;
    graph->write_buf = allocate_memory(BUFFER_SIZE);
    graph->write_buf_occupied_size = 0;
    graph->read_buf = allocate_memory(BUFFER_SIZE);
    graph->read_buf_pos = 0;
    graph->read_buf_occupied_size = 0;

    db_fseek(graph, sizeof(int32_t), SEEK_SET);
    store_schema_in_file(graph, sch);
    int32_t schema_length = db_ftell(graph);
    db_fseek(graph, 0, SEEK_SET);
    db_fwrite(&schema_length, sizeof(schema_length), 1, graph);
    db_fflush(graph);
    return graph;
}


void delete_graph_db(graph_db *db) {
    delete_schema(db->schema);
    db_fclose(db);
    free(db->read_buf);
    free(db->write_buf);
    memory_used -= 2 * BUFFER_SIZE;
    free(db);
    fprintf(stdout, "Memory used: %zu\n", memory_used);
    memory_used -= sizeof(*db);
}
