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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    mow, av
// Date:         2009-06-24
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include "flash_config.h"
#include "ezxml.h"
#include "xconfig.h"

#undef _DO_TRACE
#undef DEBUG
#ifdef DEBUG
#define printf_d(args...) printf(args)
#else
#define	printf_d(args...) /* nothing */
#endif

ezxml_t xml_cfgroot=NULL;		// root-node of in-RAM representation of fconfig-data
ezxml_t xml_fisroot=NULL;		// root-node of in-RAM representation of fis-data
ezxml_t xml_confroot=NULL;		// root-node of in-RAM XML-data
char *xml_confroot_str=NULL;
char *target_device=NULL;		// device for write-back
//int c=0;
options_t options={0, 0, NULL, NULL, NULL, NULL, 0, 0};

int usage()
{
	printf("Usage: $0 [command] [options] \n"
"\n"
"Script to manipulate the XML-configuration data of GuF i.MX Boards\n"
"inside the NAND Flash. \n"
"\n"
"xconfig expect one of the following commands to define its behavior: \n"
"  list           show the current contents of the XML-configuration as\n"
"                 obtained from NAND\n"
"  import         import an external XML-configuration file into the NAND\n"
"                 Flash\n"
"  addnode        add a new (empty) XML node\n"
"  delnode        delete XML node(s) and its/their children\n"
"  addattribute   add a new or change an existing attribute\n"
"  delattribute   delete an existing attribute\n"
"If no command and option given xconfig will display its version and exit.\n"
"\n"
"Syntax:\n"
"  xconfig list [-v] [-f] [-p <path>] \n"
"    -v           show whole (sub)tree instead of summary, the format is\n"
"	             suitable to be used as input to \"xconfig import\"\n"
"	-f           by default xconfig operates on the <variables>-subtree\n"
"	             only, this option switches to the <flash>-subtree\n"
"				 defining the NAND partitions\n"
"				 *strongly discouraged* - Use the \"fis\" commands instead\n"
"	-p <path>    path to subtree, e.g.:\n"
"	    -p /configurationFile/variables/setting[@key='enable_serialdiag]\n"
"		         if omitted, the whole tree is shown\n"
"  xconfig import [-v] [-f] [-y] [-b <addr>]\n"
"    -v           be verbose\n"
"	-f           same as above\n"
"	-y           assume yes - apply changes without user interaction\n"
"	-b <addr>    path to file to import\n"
"  xconfig addnode [-f] [-y] -p <path> -n <name>\n"
"	-f           same as above\n"
"	-y           same as above\n"
"	-p <path>    path to parent node\n"
"	-n <name>    name of new node\n"
"  xconfig delnode [-f] [-y] -p <path>\n"
"	-f           same as above\n"
"	-y           same as above\n"
"	-p <path>    path to node\n"
"  xconfig addattribute [-f] [-y] -p <path> -n <attr-name> -v <value>\n"
"	-f           same as above\n"
"	-y           same as above\n"
"	-p <path>    path to parent node\n"
"	-n <attr-name>   name of the attribute\n"
"	-v <value>   new value of attribute\n"
"  xconfig delattribute [-f] [-y] -p <path> -n <attr-name>\n"
"	-f           same as above\n"
"	-y           same as above\n"
"	-p <path>    path to parent node\n"
"	-n <attr-name>   name of the attribute\n");
	return 0;
}

int parseargs(int argc, char *argv[])
{
	int opt;
	if (argc == 1) {
		usage();
		return 0;
	}
	printf_d("Start parsing args\n");
	char *command = argv[1];
	if ( ! strcmp(command, "list")) {
		options.command = 1;
		while ((opt = getopt(argc, argv, "vfp:")) != -1) {
			switch (opt) {
			case 'v':
				options.verbose = 1;
				printf_d("verbose\n");
				break;
			case 'f':
				options.flash = 1;
				printf_d("flash\n");
				break;
			case 'p':
				options.path = strdup(optarg);
				if ( ! strncmp(optarg,"/configurationFile", 18) ) {
					options.path+=18;
					printf_d("trunk'd path\n");
				}
				printf_d("verbose\n");
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	}else if ( ! strcmp(command, "addnode")) {
		options.command = 2;
		while ((opt = getopt(argc, argv, "vyp:n:")) != -1) {
			switch (opt) {
			case 'v':
				options.verbose = 1;
				printf_d("verbose\n");
				break;
			case 'y':
				options.assumeyes = 1;
				printf_d("assume_yes\n");
				break;
			case 'p':
				options.path = strdup(optarg);
				if ( ! strncmp(optarg,"/configurationFile", 18) ) {
					options.path+=18;
					printf_d("trunk'd path\n");
				}
				printf_d("path=%s\n", options.path);
				break;
			case 'n':
				options.name = strdup(optarg);
				printf_d("name=%s\n", options.name);
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	} else if ( ! strcmp(command, "delnode")) {
		options.command = 3;
		while ((opt = getopt(argc, argv, "vyp:")) != -1) {
			switch (opt) {
			case 'v':
				options.verbose = 1;
				printf_d("verbose\n");
				break;
			case 'y':
				options.assumeyes = 1;
				printf_d("assume_yes\n");
				break;
			case 'p':
				options.path = strdup(optarg);
				if ( ! strncmp(optarg,"/configurationFile", 18) ) {
					options.path+=18;
					printf_d("trunk'd path\n");
				}
				printf_d("path=%s\n", options.path);
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	} else if ( ! strcmp(command, "addattribute")) {
		options.command = 4;
		while ((opt = getopt(argc, argv, "fyp:n:v:")) != -1) {
			switch (opt) {
			case 'f':
				options.flash = 1;
				printf_d("flash\n");
				break;
			case 'y':
				options.assumeyes = 1;
				printf_d("assume_yes\n");
				break;
			case 'p':
				options.path = strdup(optarg);
				if ( ! strncmp(optarg,"/configurationFile", 18) ) {
					options.path+=18;
					printf_d("trunk'd path\n");
				}
				printf_d("path=%s\n", options.path);
				break;
			case 'n':
				options.name = strdup(optarg);
				printf_d("name=%s\n", options.name);
				break;
			case 'v':
				options.value = strdup(optarg);
				printf_d("value=%s\n", options.value);
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	} else if ( ! strcmp(command, "delattribute")) {
		options.command = 5;
		while ((opt = getopt(argc, argv, "fyp:n:")) != -1) {
			switch (opt) {
			case 'f':
				options.flash = 1;
				printf_d("flash\n");
				break;
			case 'y':
				options.assumeyes = 1;
				printf_d("assume_yes\n");
				break;
			case 'p':
				options.path = strdup(optarg);
				if ( ! strncmp(optarg,"/configurationFile", 18) ) {
					options.path+=18;
					printf_d("trunk'd path\n");
				}
				printf_d("path=%s\n", options.path);
				break;
			case 'n':
				options.name = strdup(optarg);
				printf_d("name=%s\n", options.name);
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	} else if ( ! strcmp(command, "import")) {
		options.command = 6;
		while ((opt = getopt(argc, argv, "vfyb:")) != -1) {
			switch (opt) {
			case 'v':
				options.verbose = 1;
				printf_d("verbose\n");
				break;
			case 'f':
				options.flash = 1;
				printf_d("flash\n");
				break;
			case 'y':
				options.assumeyes = 1;
				printf_d("assume_yes\n");
				break;
			case 'b':
				options.base = strdup(optarg);
				printf_d("base=%s\n", options.base);
				break;
			default: /* '?' */
				printf("Unknown option...\n");
				usage();
				return 1;
			}
		}
	} else {
		printf("Unknown command: %s\n", command);
		return 1;
	}
	printf_d("Found command: %i\n", options.command);
	return 0;
}

/******************************************
 *** Internal helper functions
 ******************************************/
/*****************************************************************
 *Function name	: find_node
*/
/**
 * @brief	Checks if a given XML-node is already on the node-stack
 *
 * @param	xml			XML-node to search for
 * @param	head		head of node-stack
 *
 * @return	1 if element is already on node-stack,  otherwise
 */
static int find_node(ezxml_t xml, node_stack_t *head)
{
	for (; head; head = head->next)
		if (head->node == xml)
			return 1;

	return 0;
}

/*****************************************************************
 *Function name	: push_node
*/
/**
 * @brief	Push new element on node-stack
 *
 * @param	xml			XML-node to push on node-stack
 * @param	phead		head of node-stack
 *
 * @return	1 if element was pushed successfully,  otherwise
 */
static int push_node(ezxml_t xml, node_stack_t **phead)
{
	if (!phead)
		return 0;

	// only add node, if it is not already present
	if (!find_node(xml, *phead))
	{
		node_stack_t *new_node = (node_stack_t *)malloc(sizeof(node_stack_t));
		if (!new_node)
		{
			printf("ERROR: xconfig out-of-memory\n");
			return 0;
		}
		new_node->node = xml;
		new_node->next = *phead;
		*phead = new_node;
	}
	return 1;
}

/*****************************************************************
 *Function name	: pop_node
*/
/**
 * @brief	Pop top element from temporary node-stack
 *
 * @param	phead		head of node-stack
 *
 * @return	Top element from node-stack, if stack was not empty. NULL otherwise
 */
static node_stack_t *pop_node(node_stack_t **phead)
{
	node_stack_t *cur = NULL;

	if (!phead)
		return NULL;

	cur = *phead;
	if (cur)
		*phead = cur->next;

	return cur;
}

/*****************************************************************
 *Function name	: show_xml_node_pre
*/
/**
 * @brief	Prints the opening tag of an XML-node and its attributes
 *
 * @param	node		XML-node to be printed
 * @param	verbose		if 1, show all attributes
 *						if , only show identifying attributes
 *
 * @return	Nothing
 */
static void show_xml_node_pre(ezxml_t node, int verbose)
{
	int i;
	char **attrs = NULL;
	int bprint_ellipsis = 0;
	int level = ezxml_depth(node);

	if (!node)
		return;

	// print yourself
	for (i = 0; i < level; i++)
		printf("    ");

	printf("<%s", ezxml_name(node));

	// print attributes
	for (attrs = ezxml_attr_next(node, NULL);
		 attrs && attrs[0] && attrs[1];
		 attrs = ezxml_attr_next(node, attrs))
	{
		if (verbose)
		{
			// in verbose-mode we show all attributes,
			// each one on a new line.
			printf("\n");
			for (i = 0; i < (level+1); i++)
				printf("    ");

			// printf() has problems with long strings...
			if (strlen(attrs[1]) < 128)
				printf("%s=\"%s\"", attrs[0], attrs[1]);
			else
			{
				char *ptr = attrs[1];
				printf("%s=\"", attrs[0]);
				for (i = strlen(attrs[1]); i; i--)
					printf("%c", *ptr++);
				printf("\"");
			}
		}
		else
		{
			// in non-verbose mode we only show identifying attributes
			if (!strcmp(attrs[0], "name") ||
				!strcmp(attrs[0], "key"))
			{
				printf(" %s=\"%s\"", attrs[0], attrs[1]);
			}
			else
			{
				// if tag has other attributes, show '...'
				bprint_ellipsis = 1;
			}
		}
	}
	printf("%s>\n", bprint_ellipsis ? " ... " : "");
}

/*****************************************************************
 *Function name	: show_xml_node_post
*/
/**
 * @brief	Prints the closing tag of an XML-node
 *
 * @param	node		XML-node to be printed
 * @param	verbose		if 1, show what's happening
 *
 * @return	Nothing
 */
static void show_xml_node_post(ezxml_t node, int verbose)
{
	int i;
	int level = ezxml_depth(node);

	if (!node)
		return;

	// print closing tag
	for (i = 0; i < level; i++)
		printf("    ");
	printf("</%s>\n", ezxml_name(node));
}

/*****************************************************************
 *Function name	: select_xmlroot
*/
/**
 * @brief	Selects XML configuration tree to be used
 *
 * @param	use_fisroot		if 1, returns FIS-tree, otherwise returns fconfig-tree
 * @param	warn_fis		if 1, print warning about dangers of using the FIS-tree
 *
 * @return	Handle to selected XML configuration tree
 */
static ezxml_t select_xmlroot(int use_fisroot, int warn_fis)
{
	if (use_fisroot)
	{
		if (warn_fis)
			printf("Warning: direct manipulation of fis-data may render the device unusable!!!\n");
		return xml_fisroot;
	}

	return xml_cfgroot;
}

/*****************************************************************
 *Function name	: save_to_flash
*/
/**
 * @brief	Saves XML configuration tree to flash
 *
 * @param	use_fisroot		if 1, save FIS-tree, otherwise saves fconfig-tree
 * @param	assume_yes		if 1, don't query for confirmation of writing to flash
 *
 * @return	Nothing
 */
int save_to_flash(int use_fisroot, int assume_yes)
{
	char *xml_str;
	int ret=0;
	ezxml_t xml_flash = NULL;

	// update generation information
	xml_flash = ezxml_child(xml_confroot, "flash");
	if (!xml_flash)	{
		ret=1;
	} else {
		char temp_buf[11];
		int img_generation = strtoul(ezxml_attr(xml_flash, "generation"), NULL, 0);
		printf_d("img_generation=%u\n", img_generation);
		sprintf(temp_buf, "%u", img_generation+1);
		ezxml_set_attr_d(xml_flash, "generation", temp_buf);
	}

	xml_str=ezxml_toxml(xml_confroot);

	ret=xml_write_to_mtd(&xml_str, assume_yes, &target_device);
	if (!ret)
		printf("Configuration successfully written.\n");
	return ret;
}

/*****************************************************************
 *Function name	: xml_copy_node
*/
/**
 * @brief	Copies a node from import-file into the configuration tree
 *
 * @param	xml_root	handle node under which the import-node should be added
 * @param	xml_dest	if not NULL, node in configuration tree that should
 *						be overwritten. If NULL, a new node is allocated under
 *						xml_root.
 * @param	xml_src		source node in import-file that shall be copied
 *						into the configuration tree.
 * @param	verbose		if 1, show what's happening
 * @param	key			name of attribute that can be used to identify nodes
 *
 * @return	Nothing
 */
static void xml_copy_node(ezxml_t xml_root, ezxml_t xml_dest,
						  ezxml_t xml_src, int verbose, const char *key)
{
	ezxml_t xml_child;
	char **attrs = NULL;

	if (!xml_root || !xml_src || !key)
		return;

	if (verbose)
		printf("%s \"%s\"-tag \"%s\"\n", xml_dest ? "Replacing" : "Adding",
					ezxml_name(xml_src), ezxml_attr(xml_src, key));

	// if now destination node is given, create a new one under xml_root
	if (!xml_dest)
	{
		xml_dest = ezxml_add_child_d(xml_root, ezxml_name(xml_src), 0);
		if (!xml_dest)
		{
			printf("Error: Couldn't add \"%s\"-tag \"%s\"\n",
						ezxml_name(xml_src), ezxml_attr(xml_src, key));
			return;
		}
	}

	// copy all attributes
	for (attrs = ezxml_attr_next(xml_src, NULL);
		 attrs && attrs[0];
		 attrs = ezxml_attr_next(xml_src, attrs))
	{
		if (!attrs[1])
			ezxml_set_attr(xml_dest, attrs[0], NULL);		// remove attribute
		else
			ezxml_set_attr_d(xml_dest, attrs[0], attrs[1]); // add attribute
	}

	// copy all children
	while((xml_child = ezxml_child(xml_src, NULL)))
	{
		// copy xml_child into xml_dest, replacing existing sub-nodes
		// with identical names.
		xml_copy_node(xml_dest, ezxml_child(xml_dest, ezxml_name(xml_child)), xml_child, verbose, "key");

		ezxml_remove(xml_child);
	}
}

/*****************************************************************
 *Function name	: xml_find_attribute
*/
/**
 * @brief	Helper function of xml_import. Searches of an XML
 *			node with a given attribute/value combination.
 *			(Can be used to find "setting" or "partition" nodes).
 *
 * @param	xml_root	handle node where to start searching
 * @param	name		name of attribute to search for
 * @param	value		value of attribute to search for
 *
 * @return	Handle to node containg name/value-attribute, if found.
 *			NULL otherwise.
 */
static ezxml_t xml_find_attribute(ezxml_t xml_root, const char *name, const char *value)
{
	if (!name || !value)
		return NULL;

	for (; xml_root; xml_root = xml_root->next)
	{
		const char *my_value = ezxml_attr(xml_root, name);
		if (my_value && !strcmp(my_value, value))
			break;
	}

	return xml_root;
}

/*****************************************************************
 *Function name	: xml_import_unsupported
*/
/**
 * @brief	Helper function of xml_import to handle unsupported import data
 *
 * @param	xml_root	handle to some node of configuration tree
 * @param	xml_import	handle to unsupported node of import file
 * @param	verbose		if 1, show what is happening
 *
 * @return	None
 */
static void xml_import_unsupported(ezxml_t xml_root, ezxml_t xml_import, int verbose)
{
	printf("Warning: Import of \"%s\"-tag not supported. Skipped.\n",
			    ezxml_name(xml_import));
}
/*****************************************************************
 *Function name	: xml_import_variables
*/
/**
 * @brief	Helper function of xml_import to handle importing data
 *			into a "variables"-node
 *
 * @param	xml_root	handle to "variables"-node of configuration tree
 * @param	xml_import	handle to "variables"-node of import file
 * @param	verbose		if 1, show what is happening
 *
 * @return	None
 */
static void xml_import_variables(ezxml_t xml_root, ezxml_t xml_import, int verbose)
{
	ezxml_t xml_child;

	if (!xml_import)	// nothing to do
		return;

	if (!xml_root)
	{
		xml_import_unsupported(xml_root, xml_import, verbose);
		return;
	}

	if (verbose)
		printf("Importing \"%s\"-tag\n", ezxml_name(xml_import));

	while((xml_child = ezxml_child(xml_import, NULL)))
	{
		if (!strcmp(ezxml_name(xml_child), "setting"))
		{
			// try to find a setting-tag with the same key
			ezxml_t xml_setting = xml_find_attribute(ezxml_child(xml_root, "setting"), "key", ezxml_attr(xml_child, "key"));

			// copy new setting into tree (either overwriting existing setting or allocating a new one)
			xml_copy_node(xml_root, xml_setting, xml_child, verbose, "key");
		}
		else if (!strcmp(ezxml_name(xml_child), "display"))
		{
			// try to find a display-tag and delete it
			ezxml_remove(ezxml_child(xml_root, "display"));

			// copy new display into tree (allocating a new display-node)
			xml_copy_node(xml_root, NULL, xml_child, verbose, "name");
		}
		else if (!strcmp(ezxml_name(xml_child), "logo-license"))
		{
			// try to find a logo-license-tag and delete it
			ezxml_remove(ezxml_child(xml_root, "logo-license"));

			// copy new logo-license into tree (allocating a new node)
			xml_copy_node(xml_root, NULL, xml_child, verbose, "license");
		}
		else
		{
			xml_import_unsupported(xml_root, xml_child, verbose);
		}

		ezxml_remove(xml_child);
	}
}

/*****************************************************************
 *Function name	: xml_import_configurationFile
*/
/**
 * @brief	Helper function of xml_import to handle importing data
 *			into a "configurationFile"-node
 *
 * @param	xml_root	(top-level) handle to configuration tree
 * @param	xml_import	(top-level) handle to import file
 * @param	verbose		if 1, show what is happening
 *
 * @return	None
 */
static void xml_import_configurationFile(ezxml_t xml_root, ezxml_t xml_import, int verbose)
{
	ezxml_t xml_child;

	if (!xml_root || !xml_import)	// we're done
		return;

	if (verbose)
		printf("Importing \"%s\"-tag\n", ezxml_name(xml_import));

	while((xml_child = ezxml_child(xml_import, NULL)))
	{
		if (!strcmp(ezxml_name(xml_child), "variables"))
			xml_import_variables(ezxml_child(xml_root, "variables"), xml_child, verbose);
		else
			xml_import_unsupported(xml_root, xml_child, verbose);

		ezxml_remove(xml_child);
	}
}

/*****************************************************************
 *Function name	: xml_list
*/
/**
 * @brief	List contents of XML configuration tree
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_list(int verbose, char *path, int use_fisroot)
{
    const char **pi = NULL;
    ezxml_t xml_root = xml_cfgroot;
    printf_d("\n==> xml_list(verbose= %i, path=%s, use_fisroot=%i)\n",
    		verbose, path, use_fisroot);
	xml_root = select_xmlroot(use_fisroot, 0);

	// in verbose mode we print processing instructions
	if (verbose)
		for (pi = ezxml_pi(xml_root, "xml"); *pi; pi++)
			printf("<?xml %s?>\n", *pi);

	// print XML-tree
	ezxml_select_xpath(xml_root, path, 0,
				       (ezxml_node_func_t)show_xml_node_pre, (void *)verbose,
				       (ezxml_node_func_t)show_xml_node_post, (void *)verbose);
}

/*****************************************************************
 *Function name	: xml_import
*/
/**
 * @brief	Import an XML configuration file from RAM into configuration tree
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_import(int verbose, char *filename, int use_fisroot, int assume_yes)
{
    ezxml_t xml_import = NULL;
    char *xml_import_str = NULL;
	FILE *input = NULL;
	int len;

	// Get file length
	input = fopen(filename, "r");
	if (! input) {
		printf("Failed to open %s\n", filename);
		return;
	}
	fseek(input, 0L, SEEK_END);
	len = ftell(input);
	rewind(input);

	xml_import_str = malloc(len);
	if ( ! xml_import_str ) {
		perror("Insufficient memory\n");
		fclose(input);
		return;
	}

	// check for <?xml tag
	fread(xml_import_str, 5, 1, input);
	if ( ! strstr(xml_import_str, "<?xml") ) {
		printf("%s: No valid XML structure\n", filename);
		return;
	}

	// read file content into memory
	rewind(input);
	fread(xml_import_str, len, 1, input);
	// check for valid string
	if ( strlen(xml_import_str) >= len ) {
		perror("Invalid data\n");
		fclose(input);
		return;
	}

	fclose(input);

	if (verbose)
		printf("Importing %u bytes of XML-data from 0x%08x\n", strlen(xml_import_str), (unsigned int)xml_import_str);

	xml_import = ezxml_parse_str(xml_import_str, strlen(xml_import_str));

	if (xml_import && !strcmp(ezxml_name(xml_import), "configurationFile"))
		xml_import_configurationFile(select_xmlroot(use_fisroot, 1), xml_import, verbose);
	else
		printf("Error: Unsupported configuration file format.\n");

	ezxml_free(xml_import);
	if (xml_import_str)
		free(xml_import_str);
	save_to_flash(use_fisroot, assume_yes);
}

/*****************************************************************
 *Function name	: xml_delnode
*/
/**
 * @brief	Delete an XML-node
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_delnode(int verbose, char *path, int use_fisroot, int assume_yes)
{
    node_stack_t *matching_nodes = NULL;
    node_stack_t *cur_node = NULL;

	// find all selected nodes
	ezxml_select_xpath(select_xmlroot(use_fisroot, 1), path, 1,
				       (ezxml_node_func_t)push_node, (void *)&matching_nodes,
				       (ezxml_node_func_t)NULL, (void *)NULL);

	// delete them
	while ((cur_node = pop_node(&matching_nodes)))
	{
		printf("delete node \"%s\":0x%08x\n", ezxml_name(cur_node->node), (unsigned int)cur_node->node);
		ezxml_remove(cur_node->node);
		free(cur_node);
	}

	save_to_flash(use_fisroot, assume_yes);
}

/*****************************************************************
 *Function name	: xml_addnode
*/
/**
 * @brief	Add new sub-node to an XML-node
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_addnode(int verbose, char *path, char *name, int use_fisroot, int assume_yes)
{
    node_stack_t *matching_nodes = NULL;
    node_stack_t *cur_node = NULL;

	// find all selected nodes
	ezxml_select_xpath(select_xmlroot(use_fisroot, 1), path, 1,
				       (ezxml_node_func_t)push_node, (void *)&matching_nodes,
				       (ezxml_node_func_t)NULL, (void *)NULL);

	// add a new node to each one
	while ((cur_node = pop_node(&matching_nodes)))
	{
		printf("add node \"%s\" to node \"%s\":0x%08x\n", name, ezxml_name(cur_node->node), (unsigned int)cur_node->node);
		ezxml_add_child_d(cur_node->node, name, 0);
		free(cur_node);
	}

	save_to_flash(use_fisroot, assume_yes);
}

/*****************************************************************
 *Function name	: xml_delattribute
*/
/**
 * @brief	Delete attribute of an XML-node
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_delattribute(char *path, char *name, int use_fisroot, int assume_yes)
{
    node_stack_t *matching_nodes = NULL;
    node_stack_t *cur_node = NULL;

	// find all selected nodes
	ezxml_select_xpath(select_xmlroot(use_fisroot, 1), path, 1,
				       (ezxml_node_func_t)push_node, (void *)&matching_nodes,
				       (ezxml_node_func_t)NULL, (void *)NULL);

	// delete attribute in each one
	while ((cur_node = pop_node(&matching_nodes)))
	{
		printf("delete attribute \"%s\" from node \"%s\":0x%08x\n", name, ezxml_name(cur_node->node), (unsigned int)cur_node->node);
		ezxml_set_attr(cur_node->node, name, NULL);
		free(cur_node);
	}

	save_to_flash(use_fisroot, assume_yes);
}

/*****************************************************************
 *Function name	: xml_addattribute
*/
/**
 * @brief	Add new or modify existing attribute of an XML-node
 *
 * @param	argc	number of command-line arguments
 * @param	argv	list of command-line arguments
 *
 * @return	None
 */
static void xml_addattribute(char *path, char *name, char *value, int use_fisroot, int assume_yes)
{
    node_stack_t *matching_nodes = NULL;
    node_stack_t *cur_node = NULL;

	// find all selected nodes
	ezxml_select_xpath(select_xmlroot(use_fisroot, 1), path, 1,
				       (ezxml_node_func_t)push_node, (void *)&matching_nodes,
				       (ezxml_node_func_t)NULL, (void *)NULL);

	// add attribute to each one
	while ((cur_node = pop_node(&matching_nodes)))
	{
		printf("add attribute \"%s\"=\"%s\" to node \"%s\":0x%08x\n", name, value, ezxml_name(cur_node->node), (unsigned int)cur_node->node);
		ezxml_set_attr_d(cur_node->node, name, value);
		free(cur_node);
	}

	save_to_flash(use_fisroot, assume_yes);
}

/**************************************************************
 * main
 */
int main(int argc, char *argv[])
{
	int ret=0;
	ret = parseargs(argc, argv);
	int len=0;
	if(ret)
		goto _exit;

	// read xml-data from NAND
	ret = xml_read_from_mtd(&xml_confroot_str, &target_device);
	printf_d("Got XML Data\n");
	if (ret ||  (! (xml_confroot_str && target_device)) ) {
		perror("Error obtaining XML-data\n");
		goto _exit;
	}
	len=strlen(xml_confroot_str);
	xml_confroot=ezxml_parse_str(xml_confroot_str, len);
	printf_d("XML-Error: %s\n", ezxml_error(xml_confroot));

	// split into xml_cfgroot (<variables>...</variables>) and
	//            xml_fisroot (<flash>...</flash>
	xml_cfgroot = ezxml_child(xml_confroot, "variables");
	if (!xml_cfgroot) {
		perror("No <variables> tag found, aborting\n");
		ret=1;
		goto _exit;
	}
	printf_d("Got <variables> subtree \n");
	xml_fisroot = ezxml_child(xml_confroot, "flash");
	if (!xml_fisroot) {
		perror("No <flash> tag found, aborting\n");
		ret=1;
		goto _exit;
	}
	printf_d("Got <flash> subtree \n");

	// eval options
	switch (options.command) {
	case 1: 	// list
		// set the default path if none given
		if (! options.path )
			options.path = ( options.flash ) ? "/flash" : "/variables" ;
		xml_list(options.verbose, options.path, options.flash);
		break;
	case 2: 	// addnode
		if (! options.path ) {
			perror("No path given! Use -p to specify the path to the node. \n");
			ret=1;
			goto _exit;
		}
		if (! options.name ) {
			perror("No name given! Use -n to specify the name of the node. \n");
			ret=1;
			goto _exit;
		}
		xml_addnode(options.verbose, options.path, options.name, options.flash, options.assumeyes);
		break;
	case 3: 	// delnode
		if (! options.path ) {
			perror("No path given! Use -p to specify the path to the node. \n");
			ret=1;
			goto _exit;
		}
		xml_delnode(options.verbose, options.path, options.flash, options.assumeyes);
		break;
	case 4: 	// addattribute
		if (! options.path ) {
			perror("No path given! Use -p to specify the path to the attribute. \n");
			ret=1;
			goto _exit;
		}
		if (! options.name ) {
			perror("No name given! Use -n to specify the name of the attribute. \n");
			ret=1;
			goto _exit;
		}
		if (! options.value ) {
			perror("No value given! Use -v to specify the value of the attribute. \n");
			ret=1;
			goto _exit;
		}
		xml_addattribute(options.path, options.name, options.value, options.flash, options.assumeyes);
		break;
	case 5: 	// delattribute
		if (! options.path ) {
			perror("No path given! Use -p to specify the path to the attribute. \n");
			ret=1;
			goto _exit;
		}
		if (! options.name ) {
			perror("No name given! Use -n to specify the name of the attribute. \n");
			ret=1;
			goto _exit;
		}
		xml_delattribute(options.path, options.name, options.flash, options.assumeyes);
		break;
	case 6: 	// import
		if (! options.base ) {
			perror("No filename given! Use -b to specify the path to the file to import. \n");
			ret=1;
			goto _exit;
		}
		xml_import(options.verbose, options.base, options.flash, options.assumeyes);
		break;
	default: 	// fail
		ret=1;
	}

	// clean up
_exit:
	if (xml_confroot)
		ezxml_free(xml_confroot);
	if (target_device)
		free(target_device);
	if (xml_confroot_str)
		free(xml_confroot_str);
	exit(ret);
}
