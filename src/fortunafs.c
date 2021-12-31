#include "fortunafs.h"

#define SECTORS_PER_MB ((1024 * 1024) / 512)

#define TRY(__r) { if ((__r) != F_OK) return __r; }

FResult ffs_mkfs(FFS* f, uint8_t partition_nr, uint32_t metadata_mb, uint32_t total_size_mb)
{
    if (!f || !f->buffer || !f->read_f || !f->write_f)
        return F_INVALID_CALL;

    TRY(partition_find_start(f, &f->partition_start))

    // calculate locations
    f->hash_start = 8192;
    f->metadata_start = f->hash_start + (64 * SECTORS_PER_MB);
    f->cat_start = f->metadata_start + (metadata_mb / SECTORS_PER_MB);
    
    uint64_t content_size;
    TRY(calculate_content_size_sectors(f, &content_size))
    f->content_start = 

    // TODO - create image

    return F_NOT_IMPLEMENTED;
}

FResult ffs_init(FFS* f, uint8_t partition_nr)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_put(FFS* f, uint64_t* index, uint16_t bytes)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_get(FFS* f, uint64_t* index, uint16_t* bytes)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_delete(FFS* f)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_listkeys(FFS* f, uint64_t* index)
{
    return F_NOT_IMPLEMENTED;
}
