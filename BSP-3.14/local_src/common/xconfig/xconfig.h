/*
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define DEFAULT_PATH "/configurationFile/variables/"

typedef struct node_stack_t
{
	ezxml_t				node;
	struct node_stack_t	*next;
} node_stack_t;

typedef struct options_t {
	int command;
	/* values for command:
	 * 		0 	fail
	 * 		1	list
	 * 		2	addnode
	 * 		3	delnode
	 * 		4	addattribute
	 * 		5	delattribute
	 *		6	import
 	 */
	int flash;
	char *name;
	char *path;
	char *base;
	char *value;
	int verbose;
	int assumeyes;
} options_t;
