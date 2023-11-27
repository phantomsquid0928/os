#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// int main() {
// 	printf(1, "size : %d }|\n", getvp());
// 	int i = ssualloc2(2);
// 	printf(1, "size : %d }|\n", getvp());
// 	char *addr = (char*)i;
// 	addr[9216] = '1';
// 	printf(1, "%d", addr[9216]);
// 	// printf(1, "%d", i);
// 	// char *addr = (char *)i;
// 	// printf(1, "%p", addr);
// 	// printf(1, "%d", addr);
// 	// addr[0] = '1';
// 	exit();
// }

int main(void)
{
	int ret;
	printf(1, "Start: memory usages: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
	ret = ssualloc(-1234);

	if(ret < 0) 
		printf(1, "ssualloc() usage: argument wrong...\n");
	else
		exit();
  	
	ret = ssualloc(1234);

	if(ret < 0)
		printf(1, "ssualloc() usage: argument wrong...\n");
	else
		exit();

	ret = ssualloc(4096);

	if(ret < 0 )
		printf(1, "ssualloc(): failed...\n");
	else {
		printf(1, "After allocate one virtual page: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
		char *addr = (char *) ret;

		addr[0] = 'I';
		printf(1, "After access one virtual page: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
	}

	ret = ssualloc(12288);

	if(ret < 0 )
		printf(1, "ssualloc(): failed...\n");
	else {
		printf(1, "After allocate three virtual pages: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
		char *addr = (char *) ret;

		addr[0] = 'a';
		printf(1, "After access of first virtual page: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
		addr[10000] = 'b';
		printf(1, "After access of third virtual page: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
		addr[8000] = 'c';
		printf(1, "After access of second virtual page: virtual pages: %d, physical pages: %d\n", getvp(), getpp());
	}

	exit();
}