#include "fortunafs.h"

#include <string.h>

#define SECTORS_PER_MB ((1024 * 1024) / 512)
#define FORTUNAFS_SIGNATURE 0xd35f389f
#define FORTUNAFS_VERSION   1

#define BOOT_SIZE_POS 0x20

#define MAX_BOOT_SIZE (8191 * 512)

#define CONFIG_SECTOR 0
#define BOOT_SECTOR   1

#define TRY(__r) { if ((__r) != F_OK) return __r; }

#define BOOT_MAGIC_KEY "@boot"

// region Utils

static void to_16(uint8_t *buffer, uint16_t pos, uint16_t data)
{
    buffer[pos] = data & 0xff;
    buffer[pos+1] = data >> 8;
}

static void to_32(uint8_t *buffer, uint16_t pos, uint32_t data)
{
    buffer[pos] = data & 0xff;
    buffer[pos+1] = data >> 8;
    buffer[pos+2] = data >> 16;
    buffer[pos+3] = data >> 24;
}

static void to_64(uint8_t *buffer, uint16_t pos, uint64_t data)
{
    to_32(buffer, pos, data & 0xffffffff);
    to_32(buffer, pos + 4, data >> 32);
}

// endregion

// region -> Partitions

static FResult partition_find_start(FFS* f, uint8_t partition_nr, uint32_t* partition_start)
{
    (void) f;

    if (partition_nr == 0) {
        *partition_start = 0;
        return F_OK;
    } else {
        return F_NOT_IMPLEMENTED;  // TODO
    }
}

// endregion

// region -> Boot

static FResult update_boot_size(FFS* ffs, uint64_t size)
{
    TRY(ffs->read_f(CONFIG_SECTOR, ffs->buffer, ffs->data))
    to_32(ffs->buffer, BOOT_SIZE_POS, size);
    TRY(ffs->write_f(CONFIG_SECTOR, ffs->buffer, ffs->data))
    return F_OK;
}

// endregion

// region -> mkfs

static FResult calculate_content_size_sectors(FFS *f, uint64_t total_size_mb, uint64_t *content_size)
{
    (void) f;
    *content_size = (total_size_mb * SECTORS_PER_MB) / 65.0 * 64;
    return F_OK;
}

static FResult create_filesystem(FFS *f, uint64_t content_size)
{
    memset(f->buffer, 0,  512);
    to_32(f->buffer, 0x0, FORTUNAFS_SIGNATURE);
    to_16(f->buffer, 0x4, FORTUNAFS_VERSION);
    to_32(f->buffer, 0x8, f->hash_start);
    to_32(f->buffer, 0xc, f->metadata_start);
    to_32(f->buffer, 0x10, f->cat_start);
    to_32(f->buffer, 0x14, f->content_start);
    to_64(f->buffer, 0x18, content_size);
    to_64(f->buffer, 0x30, content_size);

    TRY(f->write_f(CONFIG_SECTOR, f->buffer, f->data))

    return F_OK;
}

// endregion

// region -> Public functions

FResult ffs_mkfs(FFS* f, uint8_t partition_nr, uint32_t metadata_mb, uint64_t total_size_mb)
{
    // TODO - check for little endianess

    if (!f || !f->buffer || !f->read_f || !f->write_f)
        return F_INVALID_CALL;

    TRY(partition_find_start(f, partition_nr, &f->partition_start))

    // calculate locations
    f->hash_start = 8192;
    f->metadata_start = f->hash_start + (64 * SECTORS_PER_MB);
    f->cat_start = f->metadata_start + (metadata_mb / SECTORS_PER_MB);
    
    uint64_t content_size;
    TRY(calculate_content_size_sectors(f, total_size_mb, &content_size))
    f->content_start = f->cat_start + (content_size / 64);

    // create filesystem
    TRY(create_filesystem(f, content_size))

    return F_OK;
}

FResult ffs_init(FFS* f, uint8_t partition_nr)
{
    (void) f; (void) partition_nr;
    return F_NOT_IMPLEMENTED;
}

FResult ffs_putkey(FFS* ffs, uint64_t* key_index, uint64_t value_size)
{
    (void) ffs; (void) key_index; (void) value_size;

    if (strcmp((const char *) ffs->buffer, BOOT_MAGIC_KEY) == 0) {  // special case: @boot
        if (value_size >= MAX_BOOT_SIZE)
            return F_BOOT_TOO_LARGE;
        TRY(update_boot_size(ffs, value_size))
        *key_index = BOOT_SECTOR;
    } else {
        return F_NOT_IMPLEMENTED;  // TODO
    }

    return F_OK;
}

FResult ffs_putvalue(FFS* ffs, uint64_t key_index, uint64_t* value_index, uint16_t bytes)
{
    (void) ffs; (void) key_index; (void) value_index; (void) bytes;

    if (key_index == BOOT_SECTOR) {  // special case: @boot
        if (*value_index == 0)
            *value_index = BOOT_SECTOR;
        if (bytes < 512)
            memset(&ffs->buffer[bytes], 0, 512 - bytes);
        TRY(ffs->write_f(*value_index, ffs->buffer, ffs->data))
        ++(*value_index);
        return F_OK;
    }

    return F_NOT_IMPLEMENTED;
}

// endregion