#include <stdio.h>

/*
	smtofont - .sm file to wscons font file
*/

int main(int argc, char **argv) {
	FILE *in, *out;

	switch(argc) {
		case 1: {
			unsigned char b;
			in = fopen("/tmp/.smtofont.tmp", "w+");
			while(1) {
				fread(&b,1,1,stdin);
				if(feof(stdin))
					break;
				else
					fwrite(&b,1,1,in);
			}
		}
		rewind(in);
		out = stdout;
		break;

		case 2:
			if(!strcmp(argv[1], "-h")) {
				printf("program:\n\t
								smtofont - convert .sm file to wscons font file\n
								syntax:\n\t
								smtofile [[-h]] [[.sm]] [[font outfile]]\n
								examples:\n\t
								$ cat somefnt.sm | smtofont > somefnt.wsf\n\t
								$ smtofont smfnt.sm smfnt.wsf\n"
				);
				return 0;
			}
			else {
				in = fopen(argv[1], "r");
				out = stdout;
			}
			break;

		case 3:
			in = fopen(argv[1], "r");
			out = fopen(argv[2], "w");
			break;

		default:
			perror("bad args list.\n");
			return -1;
	}

	{
		int i = 0, cols, rows, fheight;
		unsigned char b, *buf;
		long size;

		fseek(in, 0, SEEK_END);
		size = ftell(in) - 1;
	
		if(size != 4096 && size != 2048 && size != 2560 && size != 3584) {
			perror("bad .sm file size.\n");
			return -1;
		}

		rewind(in);
		cols = fgetc(in);
		if(256 % ++cols != 0 || cols < 1 && cols > 256) {
			perror("wrong columns value at .sm-header.\n");
			return -1;
		}

		buf = (unsigned char *) malloc(size);
		fread(buf, size, 1, in);

		fheight = size / 256;
		rows = 256 / cols;

		{
		int r,c,l;
		for (r=0;r<rows;++r)
			for (c=0;c<cols;++c)
				for(l=0;l<fheight;++l)
				fwrite( buf+r*cols*fheight+l*cols+c,1,1,out);
		}
	}
	fclose(in);
	fclose(out);
	return 0;
}
