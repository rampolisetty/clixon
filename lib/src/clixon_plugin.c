/*
 *
  ***** BEGIN LICENSE BLOCK *****
 
  Copyright (C) 2009-2018 Olof Hagsand and Benny Holmgren

  This file is part of CLIXON.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Alternatively, the contents of this file may be used under the terms of
  the GNU General Public License Version 3 or later (the "GPL"),
  in which case the provisions of the GPL are applicable instead
  of those above. If you wish to allow use of your version of this file only
  under the terms of the GPL, and not to allow others to
  use your version of this file under the terms of Apache License version 2, 
  indicate your decision by deleting the provisions above and replace them with
  the  notice and other provisions required by the GPL. If you do not delete
  the provisions above, a recipient may use your version of this file under
  the terms of any one of the Apache License version 2 or the GPL.

  ***** END LICENSE BLOCK *****
 */

#ifdef HAVE_CONFIG_H
#include "clixon_config.h" /* generated by config & autoconf */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "clixon_err.h"
#include "clixon_queue.h"
#include "clixon_hash.h"
#include "clixon_log.h"
#include "clixon_handle.h"
#include "clixon_plugin.h"


static find_plugin_t *
clicon_find_plugin(clicon_handle h)
{
    void *p;
    find_plugin_t *fp = NULL;
    clicon_hash_t *data = clicon_data(h);
    
    if ((p = hash_value(data, "CLICON_FIND_PLUGIN", NULL)) != NULL)
	memcpy(&fp, p, sizeof(fp));

    return fp;
}


/*! Return a function pointer based on name of plugin and function.
 * If plugin is specified, ask daemon registered function to return 
 * the dlsym handle of the plugin.
 */
void *
clicon_find_func(clicon_handle h, char *plugin, char *func)
{
    find_plugin_t *plgget;
    void          *dlhandle = NULL;

    if (plugin) {
	/* find clicon_plugin_get() in global namespace */
	if ((plgget = clicon_find_plugin(h)) == NULL) {
	    clicon_err(OE_UNIX, errno, "Specified plugin not supported");
	    return NULL;
	}
	dlhandle = plgget(h, plugin);
    }
    
    return dlsym(dlhandle, func);
}

/*! Load a dynamic plugin object and call its init-function
 * Note 'file' may be destructively modified
 * @param[in]  h       Clicon handle
 * @param[in]  file    Which plugin to load
 * @param[in]  dlflags See man(3) dlopen
 */
plghndl_t 
plugin_load(clicon_handle h, 
	    char         *file, 
	    int           dlflags)
{
    char      *error;
    void      *handle = NULL;
    plginit_t *initfn;

    clicon_debug(1, "%s", __FUNCTION__);
    dlerror();    /* Clear any existing error */
    if ((handle = dlopen (file, dlflags)) == NULL) {
        error = (char*)dlerror();
	clicon_err(OE_PLUGIN, errno, "dlopen: %s\n", error ? error : "Unknown error");
	goto done;
    }
    /* call plugin_init() if defined */
    if ((initfn = dlsym(handle, PLUGIN_INIT)) == NULL){
	clicon_err(OE_PLUGIN, errno, "Failed to find plugin_init when loading restconf plugin %s", file);
	goto err;
    }
    if ((error = (char*)dlerror()) != NULL) {
	clicon_err(OE_UNIX, 0, "dlsym: %s: %s", file, error);
	goto done;
    }
    if (initfn(h) != 0) {
	clicon_err(OE_PLUGIN, errno, "Failed to initiate %s", strrchr(file,'/')?strchr(file, '/'):file);
	if (!clicon_errno) 	/* sanity: log if clicon_err() is not called ! */
	    clicon_err(OE_DB, 0, "Unknown error: %s: plugin_init does not make clicon_err call on error",
		       file);
	goto err;
    }
 done:
    return handle;
 err:
    if (handle)
	dlclose(handle);
    return NULL;
}


/*! Unload a plugin
 * @param[in]  h       Clicon handle
 * @param[in]  handle   Clicon handle
 */
int
plugin_unload(clicon_handle h, 
	      plghndl_t    *handle)
{
    int        retval = 0;
    char      *error;
    plgexit_t *exitfn;

    /* Call exit function is it exists */
    exitfn = dlsym(handle, PLUGIN_EXIT);
    if (dlerror() == NULL)
	exitfn(h);

    dlerror();    /* Clear any existing error */
    if (dlclose(handle) != 0) {
	error = (char*)dlerror();
	clicon_err(OE_PLUGIN, errno, "dlclose: %s\n", error ? error : "Unknown error");
	/* Just report */
    }
    return retval;
}
