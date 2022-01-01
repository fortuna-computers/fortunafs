#include "fortunafs.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define ONE_MB (1024 * 1024)

#define TRY_FS(__r) { if ((__r) != F_OK) { fprintf(stderr, "Failure during fortunafs operation: 0x%X\n", __r); exit(EXIT_FAILURE); } }

extern int truncate(const char *path, off_t length);

static void print_help(__FILE *f, const char* program)
{
    fprintf(f, "Usage: %s [-C image_sz_mb] [-P partition_nr] [-M metadata_mb] [-b boot_file] device\n", program);
    fprintf(f, "Arguments:\n");
    fprintf(f, "    -C       Create a new image file instead of using an existing one.\n");
    fprintf(f, "             The image size in MB needs to be defined.\n");
    fprintf(f, "    -P       Which partition to use - uses 0 for an image without a partition table.\n");
    fprintf(f, "             When creating an image, also defines the number of partitions.\n");
    fprintf(f, "    -M       Metadata table size, in MB. If not defined, uses 128 MB.\n");
    fprintf(f, "    -b       Add boot file to image.\n");
    fprintf(f, "    device   The device can be a physical device under /dev, or an image file.\n");
}

static FResult read_f(uint64_t sector, uint8_t* buffer, void* data)
{
    FILE* f = data;
    if (fseek(f, sector * 512, SEEK_SET) < 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    if (fread(buffer, 512, 1, f) == 0) {
        perror("fread");
        exit(EXIT_FAILURE);
    }
    return F_OK;
}

static FResult write_f(uint64_t sector, uint8_t const* buffer, void* data)
{
    FILE* f = data;
    if (fseek(f, sector * 512, SEEK_SET) < 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    if (fwrite(buffer, 512, 1, f) == 0) {
        perror("fwrite");
        exit(EXIT_FAILURE);
    }
    return F_OK;
}

static void add_boot_file(FFS *ffs, const char *boot_file)
{
    uint64_t key_index = 0,
             value_index = 0;

    FILE* f = fopen(boot_file, "rb");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    strcpy((char *) ffs->buffer, "@boot");
    fseek(f, 0, SEEK_END);
    TRY_FS(ffs_putkey(ffs, &key_index, ftell(f)))
    fseek(f, 0, SEEK_SET);

    while (!feof(f)) {
        unsigned long r = fread(ffs->buffer, 1, 512, f);
        TRY_FS(ffs_putvalue(ffs, key_index, &value_index, r))
    }

    fclose(f);
}

static void create_image(const char *device, uint8_t partition_nr, unsigned long metadata_mb, const char* boot_file)
{
    struct stat statinfo;
    if (stat(device, &statinfo) < 0) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    FILE* f = fopen(device, "r+b");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fseek(f, 0, SEEK_SET);

    uint8_t buffer[512] = { 0 };
    FFS ffs = {
        .buffer = buffer,
        .read_f = read_f,
        .write_f = write_f,
        .data = f,
    };
    FResult r = ffs_mkfs(&ffs, partition_nr, metadata_mb, statinfo.st_size / ONE_MB);
    if (r != F_OK) {
        fprintf(stderr, "Failure during mkfs: 0x%X\n", r);
        exit(EXIT_FAILURE);
    }

    if (boot_file)
        add_boot_file(&ffs, boot_file);

    fclose(f);
}

static void create_new_image(unsigned long long int new_file_mb, const char *device)
{
    FILE* f = fopen(device, "w");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if (fseek(f, new_file_mb * ONE_MB - 1, SEEK_SET) < 0) {
        perror("fseek");
        exit(EXIT_FAILURE);
    }
    char c = 0;
    fwrite(&c, 1, 1, f);
    fclose(f);
}

int main(int argc, char* argv[])
{
    unsigned long long create_new   = 0;
    uint8_t            partition_nr = 0;
    unsigned long      metadata_mb  = 0;
    const char*        boot_file    = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "hC:P:M:b:")) != -1) {
        switch (opt) {
            case 'h':
                print_help(stdout, argv[0]);
                exit(EXIT_SUCCESS);
            case 'C':
                create_new = strtoull(optarg, NULL, 10);
                if (create_new == ULLONG_MAX) {
                    fprintf(stderr, "Invalid image size.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'P':
                partition_nr = strtoul(optarg, NULL, 10);
                if (partition_nr > 4) {
                    fprintf(stderr, "Invalid partition number.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'M':
                metadata_mb = strtoul(optarg, NULL, 10);
                if (metadata_mb == ULONG_MAX) {
                    fprintf(stderr, "Invalid metadata table size.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
                boot_file = optarg;
                break;
            default:
                print_help(stderr, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind != argc - 1) {
        fprintf(stderr, "Incorrect number of arguments.\n");
        exit(EXIT_FAILURE);
    }

    const char* device = argv[optind];

    if (create_new)
        create_new_image(create_new, device);

    create_image(device, partition_nr, metadata_mb, boot_file);

    return EXIT_SUCCESS;
}