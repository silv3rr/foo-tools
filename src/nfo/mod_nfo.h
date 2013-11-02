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

#ifndef _mod_nfo_h
#define _mod_nfo_h

#include <collection/hashtable.h>

/*
 * Properties that the nfo module uses.
 */
#define PROPERTY_NFO_UNWANTEDDIR "nfo_unwanteddir"
#define PROPERTY_NFO_PREPENDFILE "nfo_prepend"
#define PROPERTY_NFO_APPENDFILE "nfo_append"
#define PROPERTY_NFO_CLEANLISTFILE "nfo_cleanlist"


int nfo_check(hashtable_t *conf, char *file, char *dir, long crc);

int nfo_dupe(hashtable_t *conf, char *file, char *dir);


#endif
