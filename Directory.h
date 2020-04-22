

typedef struct dirEntry
{
    uint64_t id;
    char name[128];
    uint64_t date;
    uint64_t location;
    uint64_t sizeInBytes;
    uint64_t flags;
} dirEntry, *dirEntry_p;