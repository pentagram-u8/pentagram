/*
Copyright (C) 2005-2006 The Pentagram team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char* basename(const char* filename)
{
	const char* base;
	const char* x;

	base = filename;

	for (x = filename; *x != 0; ++x) {
		if (*x == '/' || *x == '\\')
			base = x+1;
	}

	return base;
}

int scanname(const char* name)
{
	const char* x;
	char c;

	for (x = name; *x != 0; ++x) {
		c = *x;
		if (c < 'A' && c > 'Z' && c < 'a' && c > 'z' &&
			c < '0' && c > '9' && c != '_' && c != '.' && c != '-')
			return 0;
	}

	return 1;
}

int main(int argc, char* argv[])
{
	int i;
	const char* outfilename = 0;
	FILE* outfile;
	unsigned int* filesizes;

	if (argc < 2) {
		fprintf(stderr, "Not enough arguments: %s output.h input_files \n",
				argv[0]);
		return 1;
	}

	outfilename = argv[1];
/*	printf("Output file: %s\n", outfilename);*/
	outfile = fopen(outfilename, "w");
	if (!outfile) {
		fprintf(stderr, "Error opening output file: %s\n", outfilename);
		return 1;
	}

	fprintf(outfile, "/* This file was generated automatically. DO NOT EDIT! */\n\n");

	fprintf(outfile, "namespace PentagramData {\n");

	filesizes = (unsigned int*)malloc((argc-2)*sizeof(filesizes[0]));

	for (i = 2; i < argc; ++i) {
		const char* filename;
		const char* base;
		FILE* infile;
		int index;
		unsigned char buf[1024];
		unsigned int read;
		unsigned int j;
		int first;

		filename = argv[i];
		base = basename(filename);
		index = i-2;

/*
		printf("Input file: %s\n", filename);
		printf("base: %s\n", base);
*/

		if (strlen(filename) == 0 || strlen(base) == 0 || !scanname(base)) {
			fprintf(stderr, "Invalid filename.");
			fclose(outfile);
			remove(outfilename);
			return 1;
		}

		infile = fopen(filename, "rb");
		if (!infile) {
			fprintf(stderr, "Error opening input file: %s\n", filename);
			fclose(outfile);
			remove(outfilename);
			return 1;
		}

		fprintf(outfile, "static const uint8 data%d[] = {\n", index);
		first = 1;
		filesizes[index] = 0;

		do {
			unsigned int s;

			read = fread(buf, 1, 1024, infile);
			filesizes[index] += read;
			for (j = 0; j < read/8; j++) {
				fprintf(outfile, "%s0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X", (first?"":",\n"),buf[j*8],buf[j*8+1],buf[j*8+2],buf[j*8+3],buf[j*8+4],buf[j*8+5],buf[j*8+6],buf[j*8+7]);
				first = 0;
			}
			s = j*8;
			for (j = 0; j < read%8; j++) {
				fprintf(outfile, "%s0x%02X", j?",":(first?"":",\n"), buf[s+j]);
			}

		} while (read == 1024);

		fprintf(outfile, "\n};\n");

		fclose(infile);
	}

	fprintf(outfile, "struct DataFile { const char* name; const uint8* data; unsigned int size; };\n");

	fprintf(outfile, "DataFile files[] = {\n");

	for (i = 2; i < argc; ++i) {
		const char* filename;
		const char* base;
		int index;

		filename = argv[i];
		base = basename(filename);
		index = i-2;

		fprintf(outfile, "{ \"%s\", data%d, %d },\n", base, index, filesizes[index]);
	}

	fprintf(outfile, "{ 0, 0, 0 }\n};\n");

	fprintf(outfile, "}\n");

	fclose(outfile);

	return 0;
}
