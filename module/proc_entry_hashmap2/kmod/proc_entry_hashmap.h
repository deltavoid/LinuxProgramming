#ifndef PROC_ENTRY_HASHMAP_H
#define PROC_ENTRY_HASHMAP_H




// define user interface

struct hello_entry_param
{
    long long operation;
    long long key;
    long long value;
};

enum hello_entry_operation
{
    HELLO_ENTRY_GET,
    HELLO_ENTRY_INSERT,
    HELLO_ENTRY_REMOVE,
};





#endif // PROC_ENTRY_HASHMAP_H