#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))

typedef unsigned int uintE;
typedef unsigned int uintT;

#define MAX_BUF_SIZE MAX_SUB_GRAPH_STRUCT_SIZE - 8
#define MAX_ARRAYS_SIZE MAX_SUB_GRAPH_STRUCT_SIZE
#define MAX_SUB_GRAPH_STRUCT_SIZE 1024 * 32

#define FLUSH_EDGE_NUM_LIMIT 1024 * 16

#define READ_INPUT_FILE_BUFFER_SIZE 1024 * 16

#define MAX_EDGE_NUM 1000000000 // 10000000

#define MAX_VERTEX_ID 4847570