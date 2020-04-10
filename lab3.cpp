#include "lab3.h"

//
// lab3 code should be located here!
//


unsigned int lab3_thread_graph_id() 
{
    return 999;
}

const char* lab3_unsynchronized_threads()
{
    return "xyz";
}

const char* lab3_sequential_threads()
{
    return "rst,tuvw";
}

int lab3_init()
{
    // ...
    
    CreateThread(...);
    // ...

    WaitForMultipleObjects(...);
    // ...
    
    return 0;
}
