#ifndef LLP1_DB_H
#define LLP1_DB_H

#include "structures.h"

void store_node_type_def_in_file(graph_db *db, node_type_def *node);

void store_schema_in_file(graph_db *db, schema *sch);

void store_attribute_def_list_in_file(graph_db *db, attribute_def *attr);

void store_relationship_def_list_in_file(graph_db *db, relationship_def *rel);

bool does_node_type_def_name_exist(schema *sch, char *name);

bool does_relationship_def_exist(node_type_def *connect_from, node_type_def *connect_to);

#endif //LLP1_DB_H
