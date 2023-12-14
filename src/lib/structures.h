#ifndef LLP1_STRUCTURES_H
#define LLP1_STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


typedef struct attribute_def { // структура для хранения описания атрибута
    char *name; // имя атрибута
    char type; // тип атрибута
    struct attribute_def *next; // указатель на следующий атрибут
} attribute_def;


enum attribute_def_type { // типы атрибутов
    ATTR_INT,
    ATTR_FLOAT,
    ATTR_BOOL,
    ATTR_STR
};


typedef struct relationship_def { // структура для хранения описания связей между узлами
    struct node_type_def *node_connect; // указатель на тип узла, к которому осуществлена связь
    struct relationship_def *next; // указатель на следующую связь
} relationship_def;


typedef struct node_type_def { // структура для хранения описания типа узла
    struct node_type_def *next; // указатель на следующий тип узла
    char *name; // имя типа узла
    attribute_def *attribute_def_first;
    attribute_def *attribute_def_last;
    relationship_def *relationship_def_first; // указатель на первую связь
    relationship_def *relationship_def_last; // указатель на последнюю связь
    uint64_t file_root; // смещение от начала файла до списка узлов данного типа
    uint64_t file_first_element; // смещение от начала файла до первого элемента данного типа
    uint64_t file_last_element; // смещение от начала файла до последнего элемента данного типа

    char *buffer; // буфер с данными
    int buffer_occupied_size; // размер занятой части буфера
    uint64_t adding; // флаг, указывающий, что в данный момент идет добавление нового элемента
    uint64_t file_current_element; // смещение от начала файла до текущего элемента
    uint64_t file_previous_element; // смещение от начала файла до предыдущего элемента
} node_type_def;


typedef struct node { // структура для хранения самого узла
    node_type_def *node; // указатель на тип узла
    uint64_t file_previous_element; // смещение от начала файла до предыдущего элемента
    uint64_t file_current_element; // смещение от начала файла до текущего элемента
    struct node *next; // указатель на следующий узел
    struct node *prev; // указатель на предыдущий узел
} node;


typedef struct schema { // структура для хранения схемы базы данных
    node_type_def *first; // указатель на первый тип узла
    node_type_def *last; // указатель на последний тип узла
} schema;


typedef struct graph_db { // структура для хранения базы данных
    schema *schema; // указатель на схему
    char *filename;
    FILE *file;
    char *read_buf; // буфер для чтения
    uint64_t read_buf_pos; // позиция в буфере для чтения
    uint64_t read_buf_occupied_size; // размер занятой части буфера для чтения
    char *write_buf; // буфер для записи
    uint64_t write_buf_occupied_size; // размер занятой части буфера для записи
} graph_db;


attribute_def *create_attribute_def(node_type_def *node_type, char *name, char type); // создание описания атрибута

void delete_attribute_def(attribute_def *attr); // удаление описания атрибута

relationship_def *create_relationship_def(node_type_def *connect_from, node_type_def *connect_to); // создание описания связи

void delete_relationship_def(relationship_def *rel); // удаление описания связи

node_type_def *create_node_type_def(schema *schema, char *name); // создание описания типа узла

void delete_node_type_def(node_type_def *node_type); // удаление описания типа узла

void create_node(graph_db *db, node_type_def *node_type); // создание узла

int delete_node(graph_db *db, node_type_def *node_type); // удаление узла

void post_node(graph_db *db, node_type_def *node_type); // добавление узла в базу данных

int get_node(graph_db *db, node_type_def *node_type); // получение узла из базы данных

schema *create_schema(); // создание схемы

void delete_schema(schema *schema); // удаление схемы

graph_db *create_graph_db(schema *schema, char *filename); // создание базы данных

void delete_graph_db(graph_db *db); // удаление базы данных

bool next_node(graph_db *db, node_type_def *node_type); // переход к следующему узлу

void restart_node_pointer(graph_db *db, node_type_def *node_type); // переход к первому узлу

void set_value_for_attribute_of_node(graph_db *db, node_type_def *node_type, char *attr_name, float value); // установка значения атрибута узла

float get_attribute_value_of_node(node_type_def *node_type, char *attr_name); // получение значения атрибута узла

attribute_def *search_attribute_def(node_type_def *node, char *name, int *num); // поиск описания атрибута

#endif //LLP1_STRUCTURES_H
