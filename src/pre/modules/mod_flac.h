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

#ifndef mod_flac_h
#define mod_flac_h

#define PROPERTY_MOD_FLAC_OUTPUT "mod_flac.output"

// default output mode
#define DEFAULT_OUTPUT "PRE-FLACNFO: \"%R\" \"a\" \"l\" \"%y\" \"%g\" \"\" \"%r\" \"%Q\" \"%o\" \"\" \"\" \"\" \"\" \"%v\""

#define NAME_MAX 255

typedef struct flac_info {
	char vendor_string[NAME_MAX];
	char artist[NAME_MAX];
	char album[NAME_MAX];
	char genre[NAME_MAX];
	char year[5];
    char bitrate[8];
    char samplingrate[10];
    char channelmode[2];
} flac_info_t;


#endif
