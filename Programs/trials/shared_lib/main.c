#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<dlfcn.h>

int main() {
	void *module;
	void (*function)();

	module = dlopen("./libtemp.so", RTLD_NOW | RTLD_LOCAL);
	if(!module) {
		printf("dlopen() error = %s", dlerror());
		return -1;
	}
	*(void **) (&function) = dlsym(module, "print");
	if(!module) {
		printf("dlopen() error = %s", dlerror());
		return -1;
	}
	(*function) ();
	dlclose(module);

	return 0;
}
