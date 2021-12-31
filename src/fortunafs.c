#include "fortunafs.h"

#define TRY(__r) { if ((__r) != F_OK) return __r; }

FResult ffs_mkfs(FFS* ffs, uint8_t partition_nr, uint32_t metadata_mb, uint32_t content_mb)
{
    if (!ffs || !ffs->buffer || !ffs->read_f || !ffs->write_f)
        return F_INVALID_CALL;

    TRY(partition_find_start(ffs, &ffs->partition_start))

    return F_NOT_IMPLEMENTED;
}

FResult ffs_init(FFS* ffs, uint8_t partition_nr)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_put(FFS* ffs, uint64_t* index, uint16_t bytes)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_get(FFS* ffs, uint64_t* index, uint16_t* bytes)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_delete(FFS* ffs)
{
    return F_NOT_IMPLEMENTED;
}

FResult ffs_listkeys(FFS* ffs, uint64_t* index)
{
    return F_NOT_IMPLEMENTED;
}

