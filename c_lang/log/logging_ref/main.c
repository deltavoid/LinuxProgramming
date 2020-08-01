#include<stdio.h>

#include "debug_log.h"


int main(int argc, const char *argv[])
{
    Msg_Info("test!\n");
    Msg_Warn("%d\n",10);
    Msg_Error("%s\n","error");
    Msg_Debug("Debug\n");
    return 0;
}
