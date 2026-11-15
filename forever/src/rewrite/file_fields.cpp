// SceneryItem::FileFields, AttractionInstance::FileFields, CIDDatabaseEntry::FileFields,
// CarriedItemInstance::FileFields and PPGDatabaseEntry::FileFields (one function written out five times;
// tools/family.py: file_fields.inc): make a saved link to a database entry and give it its field. Gives the link.
//   - The link: 0x20 new bytes built by 0x808700, its +0x18 and +0x1C set to the copy's values (a name and the
//     function that reads it); none without memory.
//   - The field: 0x2C new bytes built by 0x8088C0 with the copy's name, kind and two sizes, then given the copy's
//     table; none without memory.
//   - The field handed to the link (0x8087E0), even when either is none.
// (The game's stack cookie check guards only its own frame.)
#include "rewrite.h"

namespace {
struct FileFields {
    uint32_t at, link_name, reader, second_size, first_size, kind, name, table;
};
#define FILE_FIELDS(at, link_name, reader, second_size, first_size, kind, name, table, ...) {at, link_name, reader, second_size, first_size, kind, name, table},
constexpr FileFields file_fields[] = {
#include "file_fields.inc"
};
#undef FILE_FIELDS
#define FILE_FIELDS(at, link_name, reader, second_size, first_size, kind, name, table, ...) {__VA_ARGS__},
const uint8_t file_fields_first[][7] = {
#include "file_fields.inc"
};
#undef FILE_FIELDS
typedef void *(__cdecl *Alloc)(uint32_t);
const Alloc alloc = (Alloc)0xbcdfc0;

volatile long g_file_fields;
uint8_t *file_field(const FileFields &kind) {
    g_file_fields++;
    uint8_t *link = (uint8_t *)alloc(0x20);
    if (link) {
        ((void(__thiscall *)(void *))0x808700)(link);
        *(volatile uint32_t *)(link + 0x18) = kind.link_name;
        *(volatile uint32_t *)(link + 0x1c) = kind.reader;
    }
    uint8_t *field = (uint8_t *)alloc(0x2c);
    if (field) {
        ((void(__thiscall *)(void *, uint32_t, uint32_t, uint32_t, uint32_t))0x8088c0)(field, kind.name, kind.kind, kind.first_size, kind.second_size);
        *(volatile uint32_t *)field = kind.table;
    }
    ((void(__thiscall *)(void *, void *))0x8087e0)(link, field);
    return link;
}

template <int K> uint8_t *__cdecl file_field_copy() { return file_field(file_fields[K]); }
}   // namespace
// (the addresses again, as numbers, checked against the table)
static_assert(file_fields[0].at == 0x7fd800 && file_fields[1].at == 0x7fe640 && file_fields[2].at == 0x7fe7d0 && file_fields[3].at == 0x7fe9a0 &&
                  file_fields[4].at == 0x7ffd20,
              "file_fields.inc changed");
REWRITE("Database::FileFields", 0x7fd800, file_fields_first[0], (&file_field_copy<0>), &g_file_fields);
REWRITE("Database::FileFields", 0x7fe640, file_fields_first[1], (&file_field_copy<1>), &g_file_fields);
REWRITE("Database::FileFields", 0x7fe7d0, file_fields_first[2], (&file_field_copy<2>), &g_file_fields);
REWRITE("Database::FileFields", 0x7fe9a0, file_fields_first[3], (&file_field_copy<3>), &g_file_fields);
REWRITE("Database::FileFields", 0x7ffd20, file_fields_first[4], (&file_field_copy<4>), &g_file_fields);
