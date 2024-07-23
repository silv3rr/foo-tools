/*
 * foo-tools, a collection of utilities for glftpd users.
 * Copyright (C) 2003  Tanesha FTPD Project, www.tanesha.net
 *
 * This file is part of foo-tools.
 *
 * foo-tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * foo-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with foo-tools; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../foo-pre.h"
#include <collection/hashtable.h>
#include <lib/gllogs.h>

// test a module.
int test_module(hashtable_t cfg, char *module, char *mode, char *file, char *path) {

	void *handle;
	module_list_t *module_func;
	module_list_t* (*module_loader)();

	//char *args[] = {"", "Whatever-REL", "mp3"};
	char *args[] = {"", "Whatever-REL", "flac"};

	handle = dlopen(module, RTLD_LAZY);

	int rc;

	void (*set_config)(hashtable_t *ht);

	if (!handle) {
		printf("Error loading module '%s':\n%s\n", module, dlerror());
		return 0;
	}

	module_loader = dlsym(handle, MODULE_LOADER_FUNC);
	set_config = dlsym(handle, MODULE_SETCONFIG_FUNC);

	if (!module_loader) {
		printf("Error loading module %s: No loader func\n");
		dlclose(handle);
		return 0;
	}

	if (!mode)
		strcpy(mode, "1");

	set_config(&cfg);

	// show module name
	printf("INFO: [tester] mod_name is \"%s\"\n", module_loader()->mod_name);

	switch(mode[0]) {
		case '1':
		// test module file func
		if (module_func->mod_func_file != 0)
			module_loader()->mod_func_file(file, args);
		break;

		case '2':
		// test module dir func
		if (module_func->mod_func_dir != 0)
			module_loader()->mod_func_dir(path, args);
		break;

		case '3':
		// test module rel func
		if (module_func->mod_func_rel != 0)
			module_loader()->mod_func_rel(path, args);
		break;

		default:
			printf("ERROR: specify func num, exiting...\n");
			exit(1);
	}

	dlclose(handle);

	return 1;
}

hashtable_t *cfg = 0;
hashtable_t *get_config() {
	return cfg;
}

int do_module(hashtable_t cfg, char *module, char *mode, char *file, char *path) {
	void *handle;
	module_list_t *module_func;
	module_list_t* (*module_loader)();
	//char *args[] = {"", "Whatever-REL", "mp3"};
	char *args[] = {"", "Whatever-REL", "flac"};

	void (*set_config)(hashtable_t *ht);

	//char *module;

	char *err;

	//module = "mod_flac.so";

	handle = dlopen(module, RTLD_LAZY);
	handle = dlopen(module, RTLD_LAZY);
	if (!handle) {
		err = dlerror();
		printf("ERROR: loading module %s: %s\n", module, err);
		return 0;
	}

	module_loader = dlsym(handle, MODULE_LOADER_FUNC);
	set_config = dlsym(handle, MODULE_SETCONFIG_FUNC);

	if (!module_loader || !set_config) {
		printf("ERROR: loading module %s: No loader func found\n");
		dlclose(handle);
		return 0;
	}

	set_config(&cfg);

	module_func = module_loader();

	if (module_func->mod_func_file != 0)
		module_func->mod_func_file(file, args);

	if (module_func->mod_func_dir != 0)
		module_func->mod_func_dir(path, args);

	if (module_func->mod_func_rel != 0)
		module_func->mod_func_rel(path, args);

	dlclose(handle);
}

int main(int argc, char *argv[]) {

	if (!argv[2]) {
		printf("Error no module specified\n");
		exit(1);
	}

	cfg = malloc(sizeof(hashtable_t));
	ht_init(cfg);
	ht_load(cfg, argv[1]);

	get_config();
	//gl_gllog_add("");

	//mode 1:file 2:dir 3:rel
	if (argv[3][0] == '1' || argv[3][0] == '2' || argv[3][0] == '3')
		test_module(*cfg, argv[2], argv[3], argv[4], argv[5]);

	// mode 4
	if (argv[3][0] == '4')
		do_module(*cfg, argv[2], argv[3], argv[4], argv[5]);

}
/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
