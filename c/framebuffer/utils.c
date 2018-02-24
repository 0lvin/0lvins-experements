#include "utils.h"

void mkdir_if_not_exists(const char *target, mode_t mode) {
	int res = 0;
	struct stat sb;

	if (stat(target, &sb) != 0)
	{
		res =  mkdir(target, mode);
		if (res < 0) {
			printf("can't create '%s' dir for mount\n", target);
		} else {
			printf("New dir for %s mount\n", target);
		}
	} else if (!S_ISDIR(sb.st_mode)) {
		printf("Exist but not directory %s.\n", target);
	}
}
