#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MOD_SOUNDTRACKER 0x2e4b2e4d

#define RATE 11025
// #define RATE 8287

struct sample {
    char name[22];
    uint16_t length;
    uint8_t tune;
    uint8_t volume;
    uint16_t loop_start;
    uint16_t loop_length;
} __attribute__((packed));

struct patternlist {
    uint8_t length;
    uint8_t ignored;
    uint8_t table[128];
    uint32_t type;
} __attribute__((packed));

struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    uint32_t wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"
    
    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    uint32_t fmt_chunk_size; // Should be 16 for PCM
    uint16_t audio_format; // Should be 1 for PCM. 3 for IEEE Float
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    uint16_t sample_alignment; // num_channels * Bytes Per Sample
    uint16_t bit_depth; // Number of bits per sample
    
    // Data
    char data_header[4]; // Contains "data"
    uint32_t data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} __attribute__((packed));

static inline void usage() {
    printf("Usage: modextract [-r rate] mod.filename\n");
}

int main(int argc, char **argv) {

    int rate = RATE;
    char *module = NULL;

    int opt = 0;

    while ((opt = getopt(argc, argv, "r:")) != -1) {
        switch (opt) {
            case 'r':
                rate = atoi(optarg);
                break;
            default:
                usage();
                return -1;
        }
    }

    if (optind >= argc) {
        usage();
        return -1;
    }

    module = argv[optind];

    int acc = access(module, R_OK);

    if (acc != 0) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    int fd = open(module, O_RDONLY);
    if (!fd) {
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    char modname[21] = {0};
    read(fd, modname, 20);
    printf("Module name: %s\n", modname);

    struct sample s[31];
    int i;
    for (i = 0; i < 31; i++) {
        read(fd, &s[i], sizeof(struct sample));
        s[i].length = be16toh(s[i].length);
        s[i].loop_start = be16toh(s[i].loop_start);
        s[i].loop_length = be16toh(s[i].loop_length);
        printf("Sample %2d: %s (%d words)\n", i, s[i].name, s[i].length);
    }

    struct patternlist pl;

    read(fd, &pl, sizeof(struct patternlist));

    switch (pl.type) {
        case MOD_SOUNDTRACKER:
            printf("Type: Soundtracker\n");
            break;
        default:
            printf("Type: Unknown (0x%08x)\n", pl.type);
            printf("Can't extract this type of module. Aborting.\n");
            return -1;
            break;
    }

    int maxpat = 0;

    for (i = 0; i < pl.length; i++) {
        if (pl.table[i] > maxpat) maxpat = pl.table[i];
    }

    uint8_t pattern[1024];

    int j;

    for (i = 0; i <= maxpat; i++) {
        read(fd, pattern, 1024);
    }

    printf("Samples start at %lu\n", lseek(fd, 0, SEEK_CUR));

    struct wav_header header;
    memcpy(header.riff_header, "RIFF", 4);
    memcpy(header.wave_header, "WAVE", 4);
    memcpy(header.fmt_header, "fmt ", 4);
    memcpy(header.data_header, "data", 4);
    header.fmt_chunk_size = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = rate;
    header.byte_rate = header.sample_rate * 2;
    header.sample_alignment = 2;
    header.bit_depth = 16;

    uint32_t wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    uint32_t data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size

    for (i = 0; i < 31; i++) {
        char tmp[100];
        if (s[i].length == 0) continue;

        mkdir("samples", 0755);
        snprintf(tmp, 100, "samples/%s", modname);
        mkdir(tmp, 0755);

        snprintf(tmp, 100, "samples/%s/%02d - %s.wav", modname, i, s[i].name);
        printf("Saving: %s\n", tmp);

        header.wav_size = s[i].length * 4 + sizeof(struct wav_header) - 8;
        header.data_bytes = s[i].length * 4;


        int fd2 = open(tmp, O_RDWR | O_CREAT, 0644);
        write(fd2, &header, sizeof(struct wav_header));
        for (j = 0; j < s[i].length * 2; j++) {
            int8_t v8;
            read(fd, &v8, 1);
            int16_t v16 = v8 * 256;
            write(fd2, &v16, 2);
        }
        close(fd2);
    }

    return 0;
}
