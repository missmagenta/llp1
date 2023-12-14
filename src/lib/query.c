#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "query.h"
#include "file.h"


condition *create_condition_numeric(unsigned char operation, char *attr_name, float val) {
    condition *res = allocate_memory(sizeof(condition));
    operand_type *op1 = allocate_memory(sizeof(operand_type));
    operand_type *op2 = allocate_memory(sizeof(operand_type));
    op1->type = OP_NAME;
    op1->op_name = allocate_memory((strlen(attr_name) + 1) * sizeof(char));
    strcpy(op1->op_name, attr_name);
    op2->type = OP_NUMERIC;
    op2->op_numeric = val;
    res->type = operation;
    res->op1 = op1;
    res->op2 = op2;
    return res;
}

condition *create_string_attr_condition(unsigned char operation, char *attr_name, char *val) {
    condition *res = allocate_memory(sizeof(condition));
    operand_type *op1 = allocate_memory(sizeof(operand_type));
    operand_type *op2 = allocate_memory(sizeof(operand_type));
    op1->type = OP_NAME;
    op1->op_name = allocate_memory((strlen(attr_name) + 1) * sizeof(char));
    strcpy(op1->op_name, attr_name);
    op2->type = OP_STR;
    op2->op_str = allocate_memory((strlen(val) + 1) * sizeof(char));
    strcpy(op2->op_str, val);
    res->type = operation;
    res->op1 = op1;
    res->op2 = op2;
    return res;
}

condition *create_condition_condition(unsigned char operation, condition *op1, condition *op2) {
    condition *res = allocate_memory(sizeof(condition));
    operand_type *_op1 = allocate_memory(sizeof(operand_type));
    operand_type *_op2 = allocate_memory(sizeof(operand_type));
    _op1->type = OP_COND;
    _op1->op_cond = op1;
    _op2->type = OP_COND;
    _op2->op_cond = op2;
    res->type = operation;
    res->op1 = _op1;
    res->op2 = _op2;
    return res;
}

void delete_condition(condition *cond) {
    free_operand(cond->op1);
    free_operand(cond->op2);
    free_memory_counter(sizeof(*cond));
    free(cond);
}

void free_operand(operand_type *op) {
    switch (op->type) {
        case OP_COND:
            delete_condition(op->op_cond);
            break;
        case OP_STR:
            free_memory_counter(1 + strlen(op->op_str));
            free(op->op_str);
            break;
        case OP_NAME:
            free_memory_counter(1 + strlen(op->op_name));
            free(op->op_name);
            break;
    }
    free_memory_counter(sizeof(*op));
    free(op);
}

void free_node_set(graph_db *db, node *node_set) {
    while (node_set != NULL) {
        node *to_delete = node_set;
        node_set = node_set->next;
        free_memory_counter(sizeof(*to_delete));
        free(to_delete);
    }
}

int test_node_condition(graph_db *db, node_type_def *node, condition *cond) {
    operand_type left;
    operand_type right;
    int res;
    if (node->buffer_occupied_size == 0)
        return 0;
    if (cond == NULL)
        return 1;

    left = *cond->op1;
    if (cond->op1->type == OP_NAME) {
        int i;
        attribute_def *desc = search_attribute_def(node, cond->op1->op_name, &i);
        float val = get_attribute_value_of_node(node, cond->op1->op_name);
        switch (desc->type) {
            case ATTR_STR:
                left.type = OP_STR;
                left.op_str = string_get(db, val);
                break;
            default:
                left.type = OP_NUMERIC;
                left.op_numeric = val;
                break;
        }
    }

    right = *cond->op2;
    if (cond->op2->type == OP_NAME) {
        int i;
        attribute_def *desc = search_attribute_def(node, cond->op2->op_name, &i);
        float val = get_attribute_value_of_node(node, cond->op2->op_name);
        switch (desc->type) {
            case ATTR_STR:
                right.type = OP_STR;
                right.op_str = string_get(db, val);
                break;
            default:
                right.type = OP_NUMERIC;
                right.op_numeric = val;
                break;
        }
    }

    switch (cond->type) {
        case COND_EQUAL:
            if (left.type == right.type) {
                if (left.type == OP_STR)
                    res = strcmp(left.op_str, right.op_str) == 0;
                else
                    res = left.op_numeric == right.op_numeric;
            } else
                res = 0;
            break;
        case COND_NOT_EQUAL:
            if (left.type == right.type) {
                if (left.type == OP_STR)
                    res = strcmp(left.op_str, right.op_str) != 0;
                else
                    res = left.op_numeric != right.op_numeric;
            } else
                res = 0;
            break;
        case COND_LESS:
            if (left.type == right.type) {
                if (left.type == OP_STR)
                    res = strcmp(left.op_str, right.op_str) < 0;
                else
                    res = left.op_numeric < right.op_numeric;
            } else
                res = 0;
            break;
        case COND_GREATER:
            if (left.type == right.type) {
                if (left.type == OP_STR)
                    res = strcmp(left.op_str, right.op_str) > 0;
                else
                    res = left.op_numeric > right.op_numeric;
            } else
                res = 0;
            break;
        case COND_NOT:
            res = !test_node_condition(db, node, left.op_cond);
            break;
        case COND_AND:
            res = test_node_condition(db, node, left.op_cond) &&
                  test_node_condition(db, node, right.op_cond);
            break;
        case COND_OR:
            res = test_node_condition(db, node, left.op_cond) ||
                  test_node_condition(db, node, right.op_cond);
            break;
        default:
            break;
    }

    if (cond->op1->type == OP_NAME && left.type == OP_STR) {
        free_memory_counter(1 + strlen(left.op_str));
        free(left.op_str);
    }
    if (cond->op2->type == OP_NAME && right.type == OP_STR) {
        free_memory_counter(1 + strlen(right.op_str));
        free(right.op_str);
    }
    return res;
}

node *query_all_nodes_of_type(graph_db *db, node_type_def *node_type, condition *cond) {
    node *res = NULL;
    node *prev = NULL;
    restart_node_pointer(db, node_type);
    while (get_node(db, node_type)) {
        if (test_node_condition(db, node_type, cond)) {
            node *item = allocate_memory(sizeof(node));
            item->node = node_type;
            item->file_previous_element = node_type->file_previous_element;
            item->file_current_element = node_type->file_current_element;
            item->next = NULL;
            item->prev = prev;
            if (prev != NULL)
                prev->next = item;
            prev = item;
            if (res == NULL)
                res = item;
        }
        next_node(db, node_type);
    }
    return res;
}

node *query_node_set(graph_db *DB, node *node_set, condition *cond) {
    node *res = NULL;
    node *prev = NULL;
    if (node_set == NULL)
        return NULL;
    restart_node_pointer(DB, node_set->node);
    while (node_set != NULL && get_node(DB, node_set->node)) {
        if (node_set->node->file_current_element == node_set->file_current_element) {
            node_set->file_previous_element = node_set->node->file_previous_element;
            if (test_node_condition(DB, node_set->node, cond)) { //
                node *item = allocate_memory(sizeof(node));
                item->node = node_set->node;
                item->file_previous_element = node_set->node->file_previous_element;
                item->file_current_element = node_set->node->file_current_element;
                item->next = NULL;
                item->prev = prev;
                if (prev != NULL)
                    prev->next = item;
                prev = item;
                if (res == NULL)
                    res = item;
            }
            stop_edit_node(node_set->node);
            node_set = node_set->next;
        } else {
            stop_edit_node(node_set->node);
            next_node(DB, node_set->node);
        }
    }
    return res;
}

float *get_directed_to_list(graph_db *db, node_type_def *node, uint32_t *n) {
    attribute_def *a_list = node->attribute_def_first;
    uint32_t *buf;
    float *result = NULL;
    uint32_t offs = 0;
    while (a_list != NULL) {
        offs += sizeof(float);
        a_list = a_list->next;
    }
    buf = (uint32_t *) (node->buffer + offs);
    *n = *buf;
    if (*n != 0) {
        int i;
        int nbytes = 2 * (*n) * sizeof(float);
        allocate_memory(nbytes);
        for (i = 0; i < *n; i++) {
            unsigned char Type;
            db_fseek(db, buf[2 * i + 2] + sizeof(uint32_t), SEEK_SET);
            db_fread(&Type, sizeof(Type), 1, db);
            if (Type != REC_NODE) {
                result[2 * i] = 0.0;
                result[2 * i + 1] = 0.0;
            } else {
                result[2 * i] = buf[2 * i + 1];
                result[2 * i + 1] = buf[2 * i + 2];
            }
        }
    }
    return result;
}

node *_select_query(graph_db *db, uint32_t n_links, va_list args) {
    node *set;
    node_type_def *node_type;
    condition *cond;
    if (n_links == 0)
        return NULL;
    node_type = va_arg(args, node_type_def *);
    cond = va_arg(args, condition *);
    set = query_all_nodes_of_type(db, node_type, cond);
    n_links--;
    while (n_links > 0) {
        node_type_def *next_node;
        condition *next_cond;
        node *next_set_0 = NULL;
        node *next_set;
        node *set_ptr;
        node *prev = NULL;
        if (set == NULL)
            return NULL;
        next_node = va_arg(args, node_type_def *);
        next_cond = va_arg(args, condition *);
        set_ptr = set;
        while (set_ptr != NULL) {
            node_type->file_previous_element = set_ptr->file_previous_element;
            node_type->file_current_element = set_ptr->file_current_element;
            if (get_node(db, node_type)) {
                uint32_t i, n;
                float *links = get_directed_to_list(db, node_type, &n);
                for (i = 0; i < n; i++) {
                    if (links[2 * i] == next_node->file_root) {
                        node *item = allocate_memory(sizeof(node));
                        item->node = next_node;
                        item->file_previous_element = 0;
                        item->file_current_element = links[2 * i + 1];
                        item->next = NULL;
                        item->prev = prev;
                        if (prev != NULL)
                            prev->next = item;
                        prev = item;
                        if (next_set_0 == NULL)
                            next_set_0 = item;
                    }
                }
                stop_edit_node(node_type);
                free_memory_counter(2 * n * sizeof(float));
                free(links);
            }
            set_ptr = set_ptr->next;
        }
        if (next_set_0 == NULL)
            next_set = NULL;
        else
            next_set = query_node_set(db, next_set_0, next_cond);
        free_node_set(db, set);
        free_node_set(db, next_set_0);
        set = next_set;
        node_type = next_node;
        cond = next_cond;
        n_links--;
    }
    return set;
}

void get_node_instance_from_set(graph_db *db, node *node_set) {
    node_set->node->file_previous_element = node_set->file_previous_element;
    node_set->node->file_current_element = node_set->file_current_element;
}

node *select_query(graph_db *db, uint32_t n_links, ...) {
    node *res;
    va_list args;
    va_start(args, n_links);
    res = _select_query(db, n_links, args);
    va_end(args);
    return res;
}

void delete_query(graph_db *db, uint32_t n_links, ...) {
    node *set;
    node *set1;
    va_list args;
    va_start(args, n_links);
    set = _select_query(db, n_links, args);
    set1 = set;
    va_end(args);
    while (set != NULL && set->next != NULL)
        set = set->next;
    while (set != NULL) {
        get_node_instance_from_set(db, set);
        delete_node(db, set->node);
        set = set->prev;
    }
    free_node_set(db, set1);
}

void update_query(graph_db *db, char *attr_name, float attr_val, uint32_t n_links, ...) {
    node *set;
    node *set1;
    va_list args;
    va_start(args, n_links);
    set = _select_query(db, n_links, args);
    set1 = set;
    va_end(args);
    while (set != NULL) {
        get_node_instance_from_set(db, set);
        if (get_node(db, set->node)) {
            set_value_for_attribute_of_node(db, set->node, attr_name, attr_val);
            post_node(db, set->node);
        }
        set = set->next;
    }
    free_node_set(db, set1);
}