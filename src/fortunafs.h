#ifndef FORTUNAFS_H_
#define FORTUNAFS_H_

#include <stdint.h>

typedef enum FResult {
    F_OK                = 0x00,
    F_IO_ERROR          = 0x01,
    F_INVALID_CALL      = 0xfe,
    F_NOT_IMPLEMENTED   = 0xff,
} FResult;

typedef struct FFS {
    uint8_t* buffer;
    FResult  (*read_f)(uint64_t sector, uint8_t* buffer);
    FResult  (*write_f)(uint64_t sector, uint8_t const* buffer);
    uint32_t partition_start;
    uint32_t hash_start;
    uint32_t metadata_start;
    uint32_t cat_start;
    uint32_t content_start;
} FFS;

FResult ffs_mkfs(FFS* ffs, uint8_t partition_nr,
        uint32_t metadata_mb, uint32_t content_mb);

FResult ffs_init(FFS* ffs, uint8_t partition_nr);

FResult ffs_put(FFS* ffs, uint64_t* index, uint16_t bytes);
FResult ffs_get(FFS* ffs, uint64_t* index, uint16_t* bytes);
FResult ffs_delete(FFS* ffs);
FResult ffs_listkeys(FFS* ffs, uint64_t* index);

#endif
