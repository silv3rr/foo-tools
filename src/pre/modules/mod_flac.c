
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
/*
 * Module to extract flac info from a pred release
 * Author, xxx, slv.
 * $Id: mod_flac.c,v x.x 2024/03/02 00:00:00 xxx, slv Exp $
 */

#include <string.h>
#include <stdio.h>

#ifdef DEBUG
#include <stdlib.h>
#include <time.h>
#endif

#include <sys/stat.h>
#include <FLAC/metadata.h>
#include <FLAC/format.h>

// project includes
#include "mod_flac.h"
#include "../foo-pre.h"

// footools includes
#include <collection/hashtable.h>

// in foo-pre
hashtable_t *_mod_flac_cfg = 0;

void set_config(hashtable_t *cfg) {
	_mod_flac_cfg = cfg;
}
hashtable_t *get_config() {
	return _mod_flac_cfg;
}

// prototype for file handling function.
int mod_flac_file_func(char *filepath, char *argv[]);

module_list_t mod_flac_info = {
	// module name
	"flac id extractor",

	// module dir func
	0,

	// module file func
	mod_flac_file_func,

	// module rel func
	0,

	// struct module_list entry
	0
};


// module global, checks number of flac files checked (only handle first).
int mod_flac_count = 0;

// function to return module info of this module.
module_list_t *module_loader() {
	return &mod_flac_info;
}

// replace function
int mod_flac_replace(char *b, char *n, char *r) {
	char *t, *save;
	int i=0;

	while (t=strstr(b, n)) {
		save=(char*)malloc(strlen(t)-strlen(n)+1);
		strcpy(save, t+strlen(n));
		*t=0;
		strcat(b, r);
		strcat(b, save);
		free(save);
		i++;
	}
}

void gl_gllog_add(char *mod);

// format output
void mod_flac_format_output(char *format_string, flac_info_t *flac_info, char *relname) {
	char mod[1000];
	char *format=format_string;

#ifdef DEBUG
	time_t now;
	struct tm *tm_now;
	now = time(0);
	tm_now = localtime(&now);
	printf("MODULE-DEBUG: flac_info->artist=%s flac_info->genre=%s\n", flac_info->artist, flac_info->genre);
	printf("MODULE-DEBUG: flac_info->samplingrate=%s\n", flac_info->samplingrate);
	FILE *f;
	f = fopen("mod_flac.log", "a");
	char fdate[12], ftime[10];
	strftime(fdate, 1024, "%Y-%m-%d", tm_now);
	strftime(ftime, 1024, "%H:%M:%S", tm_now);
	fprintf(f, "%s %s MODULE-DEBUG: flac_info->artist=%s flac_info->genre=%s\n", fdate, ftime, flac_info->artist, flac_info->genre);
	fprintf(f, "%s %s MODULE-DEBUG: flac_info->samplingrate=%s\n", fdate, ftime, flac_info->samplingrate);
	//fprintf(f, "%s %s MODULE-DEBUG: format_string=%s\n", fdate, ftime, format_string);
	fclose(f);
#endif

	strcpy(mod, format_string);

	mod_flac_replace(mod, "%a", flac_info->artist);
	mod_flac_replace(mod, "%l", flac_info->album);
	mod_flac_replace(mod, "%y", flac_info->year);
	mod_flac_replace(mod, "%g", flac_info->genre);
	mod_flac_replace(mod, "%Q", flac_info->samplingrate);
	mod_flac_replace(mod, "%o", flac_info->channelmode);
	mod_flac_replace(mod, "%r", flac_info->bitrate);
	mod_flac_replace(mod, "%R", relname);
	mod_flac_replace(mod, "%v", flac_info->vendor_string);

	gl_gllog_add(mod);
	//printf("  .. flac module says: Logged information to glftpd.log\n   .. %s\n", mod);
	printf(" %s\n", mod);
}


// file func.
int mod_flac_file_func(char *filepath, char *argv[]) {

	flac_info_t flac_info;
	char *tmp;
	FILE *fh;
	struct stat st;

	if (mod_flac_count > 0) {
		//printf(" .. already got .flac, break\n");
		return 0;
	}

	tmp = strrchr(filepath, '.');

	if (tmp)
		tmp++;
	else
		tmp = filepath;

	if (strcasecmp(tmp, "flac")) {
		//printf(" .. %s -> not .flac, continue\n", tmp);
		return 1;
	}

	fh = fopen(filepath, "rb");

	// could not open file
	if (!fh) {
#ifdef DEBUG
		printf(" .. could not open file (%s)\n", filepath);
#endif
		return 1;
	}

	fclose(fh);

	// defaults
	strcpy(flac_info.vendor_string, "Unknown");
	strcpy(flac_info.artist, "Unknown");
	strcpy(flac_info.album, "Unknown");
	strcpy(flac_info.genre, "Unknown");
	strcpy(flac_info.year, "0000");
	strcpy(flac_info.bitrate, "0");
	strcpy(flac_info.samplingrate, "0");
	strcpy(flac_info.channelmode, "0");

	FLAC__StreamMetadata *temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);

	if (FLAC__metadata_get_tags(filepath, &temp_meta)) {
		FLAC__uint32 i, j, k;
		k = 0;
		if (temp_meta->data.vorbis_comment.vendor_string.length > 0) {
			if (!strncmp((char *)temp_meta->data.vorbis_comment.vendor_string.entry, "reference libFLAC", 17))
				k = 10;
			for (i = 0; i < NAME_MAX - 1 && i < temp_meta->data.vorbis_comment.vendor_string.length; ++i)
				flac_info.vendor_string[i] = temp_meta->data.vorbis_comment.vendor_string.entry[i+k];
			flac_info.vendor_string[i] = '\0';
		}
		for (i = 0; i < temp_meta->data.vorbis_comment.num_comments; i++) {
			char t_field[255];
			char t_args[255];
			char t[255];
			int idx = 0;
			for (j = 0; j < temp_meta->data.vorbis_comment.comments[i].length; j++) {
				if (temp_meta->data.vorbis_comment.comments[i].entry[j] == '=') {
					t[idx] = '\0';
					strcpy(t_field, t);
					idx = 0;
					continue;
				}
				t[idx++] = temp_meta->data.vorbis_comment.comments[i].entry[j];
			}

			t[idx] = '\0';
			strcpy(t_args, t);

			if (strcasecmp("ARTIST", t_field) == 0) {
				if (t_field != NULL)
					strcpy(flac_info.artist, t_args);
			} else if (strcasecmp("ALBUM", t_field) == 0) {
				if (t_field != NULL)
					strcpy(flac_info.album, t_args);
			} else if (strcasecmp("DATE", t_field) == 0) {
				if (t_field != NULL)
					strcpy(flac_info.year, t_args);
			} else if (strcasecmp("GENRE", t_field) == 0) {
				if (t_field != NULL)
					strcpy(flac_info.genre, t_args);
			}

			t_field[0] = '\0';
			t_args[0] = '\0';
		}
	} else {
		return 1;
	}

	if(temp_meta != NULL)
		FLAC__metadata_object_delete(temp_meta);

	temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);

	if (FLAC__metadata_get_streaminfo(filepath, temp_meta)) {
		stat(filepath, &st);
		sprintf(flac_info.bitrate, "%ld", (st.st_size * temp_meta->data.stream_info.sample_rate) / (125 * temp_meta->data.stream_info.total_samples));
		sprintf(flac_info.samplingrate, "%u", temp_meta->data.stream_info.sample_rate);
		sprintf(flac_info.channelmode, "%u", temp_meta->data.stream_info.channels);
		//temp_meta->data.stream_info.bits_per_sample
	} else {
		return 1;
	}

	if(temp_meta != NULL)
		FLAC__metadata_object_delete(temp_meta);

	mod_flac_count++;

	tmp = ht_get(get_config(), PROPERTY_MOD_FLAC_OUTPUT);

	if (!tmp)
		tmp = strdup(DEFAULT_OUTPUT);
	else
		tmp = strdup(tmp);

	mod_flac_format_output(tmp, &flac_info, argv[1]);

	return 1;
}

/* vim: set noai tabstop=8 shiftwidth=8 softtabstop=8 noexpandtab: */
