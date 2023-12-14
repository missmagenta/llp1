#ifndef LLP1_QUERY_H
#define LLP1_QUERY_H

#include "structures.h"

enum condition_type {
    COND_LESS,
    COND_GREATER,
    COND_EQUAL,
    COND_NOT_EQUAL,
    COND_AND,
    COND_OR,
    COND_NOT
};

enum operand_type {
    OP_NUMERIC,
    OP_STR,
    OP_NAME,
    OP_COND
};

typedef struct {
    unsigned char type;
    union {
        struct condition *op_cond;
        char *op_str;
        float op_numeric;
        char *op_name;
    };
} operand_type;

typedef struct condition {
    unsigned char type;
    operand_type *op1;
    operand_type *op2;
} condition;

uint64_t string_init(graph_db *db, char *str);

char *string_get(graph_db *db, uint64_t offset);

condition *create_condition_numeric(unsigned char operation, char *name, float value);

condition *create_condition_string(unsigned char operation, char *name, char *value);

condition *create_condition_condition(unsigned char operation, condition *cond1, condition *cond2);

void delete_condition(condition *cond);

node *select_query(graph_db *db, uint32_t n_links, ...);

void free_node_set(graph_db *db, node *set);

void get_node_from_set(graph_db *db, node *set);

void delete_query(graph_db *db, uint32_t n_links, ...);

void update_query(graph_db *db, char *attr_name, float value, uint32_t n_links, ...);

void free_operand(operand_type *op);

int test_node_condition(graph_db *db, node_type_def *node, condition *cond);

node *query_all_nodes_of_type(graph_db *db, node_type_def *node, condition *cond);

node *query_node_set(graph_db *DB, node *node_set, condition *cond);

float *get_directed_to_list(graph_db *db, node_type_def *node, uint32_t *n);

#endif //LLP1_QUERY_H
