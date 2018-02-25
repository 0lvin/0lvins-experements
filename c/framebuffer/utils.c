/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
