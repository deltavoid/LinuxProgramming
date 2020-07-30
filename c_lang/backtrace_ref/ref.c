#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
 
void fun1();
void fun2();
void fun3();
void fun4();
void fun5();
void fun6();
void print_callstack();
 
int main()
{
	fun6();
	return 0;
}
 
void fun1()
{
	print_callstack();
}
 
void fun2()
{
	fun1();
}
 
void fun3()
{
	fun2();
}
 
void fun4()
{
	fun3();
}
 
void fun5()
{
	fun4();
}
 
void fun6()
{
	fun5();
}
 
void print_callstack()
{
	int size = 32;
	int i;
	void *array[32];
	int stack_num = backtrace(array, size);
	char **stacktrace = NULL;
 
	printf("%s begin\n", __func__);
	stacktrace = (char**)backtrace_symbols(array, stack_num);
 
	for (i = 0; i < stack_num; i++)
	{
		printf("%s\n", stacktrace[i]);
	}
	free(stacktrace);
	printf("%s end\n", __func__);

}