
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
 * Module to extract mp3info from a pred release
 * Author, Soren.
 * $Id: mod_idmp3.c,v 1.5 2003/06/23 14:32:18 sorend Exp $
 */

#include <string.h>
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

        // moduel dir func
        0,

        // module file func
        mod_flac_file_func,

        // module rel func
        0,

        // struct module_list entry
        0
};


// module global, checks number of mp3s checked (only handle first).
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


// format output using mp3info.
void mod_flac_format_output(char *format_string, FLAC__StreamMetadata *flac_info, char *year, char *genre, int kbps, char *relname) {
        char tmp[255]="";
        char mod[1000],*percent,*pos,*code;
        char *format=format_string;
        int modlen;

        strcpy(mod, format_string);

//      mod_flac_replace(mod, "%a", mp3->id3.artist);
//      mod_flac_replace(mod, "%l", mp3->id3.album);
        mod_flac_replace(mod, "%y", year);
        mod_flac_replace(mod, "%g", genre);

        sprintf(tmp, "%u", flac_info->data.stream_info.sample_rate);
        mod_flac_replace(mod, "%Q", tmp);

//      mod_flac_replace(mod, "%L", mod_idmp3_layer_text[header_layer(&mp3->header)-1]);

        sprintf(tmp, "%u", flac_info->data.stream_info.channels);
        mod_flac_replace(mod, "%o", tmp);

//      sprintf(tmp, "%u", flac_info->data.stream_info.bits_per_sample);
        sprintf(tmp, "%d", kbps);
        mod_flac_replace(mod, "%r", tmp);

        mod_flac_replace(mod, "%R", relname);

//      mod_flac_replace(mod, "%S", "NYI");
//      mod_flac_replace(mod, "%m", "NYI");
//      mod_flac_replace(mod, "%s", "NYI");

        gl_gllog_add(mod);

//      printf("  .. flac module says: Logged information to glftpd.log\n   .. %s\n", mod);

        printf("%s\n", mod);
}


// file func.
int mod_flac_file_func(char *filepath, char *argv[]) {

        char *tmp;
//      FILE *fh;
        char genre[255];
        char year[255];
        int seconds;
        struct stat st;

        if (mod_flac_count > 0) {
                // printf("already got the flac, break\n");
                return 0;
        }

        tmp = strrchr(filepath, '.');

        if (tmp)
                tmp++;
        else
                tmp = filepath;

        if (strcasecmp(tmp, "flac")) {
                // printf(" .. %s -> not flac, continue\n", tmp);
                return 1;
        }

//      fh = fopen(filepath, "r");

        // could not open file
//      if (!fh)
//              return 1;

        FLAC__StreamMetadata *temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);

        if (FLAC__metadata_get_tags(filepath, &temp_meta)) {
                FLAC__uint32 i;

                for (i = 0; i < temp_meta->data.vorbis_comment.num_comments; i++) {
                        char t_field[255];
                        char t_args[255];
                        char t[255];
                        int idx = 0;
                        FLAC__uint32 j;


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

//                      for (j = 0; j < strlen(t_field); j++) {
//                              t_field[j] = toupper(t_field[j]);
//                      }

                        if (strcasecmp("DATE", t_field) == 0) {
                                if (t_field != NULL) {
                                        strcpy(year, t_args);
                                } else {
                                        strcpy(year, "EUnknown");
                                }
                        } else if (strcasecmp("GENRE", t_field) == 0) {
                                if (t_field != NULL) {
                                        strcpy(genre, t_args);
                                } else {
                                        strcpy(genre, "EUnknown");
                                }
                        }

                        t_field[0] = '\0';
                        t_args[0] = '\0';
                }
        }
        FLAC__metadata_object_delete(temp_meta);

        temp_meta = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
        if (false == FLAC__metadata_get_streaminfo(filepath, temp_meta)) {
                return 1;
        }
        FLAC__metadata_object_delete(temp_meta);

        seconds = (temp_meta->data.stream_info.total_samples / temp_meta->data.stream_info.sample_rate);
        stat(filepath, &st);

        mod_flac_count++;

        tmp = ht_get(get_config(), PROPERTY_MOD_FLAC_OUTPUT);

        if (!tmp)
                tmp = strdup(DEFAULT_OUTPUT);
        else
                tmp = strdup(tmp);

        mod_flac_format_output(tmp, temp_meta, year, genre, (((st.st_size/seconds) * 8) / 1000), argv[1]);

        return 1;
}