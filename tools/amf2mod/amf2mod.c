// AMF->MOD converter
// made in 1995 by Mr. P / Powersource
// mrp@fish.share.net, ac054@sfn.saskatoon.sk.ca
// This program is hereby placed in the public domain.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main(int ac, char *av[])
{
	char *inpt,*outpt;
	FILE *inf,*outf;
	int j;
	unsigned char *workbuf;
	char orders[256];
	int workptr;
	unsigned long i,numpats,k;
	unsigned long length,repstart,replen;
	char insstruc[30];
	int numorders;
	unsigned int period,ins,effect;
	long percentile;

	int periodtable[]={6848,6464,6096,5760,5424,5120,4832,4560,4304,
			   4064,3840,3628,3424,3232,3048,2880,2712,2560,
			   2416,2280,2152,2032,1920,1814,1712,1616,1524,
			   1440,1356,1280,1208,1140,1076,1016, 960, 907,
			    856, 808, 762, 720, 678, 640, 604, 570, 538,
			    508, 480, 453, 428, 404, 381, 360, 339, 320,
			    302, 285, 269, 254, 240, 226, 214, 202, 190,
			    180, 170, 160, 151, 143, 135, 127, 120, 113,
			    107, 101,  95,  90,  85,  80,  75,  71,  67,
			     63,  60,  56,  53,  50,  47,  45,  42,  40,
			     37,  35,  33,  31,  30,  28};

	if (ac<3) {
		printf("syntax: AMF2MOD inputfile outputfile\n");
		exit(1);
	}

	inpt=av[1];
	outpt=av[2];

	inf=fopen(inpt,"rb");
	outf=fopen(outpt,"wb");

	if (!inf) {
		printf ("Unable to open file for reading '%s'\n", inpt);
		exit(-1);
	}
	else if (!outf) {
		printf ("Unable to open file for writing '%s'\n", outpt);
		exit(-1);
	}

	// AMF conversion will take place now.
	printf("Converting %s from AMF to MOD, %s as output\n\n",inpt,outpt);
	printf("Translating header:     [ ]");

	workbuf = (unsigned char *)malloc(64*37);
	// no title for MOD
	memset(workbuf,0,64*37);
	fwrite(workbuf,1,20,outf);

	// Skip past ASYLUM Music Format v1.0
	fseek(inf,32,0);

	// load valuable data
	fread(workbuf,1,6,inf);
	numorders=workbuf[4];
        numpats=workbuf[3];

	// load orders
	memset(orders,0,256);
	fread(orders,1,numorders,inf);
	fseek(inf,294,0);

	printf("\b\bX]\n");
	printf("Translating instruments [ ]");

	// load instruments
	fread(workbuf,37,64,inf);

	workptr=0;

	for (i=0;i<31;i++) {
		// status report
		printf("\b\b|]");
		// now copy name
		strcpy(insstruc,(char*) &(workbuf[workptr]));
		// read in instrument data
		insstruc[24]=workbuf[workptr+22];
		// copy the rest
		insstruc[25]=workbuf[workptr+23];
		length=((workbuf[workptr+25])     |
		       ((workbuf[workptr+26])<<8) |
		       ((workbuf[workptr+27])<<16)|
		       ((workbuf[workptr+28])<<24))/2;

		insstruc[22]=((length>>8)&0xff);
		insstruc[23]=(length&0xff);
		printf("\b\b/]");

		repstart=((workbuf[workptr+29]&0xff)     |
			 ((workbuf[workptr+30]&0xff)<<8) |
			 ((workbuf[workptr+31]&0xff)<<16)|
			 ((workbuf[workptr+32]&0xff)<<24))/2;

		insstruc[26]=((repstart>>8)&0xff);
		insstruc[27]=(repstart&0xff);
		printf("\b\b-]");

		replen=((workbuf[workptr+33]&0xff)     |
		       ((workbuf[workptr+34]&0xff)<<8) |
		       ((workbuf[workptr+35]&0xff)<<16)|
		       ((workbuf[workptr+36]&0xff)<<24))/2;

		insstruc[28]=((replen>>8)&0xff);
		insstruc[29]=(replen&0xff);
		printf("\b\b\\]");

		workptr+=37;
		// now write
		fwrite(insstruc,1,30,outf);
	}
	printf("\b\bX]\n");

	printf("Translating orders      [ ]");
	// now, misc data
	insstruc[0]=numorders;
	insstruc[1]=127;
	fwrite(insstruc,1,2,outf);
	// and orders
	fwrite(orders,1,128,outf);
	// finally, 8CHN
	insstruc[0]='8';
	insstruc[1]='C';
	insstruc[2]='H';
	insstruc[3]='N';
	fwrite(insstruc,1,4,outf);
//	gotoxy(26,wherey());
	printf("\b\bX]\n");
	printf("Translating patterns    00%%");

	// now tackle patterns
	for(i=0;i<numpats;i++) {
		// load a pattern
		fread(workbuf,64,32,inf);
		// conversion
		for(j=0;j<512;j++) {
			// convert period->note;
			if (workbuf[j*4] != 0) {
				// lookup
				period=periodtable[workbuf[j*4]];
			} else
				period=0;
			ins=workbuf[j*4+1];
			// asm is the only way
			effect=(workbuf[j*4+2] * 256)|workbuf[j*4+3];
			// encode it
			insstruc[0]=(ins&0xf0)|((period&0xf00)>>8);
			insstruc[1]=(period&0xff);
			insstruc[2]=((ins&0xf)<<4)|((effect&0xf00)>>8);
			insstruc[3]=(effect&0xff);
			// write it
			fwrite(insstruc,1,4,outf);
		}
		// status display
		percentile=((100*(i*2048))/(2048*numpats));
		printf("\b\b\b%2ld%%",percentile);
	}
	printf("\b\b\b[X]\n");

	printf("Copying sample data     00%%");
	i=2662+(2048*numpats);
	// first, find our limit
	fseek(inf,0,2);
	k=ftell(inf);
	fseek(inf,i,0);
	// read 4096-byte blocks until there is no more room
	free(workbuf);
	workbuf=(unsigned char *)malloc(31*1024);
	while((k-i)>=31*1024) {
		fread(workbuf,31*1024,1,inf);
		fwrite(workbuf,31*1024,1,outf);
		i+=31*1024;

		percentile=((100*i)/k);
		printf("\b\b\b%2ld%%",percentile);
	}
	// finish it off
	if ((k-i) != 0) {
		fread(workbuf,k-i,1,inf);
		fwrite(workbuf,k-i,1,outf);
	}

	free(workbuf);
	printf("\b\b\b[X]\n");

	fclose(inf);
	fclose(outf);
	printf("\nConversion successful.\n");
	return 0;
}

