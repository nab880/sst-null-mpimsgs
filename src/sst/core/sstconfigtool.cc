#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "sst_config.h"

void print_usage() {
	printf("======================================================\n");
	printf("SST %s Core Installation Configuration\n", PACKAGE_VERSION);
	printf("======================================================\n");
	printf("\n");
	printf("SST Install Prefix: %s\n", SST_INSTALL_PREFIX);
	exit(1);
}

int main(int argc, char* argv[]) {
	bool found_help = false;

	for(int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--help") == 0 ||
			strcmp(argv[i], "-help") == 0) {
			found_help = true;
			break;
		}
	}

	if(found_help) {
		print_usage();
	}

	if(argc > 2) {
		print_usage();
	}

	if( 0 == strcmp(argv[1], "--prefix") ) {
		printf("%s\n", SST_INSTALL_PREFIX);
	} else if( 0 == strcmp(argv[1], "--version") ) {
		printf("%s\n", PACKAGE_VERSION);
	} else {
		if( 0 == strncmp(argv[1], "--", 2) ) {
			char* param = argv[1];
			param++;
			param++;

			printf("Find: %s\n", param);
		} else {
			fprintf(stderr, "Unknown parameter to find (%s), must start with --\n", argv[1]);
			exit(-1);
		}
	}

	return 0;
}
