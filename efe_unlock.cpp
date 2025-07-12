#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

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

#define BUFFER_SIZE 8192

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		const char *filename = argv[i];

		char out_filename[1024];
		strncpy(out_filename, filename, sizeof(out_filename));
		int len = strlen(out_filename);

		if (len >= 9 && strcmp(out_filename + len - 9, ".flac.efe") == 0) {
			out_filename[len - 9] = '\0';
			strcat(out_filename, ".flac");
		} else if (len >= 4 && strcmp(out_filename + len - 4, ".efe") == 0) {
			out_filename[len - 4] = '\0';
			strcat(out_filename, ".ogg");
		} else if (len >= 5 && strcmp(out_filename + len - 5, ".mqcc") == 0) {
			out_filename[len - 5] = '\0';
			strcat(out_filename, ".ogg");
			rename(filename, out_filename);
			printf("Renamed: %s -> %s\n", filename, out_filename);
			continue;
		} else {
			printf("Not supported format: %s", filename);
			continue;
		}

		FILE *in = fopen(filename, "rb");
		if (!in) {
			printf("Error opening input file: %s", filename);
			continue;
		}

		FILE *out = fopen(out_filename, "wb");
		if (!out) {
			printf("Error opening out_filename file: %s", out_filename);
			fclose(in);
			continue;
		}

		unsigned char buffer[BUFFER_SIZE];
		size_t total_bytes = 0;

		while (1) {
			size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, in);
			if (bytes_read == 0) break;

			for (size_t j = 0; j < bytes_read; j++)
				if (total_bytes< 0x514000)
					buffer[j] ^= xor_table[total_bytes++ & 63];

			fwrite(buffer, 1, bytes_read, out);
			total_bytes += bytes_read;
		}

		fclose(in);
		fclose(out);

		printf("Processed: %s -> %s\n", filename, out_filename);
	}

	return 0;
}
