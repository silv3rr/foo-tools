/*
 * Library to handle adding/updating some glftpd logfiles.
 *
 * $Id: gllogs.c,v 1.2 2001/07/20 09:25:43 sd Exp $
 * Maintained by: Flower
 */

#include "gllogs.h"
#include <stdio.h>
#include <time.h>
#include <sys/file.h>
#include <unistd.h>

int gl_dupefile_add(char *fn, char *u) {
	FILE *f;
	struct dupefile df;

	bzero(&df, sizeof(df));
	strcpy(df.filename, fn);
	strcpy(df.uploader, u);
	time(&df.timeup);
	
	f = fopen(DUPEFILE, "ab");

	if (!f)
		return 0;

	fwrite(&df, sizeof(df), 1, f);
	fclose(f);

	return 1;
}

int gl_dirlog_add(char *dn, ushort uid, ushort gid, ushort files, long bytes) {
	FILE *f;
	struct dirlog dl;

	bzero(&dl, sizeof(dl));
	strcpy(dl.dirname, dn);
	dl.uploader = uid;
	dl.group = gid;
	dl.files = files;
	dl.bytes = bytes;
	time(&dl.uptime);
	dl.status = 0;

	f = fopen(DIRLOG, "ab");

	if (!f)
		return 0;

	fwrite(&dl, sizeof(dl), 1, f);
	fclose(f);

	return 1;
}

int gl_dirlog_update(char *dn, ushort uid, ushort gid, ushort files, long bytes, int status) {
	FILE *f;
	struct dirlog dl;
	int found;

	f = fopen(DIRLOG, "rb+");

	if (!f)
		return 0;

#ifndef WIN32
	flock(fileno(f), LOCK_EX);
#endif

	while (fread(&dl, sizeof(dl), 1, f)) {
		if (!strcmp(dl.dirname, dn)) {
			dl.files += files;
			dl.bytes += bytes;
			dl.status = (status == -1) ? dl.status : status;

			fseek(f, 0-sizeof(dl), SEEK_CUR);
			fwrite(&dl, sizeof(dl), 1, f);
			found++;
		}
	}

	if (!found) {
		bzero(&dl, sizeof(dl));
		dl.uploader = uid;
		dl.group = gid;
		dl.files = files;
		dl.bytes = bytes;
		strcpy(dl.dirname, dn);
		dl.status = (status == -1) ? 0 : status;
		time(&dl.uptime);

		fwrite(&dl, sizeof(dl), 1, f);
	}

#ifndef WIN32
	flock(fileno(f), LOCK_UN);
#endif

	fclose(f);

	return 1;
}

int gl_dupelog_add(char *rel) {
	FILE *f;
	char timebuf[50];
	long now;
	struct tm *tm;

	time(&now);
	tm = localtime(&now);
	if (!tm)
		return 0;

	strftime(timebuf, 50, "%m%d%g", tm);

	f = fopen(DUPELOG, "a");
	if (!f)
		return 0;

	fprintf(f, "%s %s\n", timebuf, rel);
	fclose(f);

	return 1;
}

int gl_gllog_add(char *str) {
    FILE *f;
    time_t t;
    char buf[300], *p;

    t = time(0);

    strftime(buf, 300, "%a %b %d %T %Y", localtime(&t));

    /*
    sprintf(buf, ctime(&t));
    p = (char*)&buf;
    while (*p)
	if (*p == '\n')
	    *p = 0;
	else
	    p++;
    */

    f = fopen(GLFTPDLOG, "a");
    if (!f)
	return 0;

    fprintf(f, "%s %s\n", buf, str);
    fclose(f);

    return 1;
}

int gl_gllog_announce(char *type, char *str) {
    char buf[1024];

    sprintf(buf, "%s: \"%s\"", type, str);

    return gl_gllog_add(buf);
}

int gl_site_msg(char *from, char *to, char *msg) {
	FILE *f;
	char buf[300], *p;
	long t;

	if (!to)
		return 0;

	sprintf(buf, "%s/%s", GLMSGPATH, to);
	f = fopen(buf, "a");

	if (!f)
		return 0;

	t = time(0);
	sprintf(buf, ctime(&t));
	p = (char*)&buf;
	while (*p)
		if (*p == '\n')
			*p = 0;
		else
			p++;

	fprintf(f, "
From: %s (%s)
--------------------------------------------------------------------------

%s
", from, buf, msg);

	fclose(f);

	return 1;
}
