# fortunafs

A key-value pair filesystem for small embedded systems. Requires a 512 RAM buffer (plus around 100 bytes), and uses less than 4 kB of ROM. A fuse filesystem is available.

The following operations can be done on the FS:

* `put`
* `get`
* `delete`
* `listkeys`
* `mkfs`

A special **@boot** key allows writing or reading the boot area.

## Image format - Sections

The image is divided in 512-byte sectors. The sections are the following:

| Section | Size, in sectors | Size, in MB |
|---------|------------------|-------------|
| Config  | 1                | < 1         |
| Boot    | 8191             | 4 MB        |
| Hash    | 131.072          | 64 MB       |


### Config area: 1 sector

Config area contains definitions about the image:

| Offset | Data size | Description                     |
|--------|-----------|---------------------------------|
| 0x00   | 4         | Image signature (`9F 38 5F D3`) |
| 0x04   | 2         | FortunaFS version               |
| 0x06   | 2         | Unused                          |
| 0x08   | 4         | Hash area start sector          |
| 0x0C   | 4         | Metadata area start sector      |
| 0x10   | 4         | Cluster allocation table start  |
| 0x14   | 4         | Content start                   |
| 0x18   | 8         | Content size                    |
| 0x20   | 4         | Boot size                       |
| 0x24   | 4         | Last metadata used              |
| 0x28   | 8         | Last CAT used                   |
| 0x30   | 8         | Free sectors                    |

### Boot area: 8.191 sectors (4 MB)

Contains a boot or kernel that will be loaded in the system initialization.

### Hash area: 131.072 sectors (64 MB)

Hash area contains the indexed hashes for the keys. The hashes are 24-bit integer values, and each hash points to a 32-bit index in the metadata area.
Thus, this area occupies 64 MB.

Example: let's say we have a key called `"world"`. The hash of this key is `0xE3070A`.
The 32-bit array location `0xE3070A` in the hash area is `0x36129C0` - thus, this is the location of the
metadata in the Metadata area.

### Metadata area: user-defined size

The metadata area contains the metadata for each one of the keys. Each metadata record has a size that is a multiple
of 32 bytes. Thus, if the hash has a pointer of `0x36129C0`, the actual location of this index is `0x36129C0 * 32d = 0x6C253800`.

The first byte of the metadata record can be:

| Value   | Description           |
|---------|-----------------------|
| `0x00`  | unused                |
| `0x01`  | metadata record       |
| `0x02`  | disambiguation record |
| `0x03`  | deleted               |

If the record is a **metadata record**, the rest of the record contains the following values:

| Offset | Data size | Description                  |
|--------|-----------|------------------------------|
 | 0x1    | 1         | Reserved                     |
| 0x2    | 8         | Sector index in content area |
| 0xA    | 8         | Value size                   |
| 0x12   | 14`*`     | Key name (ends with `NULL`)  |
`*` Can be larger if the key occupies multiple records

If the record is a **disambiguation record**, this means that the same hash points to more than one key.
In this case, starting on position `0x4`, there's a list of other 32-bit metadata record indexes that can be the right key.
In this case, it is necessary to compare the key string.

If the record is a **deleted record**, the record is deleted and can be reused.

### Cluster allocation table: size dependent on content area

The CAT contains an array of 64-bit values, which contains linked-lists of indexes to the content area.
This shows where a value continues after the end of each section.

Special values are: `0x0` for a free sector, and `-1` for the end-of-value.

This works the same as the FAT in FAT filesystems.

### Content area: rest of the volume

This is the area where the values are stored, divided in sectors. To know where the value continues at the
end of one sector, the CAT needs to be consulted.
