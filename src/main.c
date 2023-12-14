#include "lib/query.h"
#include "lib/structures.h"
#include "lib/file.h"
#include <time.h>
#include <ntdef.h>
#include <profileapi.h>

size_t memory_used = 0;
double time_seconds_counter = 0;


double time_seconds() {
#ifdef _WIN32
    // Windows
    LARGE_INTEGER frequency, time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);
    return (double)time.QuadPart / (double)frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#endif
}

void memory_leak_check() {
    printf(get_memory_used() == 0 ? "Memory freed successfully.\n" : "Memory leak occurred. %6zu bytes allocated.",
            get_memory_used());
}


double get_time() {
    double time = time_seconds() - time_seconds_counter;
    time_seconds_counter = time_seconds();
    return time;
}




#define INSERTION_TOTAL 400000
#define INSERTION_CHECKPOINT 2000

void test_insertion() {
    FILE *csv_file = fopen("D://VT-3-year//low-level//llp1//src//results_insertion.csv", "w");
    if (csv_file == NULL) {
        printf("Error opening file.\n");
        return;
    } else {
        printf("File opened successfully.\n");
    }
    fprintf(csv_file, "Iteration,Time,MemoryUsed\n");
    printf("---------------------\nINSERTION\n---------------------\n");
    schema *sch = create_schema();
    node_type_def *node1 = create_node_type_def(sch, "type_1");
    create_attribute_def(node1, "attribute", ATTR_STR);
    graph_db *db = create_graph_db(sch, "database.db");

    get_time();
    for (int i = 0; i < INSERTION_TOTAL; i++) {
        create_node(db, node1);
        char *str = malloc(sizeof(char) * (rand() % 1000 + 1));
        set_value_for_attribute_of_node(db, node1, "attribute", string_init(db, str));
        post_node(db, node1);

        if ((i + 1) % INSERTION_CHECKPOINT == 0) {
            fprintf(csv_file, "Items: %6d; Time: %9.6f; Memory: %6zu\n", i + 1, get_time(), get_memory_used());
            printf("Items: %6d; Time: %9.6f; Memory: %6zu\n", i + 1, get_time(), get_memory_used());
//            printf("%6d, %6ld\n", i + 1, get_memory_used());
//            printf("%6d, %9.6f\n", i + 1, get_time());
        }
    }
    fclose(csv_file);
    delete_graph_db(db);
    memory_leak_check();
}



#define DELETION_TOTAL 40000
#define DELETION_CHECKPOINT 200

void test_deletion() {
    FILE *csv_file = fopen("D://VT-3-year//low-level//llp1//src//results_delete.csv", "w");
    if (csv_file == NULL) {
        printf("Error opening file.\n");
        return;
    } else {
        printf("File opened successfully.\n");
    }
    fprintf(csv_file, "Iteration,Time,MemoryUsed\n");
    printf("---------------------\nDELETION\n---------------------\n");
    schema *sch = create_schema();
    node_type_def *node1 = create_node_type_def(sch, "type_1");
    create_attribute_def(node1, "attribute", ATTR_STR);
    graph_db *db = create_graph_db(sch, "database.db");

    for (int i = 0; i < DELETION_TOTAL; i += DELETION_CHECKPOINT) {
        for (int i2 = 0; i2 < i; i2++) {
            create_node(db, node1);
            char *str = malloc(sizeof(char) * (rand() % 1000 + 1));
            set_value_for_attribute_of_node(db, node1, "attribute", string_init(db, str));
            post_node(db, node1);
        }
        get_time();

        delete_query(db, 1, node1, NULL);

        fprintf(csv_file, "Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
          printf("Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
//        printf("%6d, %9.6f\n", i, get_time());
//        printf("%6d, %6ld\n", i, get_memory_used());
    }

    fclose(csv_file);
    delete_graph_db(db);
    memory_leak_check();
}



#define SELECTION_TOTAL 20000
#define SELECTION_CHECKPOINT 200

void test_selection() {
    FILE *csv_file = fopen("D://VT-3-year//low-level//llp1//src//results_selection.csv", "w");
    if (csv_file == NULL) {
        printf("Error opening file.\n");
        return;
    } else {
        printf("File opened successfully.\n");
    }
    fprintf(csv_file, "Iteration,Time,MemoryUsed\n");
    printf("---------------------\nSELECTION\n---------------------\n");
    schema *sch = create_schema();
    node_type_def *node1 = create_node_type_def(sch, "type_1");
    create_attribute_def(node1, "attr", ATTR_INT);
    graph_db *db = create_graph_db(sch, "database.db");

    condition *cond = create_condition_numeric(COND_LESS, "attr", 10);

    for (int i = 0; i < SELECTION_TOTAL; i += SELECTION_CHECKPOINT) {
        for (int i2 = 0; i2 < i; i2++) {
            create_node(db, node1);
            set_value_for_attribute_of_node(db, node1, "attr", 5);
            post_node(db, node1);
        }
        get_time();
        node *set = select_query(db, 1, node1, cond);
//        printf("%6d, %9.6f\n", i + 1, get_time());
//        printf("%6d, %6ld\n", i, get_memory_used());
        printf("Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
        fprintf(csv_file,"Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
        free_node_set(db, set);
        delete_query(db, 1, node1, NULL);
    }

    fclose(csv_file);
    delete_condition(cond);
    delete_graph_db(db);
    memory_leak_check();
}



#define UPDATE_TOTAL 20000
#define UPDATE_CHECKPOINT 200

void test_update() {
    FILE *csv_file = fopen("D://VT-3-year//low-level//llp1//src//results_update.csv", "w");
    if (csv_file == NULL) {
        printf("Error opening file.\n");
        return;
    } else {
        printf("File opened successfully.\n");
    }
    fprintf(csv_file, "Iteration,Time,MemoryUsed\n");
    printf("---------------------\nUPDATE\n---------------------\n");
    schema *sch = create_schema();
    node_type_def *node1 = create_node_type_def(sch, "type_1");
    create_attribute_def(node1, "attribute", ATTR_STR);
    graph_db *db = create_graph_db(sch, "database.db");

    for (int i = 0; i < UPDATE_TOTAL; i += UPDATE_CHECKPOINT) {
        for (int i2 = 0; i2 < i; i2++) {
            create_node(db, node1);
            set_value_for_attribute_of_node(db, node1, "attribute", string_init(db, "Hello world"));
            post_node(db, node1);
        }
        get_time();
        update_query(db, "attribute", string_init(db, "Goodbye planet"), 1, node1, NULL);
//        printf("%6d, %9.6f\n", i + 1, get_time());
//        printf("%6d, %6ld\n", i + 1, get_memory_used());
        fprintf(csv_file,"Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
        printf("Items: %6d; Time: %9.6f; Memory: %6zu\n", i, get_time(), get_memory_used());
        delete_query(db, 1, node1, NULL);
    }

    fclose(csv_file);
    delete_graph_db(db);
    memory_leak_check();
}



int main() {
//    test_insertion();
    test_deletion();
//    test_selection();
//    test_update();
}