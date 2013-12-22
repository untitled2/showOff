#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
	smtobmp
	convert mksm-output file (.sm) to windows monochrome bitmap (.bmp)
*/

int main(int argc, char **argv) {
	FILE *in, *tmp, *out;
	long datasize;

	unsigned char *buf;

	int ln, i=0;
	int cellsX, cellsY, imgX, imgY;

	struct bmpheader {
		int	bfSize;
		short	reserved0, reserved1;
		int	BiOffBits, BiSize, BiWidth, BiHeight;
		short	BiPlanes,	BiBitCnt;
		int	BiCompr, BiSizeImage,	biXPels_M, biYPels_M,	BiClrUsed, BiClrImprt;
		char pal[8];
	} hd;

	if(argc>3) {
		perror("too many args\n");
		return -1;
	}

	if(argc==2 && !strcmp(argv[1],"-h")) {
		printf("program:\n\t
						smtobmp - convert .mksm-file to .bmp monochrome image.\n
						syntax:\n\t
						$ smtobmp [[.sm]] [[.bmp]] \n
						examples:\n\t
						$ mksm somefont.816 | smtobmp > symbols.bmp\n\t
						$ smtobmp font.sm font.bmp\n\t
						$ cat somefont.sm | smtobmp | bmptosm | mkfn > myfont.wscons\n"
		);
		return 0;
	}

	if(argc==1) {
		unsigned char b;
		tmp = fopen("/tmp/.smtobmp.in.tmp","w+");
	
		while(1) {
			fread(&b,1,1,stdin);
			if (feof(stdin)) 
				break;
			fwrite(&b,1,1,tmp);
		}

		fclose(stdin);
		in = tmp;
		out = stdout;
		rewind(in);
	}

	if(argc==2) {
		in = fopen(argv[1], "r");
		out = stdout;
	}

	if(argc==3) {
		in = fopen(argv[1], "r");
		out = fopen(argv[2], "w");
	}

	if(!in || !out) {
		perror("i/o");
		fclose(in);
		fclose(out);
		return -1;
	}
	
	fseek(in,0,SEEK_END);
	datasize = ftell(in) - 1;
	rewind(in);

	cellsX = fgetc(in) + 1;	// font table width (in cells)
	cellsY = 256/cellsX;	// font table height (in cells) 
	imgX = cellsX*8;	// image width (in bits i.e. pixels)
	imgY = datasize/cellsX; // image height (in pixels) 
	
	buf = (unsigned char*) malloc(datasize);
	for (ln=imgY-1; ln>=0; --ln) {
		fseek(in, 1+ln*cellsX, SEEK_SET);
		fread( buf+cellsX*i, 1, cellsX, in);
		++i;
	}
	
	hd.bfSize = datasize + 62;
	hd.reserved0 = 0;
	hd.reserved1 = 0;
	hd.BiOffBits = 62;
	hd.BiSize = 40;
	hd.BiWidth = imgX;
	hd.BiHeight = imgY;
	hd.BiPlanes = 1;
	hd.BiBitCnt = 1;
	hd.BiCompr = 0;
	hd.BiSizeImage = datasize;	
	hd.biXPels_M = 0;
	hd.biYPels_M = 0;
	hd.BiClrUsed = 2;
	hd.BiClrImprt = 2;
	hd.pal[0]=0; 	hd.pal[1]=0; 	hd.pal[2]=0;	hd.pal[3]=0;
	hd.pal[4]=255; 	hd.pal[5]=255; 	hd.pal[6]=255;	hd.pal[7]=255;
	{
		char BM[2] = {'B','M'};
		fwrite(BM,1,2,out);
	}
	fwrite(&hd,1,60,out);	
	fwrite(buf,datasize,1,out);

	fclose(in);
	fclose(out);
	return 0;
}	
