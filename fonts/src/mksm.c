#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
*	mksm - convert wscons font to user-defined size symbol map
*	make symbol map
*/

int getcolzz(char *s) {
	int i = 0, cols;

	while (s[i]) {
		if (isdigit(s[i]))
			++i;
		else
			return -1;
	}

	cols = atoi(s);

	if(cols >= 1 && cols <= 256 && !(256 % cols))
		return cols;
	else
		return 0;
}

void help() {
	printf("program:\n\t
					mksm - make special symbol's map with wscons font\n
					syntax:\n\t
					mksm [[columns#]] [wscons font] [[output .sm-file]]\n
					examples:\n\t
					$ mksm somefnt.816 somefnt.816.mksm\n\t
					$ mksm 16 cp1251.816 > cp1251.sm\n\t
					$ mksm 1 vt220h.814 | smtobmp > vt220h.814.bmp\n
					notes:\n\t
					1) Default columns number is 256.\n\t
					2) Columns decimanl integer >= 1 and <= 256.\n\t
					3) Modulo(256/Columns) must be 0.\n\t
					4) .sm is special graphical format file \n"
	);
}

int main(int argc, char **argv) {
	FILE *in, *out;
	int h = 0, cols;

	switch(argc) {
		case 3:	
			if((cols = getcolzz(argv[1])) == -1) {
				in = fopen(argv[1], "r");
				out = fopen(argv[2], "w");
				cols = 256;
			} 
			else if(cols == 0) {
				perror("check your columns arg.\n");
				return -1;
			}
			else {
				in = fopen(argv[2],"r");
				out = stdout;
				cols = cols;
			}
			break;

		case 4:
			if((cols = getcolzz(argv[1])) <= 0) {
				perror("check your columns arg.\n");
				return -1;
			}
			in = fopen(argv[2], "r");
			out = fopen(argv[3], "w");
			break;

		case 2:
			if(!strcmp(argv[1], "-h"))
				help();
			else {
				in = fopen(argv[1], "r");
				out = stdout;
				cols = 256;
			}
			break;

		default:
			help();
			return 0;
	}

	if(!in || !out) {
		fclose(in);
		fclose(out);
		perror("i/o\n.");
		return -1;
	}
	fseek(in, 0, SEEK_END);
	h = ftell(in) / 256;
	rewind(in);
	if(h == 16 || h == 8 || h == 14 || h == 10);
	else {
		perror("unknown font height.\n(must be: 16,8,14 or 10\n");
		fclose(in);
		return -1;
	}

	/* input and output streams are ready: */
	{
		int l, c, r, rows;
		long o;
		unsigned char b;

		rewind(out);
		b = cols - 1;
		fwrite(&b, 1, 1, out);

		/*
		 * now extract and combine symbol cells into symbol matrix with
		 * user-defined cols x rows
		 */
		rows = 256 / cols;
		for (r = 0; r < rows; ++r)
			for (l = 0; l < h; ++l)
				for (c = 0; c < cols; ++c) {
					fseek(in, r * cols * h + c * h + l, SEEK_SET);
					fread(&b, 1, 1, in);
					fwrite(&b, 1, 1, out);
				}
	}

	fclose(in);
	fclose(out);
	return 0;
}
