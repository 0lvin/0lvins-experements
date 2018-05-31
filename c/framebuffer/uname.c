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
#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
	struct utsname utsname_buffer = {0};
	char buffer[1024];

	snprintf(buffer, sizeof(buffer)-1, "%p\n", buffer);
	write(1, buffer, strlen(buffer));
	if (uname(&utsname_buffer) < 0) {
		snprintf(buffer, sizeof(buffer)-1 , "Can't run uname!");
	} else {
		snprintf(buffer, sizeof(buffer)-1, "%s %s %s %s %s \n",
					    utsname_buffer.sysname,
					    utsname_buffer.nodename,
					    utsname_buffer.release,
					    utsname_buffer.version,
					    utsname_buffer.machine);
	}
	write(1, buffer, strlen(buffer));
	return 0;
}
