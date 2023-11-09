#define newA(__E,__n) (__E*) malloc((__n)*sizeof(__E))

typedef unsigned int uintE;
typedef unsigned int uintT;

#define MAX_ARRAYS_SIZE MAX_SUB_GRAPH_STRUCT_SIZE - 128
#define MAX_SUB_GRAPH_STRUCT_SIZE 1024 * 32

#define READ_INPUT_FILE_BUFFER_SIZE 1024 * 16