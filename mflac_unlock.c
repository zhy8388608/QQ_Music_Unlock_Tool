#include <stdio.h>
#include <string.h>

#define KEY_SIZE 128

unsigned char xor_table[64] = {
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
	0x64, 0x65, 0x34, 0x65, 0x37, 0x36, 0x33, 0x31,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	0x30, 0x30, 0x33, 0x33, 0x63, 0x35, 0x38, 0x37,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s file1 [file2 ...]\n", argv[0]);
		return 1;
	}

	int i;
	for (i = 1; i < argc; i++) {
		const char *infile = argv[i];
		char *dot = strrchr(infile, '.');

		if (!dot || (strcmp(dot, ".mflac") && strcmp(dot, ".mflacc0"))) {
			fprintf(stderr, "Skipping: %s (invalid suffix)\n", infile);
			continue;
		}

		FILE *fin = fopen(infile, "rb");
		if (!fin) {
			perror("Failed to open input file");
			continue;
		}

		unsigned char key[KEY_SIZE], check[KEY_SIZE];
		if (fseek(fin, 0x2000, SEEK_SET) != 0 || fread(key, 1, KEY_SIZE, fin) != KEY_SIZE ||
		        fseek(fin, 0x2080, SEEK_SET) != 0 || fread(check, 1, KEY_SIZE, fin) != KEY_SIZE ||
		        memcmp(key, check, KEY_SIZE) != 0) {
			fprintf(stderr, "%s: Key and check regions do not match. Skipping.\n", infile);
			fclose(fin);
			continue;
		}

		char outfile[1024];
		size_t len = dot - infile;
		snprintf(outfile, sizeof(outfile), "%.*s.flac", (int)len, infile);

		FILE *fout = fopen(outfile, "wb");
		if (!fout) {
			perror("Failed to create output file");
			fclose(fin);
			continue;
		}

		rewind(fin);
		unsigned char buffer[1024];
		size_t n, j;
		int ki = 0;

		while ((n = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
			for (j = 0; j < n; j++) {
				if ( ki == 0x7fff )
					buffer[j] ^= key[ 0x7fff % KEY_SIZE];
				else
					buffer[j] ^= key[(ki + ki / 0x7fff) % KEY_SIZE];

				if(0x28a000 <= ki && ki < 0x514000)
					buffer[j] ^= xor_table[ki & 63];

				ki++;
			}
			fwrite(buffer, 1, n, fout);
		}

		fclose(fin);
		fclose(fout);
		printf("Processed: %s -> %s\n", infile, outfile);
	}

	return 0;
}

