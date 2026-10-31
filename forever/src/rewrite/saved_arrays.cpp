// Saving arrays of structs. Each kind of saved object describes its fields (see ReferenceField::Load); a field that
// is an array of structs {data, count} has its own list of the fields every struct in it has. Saved, it is a
// length (of what follows), the count, then each struct's fields in turn. The compiler made a copy of these for
// every struct type; the struct's size is all that differs, so one template covers them.
#include "rewrite.h"

struct Field {
    void **vtable;   // [2] Size(item): bytes it saves as; [3] Save(item, writer)
};

struct StructArrayField {
    void **vtable;          // [2] is Size below
    char unknown[8];
    Field **fields;         // +0xc: the fields of each struct
    char unknown2[8];
    int32_t field_count;    // +0x18
    char unknown3[0xc];
    int32_t offset;         // +0x28: where the array sits in the object
};
static_assert(__builtin_offsetof(StructArrayField, offset) == 0x28, "layout used by RCT3plus.exe");

struct Array {
    uint8_t *data;
    int32_t count;
};

// Writes to the save file keep a running checksum over every byte written.
struct SaveWriter {
    void *file;
    char unknown[0xc];
    uint32_t sum;     // +0x10
    uint16_t check;   // +0x14
};

typedef void(__fastcall *WriterWrite)(SaveWriter *, void *, const void *data, int32_t size);
static const WriterWrite writer_write = (WriterWrite)0xbe4060;

static void write_checked(SaveWriter *w, const void *data, int32_t size) {
    writer_write(w, nullptr, data, size);
    for (int32_t i = 0; i < size; i++) {
        uint8_t x = (uint8_t)(w->check >> 8) ^ ((const uint8_t *)data)[i];
        w->check = (uint16_t)(0x58bf - (uint16_t)(x + w->check) * 0x3193);
        w->sum += x;
    }
}

static int32_t field_size(Field *f, uint8_t *item) {
    return ((int32_t(__fastcall *)(Field *, void *, uint8_t *))f->vtable[2])(f, nullptr, item);
}

template <uint32_t SIZE> struct Calls { static volatile long save, size; };
template <uint32_t SIZE> volatile long Calls<SIZE>::save;
template <uint32_t SIZE> volatile long Calls<SIZE>::size;

// Size(object): how many bytes the array saves as, the length and count included.
template <uint32_t SIZE> static int32_t __fastcall size(StructArrayField *f, void *, uint8_t *object) {
    Calls<SIZE>::size++;
    Array *a = (Array *)(object + f->offset);
    int32_t total = 8;
    for (int32_t i = 0, n = a->count; i < n; i++) {
        uint8_t *item = a->data + i * SIZE;
        for (int32_t j = 0; j < f->field_count; j++) total += field_size(f->fields[j], item);
    }
    return total;
}

// Save(object, writer)
template <uint32_t SIZE> static void __fastcall save(StructArrayField *f, void *, uint8_t *object, SaveWriter *w) {
    Calls<SIZE>::save++;
    Array *a = (Array *)(object + f->offset);
    int32_t count = a->count;
    typedef int32_t(__fastcall * OwnSize)(StructArrayField *, void *, uint8_t *);
    int32_t length = ((OwnSize)f->vtable[2])(f, nullptr, object) - 4;   // what follows the length
    write_checked(w, &length, 4);
    write_checked(w, &count, 4);
    for (int32_t i = 0; i < count; i++) {
        uint8_t *item = a->data + i * SIZE;
        for (int32_t j = 0; j < f->field_count; j++) {
            Field *x = f->fields[j];
            ((void(__fastcall *)(Field *, void *, uint8_t *, SaveWriter *))x->vtable[3])(x, nullptr, item, w);
        }
    }
}

static const uint8_t save_original[] = {0x83, 0xec, 0x0c, 0x8b, 0x44, 0x24, 0x10, 0x53,
                                        0x55, 0x8b, 0xe9, 0x8b, 0x55, 0x00, 0x56, 0x57};
static const uint8_t size_original[] = {0x83, 0xec, 0x08, 0x8b, 0x54, 0x24, 0x0c, 0x53,
                                        0x8b, 0xd9, 0x8b, 0x4b, 0x28, 0x03, 0xca, 0x8b};

static const uint32_t save_3[] = {
    0x63fa30,
};
REWRITE_COPIES("StructArrayField<3>::Save", save_3, save_original, save<3>,
               &Calls<3>::save);
static const uint32_t size_3[] = {
    0x63fb60,
};
REWRITE_COPIES("StructArrayField<3>::Size", size_3, size_original, size<3>,
               &Calls<3>::size);

static const uint32_t save_12[] = {
    0x4c17d0, 0x512f50, 0x81ff60, 0xa31dc0,
};
REWRITE_COPIES("StructArrayField<12>::Save", save_12, save_original, save<12>,
               &Calls<12>::save);
static const uint32_t size_12[] = {
    0x4c1900, 0x513080, 0x820090, 0xa31ef0,
};
REWRITE_COPIES("StructArrayField<12>::Size", size_12, size_original, size<12>,
               &Calls<12>::size);

static const uint32_t save_16[] = {
    0x54cd80,
};
REWRITE_COPIES("StructArrayField<16>::Save", save_16, save_original, save<16>,
               &Calls<16>::save);
static const uint32_t size_16[] = {
    0x54ceb0,
};
REWRITE_COPIES("StructArrayField<16>::Size", size_16, size_original, size<16>,
               &Calls<16>::size);

static const uint32_t save_20[] = {
    0x512d40, 0x9d2a80, 0xb84030,
};
REWRITE_COPIES("StructArrayField<20>::Save", save_20, save_original, save<20>,
               &Calls<20>::save);
static const uint32_t size_20[] = {
    0x512e70, 0x54cca0, 0x9d2bb0, 0xb84160,
};
REWRITE_COPIES("StructArrayField<20>::Size", size_20, size_original, size<20>,
               &Calls<20>::size);

static const uint32_t save_24[] = {
    0x455970, 0x6be250, 0x6e7370, 0x7a3100, 0xa8dc90, 0xab3e10, 0xb9fd20,
};
REWRITE_COPIES("StructArrayField<24>::Save", save_24, save_original, save<24>,
               &Calls<24>::save);
static const uint32_t size_24[] = {
    0x455aa0, 0x6be380, 0x6e74a0, 0x7a3230, 0xa8ddc0, 0xab3f40, 0xb9fe50,
};
REWRITE_COPIES("StructArrayField<24>::Size", size_24, size_original, size<24>,
               &Calls<24>::size);

static const uint32_t save_28[] = {
    0x460960, 0x48f9f0, 0x4a5680, 0x512920, 0x5647a0, 0x5aace0, 0x5aaef0, 0x5d6f80,
    0x63fc40, 0x73e980, 0x7ccc70, 0x81fd50, 0x8b1b10, 0x91d980, 0xa8f1c0, 0xa97cd0,
    0xae2eb0, 0xb45780, 0xb89b10, 0xb89d20, 0xb89f30, 0xbaf810,
};
REWRITE_COPIES("StructArrayField<28>::Save", save_28, save_original, save<28>,
               &Calls<28>::save);
static const uint32_t size_28[] = {
    0x460a90, 0x48fb20, 0x4a57b0, 0x512a50, 0x5648d0, 0x5aae10, 0x5ab020, 0x5d70b0,
    0x5f1dd0, 0x63fd70, 0x73eab0, 0x7ccda0, 0x81fe80, 0x8b1c40, 0x91dab0, 0xa8f2f0,
    0xa97e00, 0xae2fe0, 0xb458b0, 0xb89c40, 0xb89e50, 0xb8a060, 0xbaf940,
};
REWRITE_COPIES("StructArrayField<28>::Size", size_28, size_original, size<28>,
               &Calls<28>::size);

static const uint32_t save_32[] = {
    0x44ec90, 0x762560, 0x98c9d0,
};
REWRITE_COPIES("StructArrayField<32>::Save", save_32, save_original, save<32>,
               &Calls<32>::save);
static const uint32_t size_32[] = {
    0x44edc0, 0x762690, 0x98cb00,
};
REWRITE_COPIES("StructArrayField<32>::Size", size_32, size_original, size<32>,
               &Calls<32>::size);

static const uint32_t save_36[] = {
    0x425260, 0xac65b0,
};
REWRITE_COPIES("StructArrayField<36>::Save", save_36, save_original, save<36>,
               &Calls<36>::save);
static const uint32_t size_36[] = {
    0x425390, 0xac66e0,
};
REWRITE_COPIES("StructArrayField<36>::Size", size_36, size_original, size<36>,
               &Calls<36>::size);

static const uint32_t save_40[] = {
    0x512b30, 0x5aaad0, 0x820170, 0x82e450, 0x98cf60, 0x9d7550,
};
REWRITE_COPIES("StructArrayField<40>::Save", save_40, save_original, save<40>,
               &Calls<40>::save);
static const uint32_t size_40[] = {
    0x512c60, 0x5aac00, 0x8202a0, 0x82e580, 0x98d090, 0x9d7680,
};
REWRITE_COPIES("StructArrayField<40>::Size", size_40, size_original, size<40>,
               &Calls<40>::size);

static const uint32_t save_44[] = {
    0x815ac0, 0x91db90,
};
REWRITE_COPIES("StructArrayField<44>::Save", save_44, save_original, save<44>,
               &Calls<44>::save);
static const uint32_t size_44[] = {
    0x815bf0, 0x91dcc0,
};
REWRITE_COPIES("StructArrayField<44>::Size", size_44, size_original, size<44>,
               &Calls<44>::size);

static const uint32_t save_52[] = {
    0x4c19e0, 0x9d7760,
};
REWRITE_COPIES("StructArrayField<52>::Save", save_52, save_original, save<52>,
               &Calls<52>::save);
static const uint32_t size_52[] = {
    0x4c1b10, 0x9d7890,
};
REWRITE_COPIES("StructArrayField<52>::Size", size_52, size_original, size<52>,
               &Calls<52>::size);

static const uint32_t size_56[] = {
    0xa02020,
};
REWRITE_COPIES("StructArrayField<56>::Size", size_56, size_original, size<56>,
               &Calls<56>::size);

static const uint32_t save_80[] = {
    0x46da20,
};
REWRITE_COPIES("StructArrayField<80>::Save", save_80, save_original, save<80>,
               &Calls<80>::save);
static const uint32_t size_80[] = {
    0x46db50,
};
REWRITE_COPIES("StructArrayField<80>::Size", size_80, size_original, size<80>,
               &Calls<80>::size);

static const uint32_t save_88[] = {
    0x98cd50, 0xabeb40,
};
REWRITE_COPIES("StructArrayField<88>::Save", save_88, save_original, save<88>,
               &Calls<88>::save);
static const uint32_t size_88[] = {
    0x98ce80, 0xabec70,
};
REWRITE_COPIES("StructArrayField<88>::Size", size_88, size_original, size<88>,
               &Calls<88>::size);

static const uint32_t save_96[] = {
    0x81d860,
};
REWRITE_COPIES("StructArrayField<96>::Save", save_96, save_original, save<96>,
               &Calls<96>::save);
static const uint32_t size_96[] = {
    0x81d990,
};
REWRITE_COPIES("StructArrayField<96>::Size", size_96, size_original, size<96>,
               &Calls<96>::size);

static const uint32_t save_112[] = {
    0xb248b0,
};
REWRITE_COPIES("StructArrayField<112>::Save", save_112, save_original, save<112>,
               &Calls<112>::save);
static const uint32_t size_112[] = {
    0xb249e0,
};
REWRITE_COPIES("StructArrayField<112>::Size", size_112, size_original, size<112>,
               &Calls<112>::size);

static const uint32_t save_116[] = {
    0x6be040,
};
REWRITE_COPIES("StructArrayField<116>::Save", save_116, save_original, save<116>,
               &Calls<116>::save);
static const uint32_t size_116[] = {
    0x6be170,
};
REWRITE_COPIES("StructArrayField<116>::Size", size_116, size_original, size<116>,
               &Calls<116>::size);

// The small sizes (4 and 8 bytes) were compiled another way (the item found as data + i * size, the first bytes
// differ); they do the same.
static const uint8_t save_scaled_original[] = {0x83, 0xec, 0x08, 0x8b, 0x44, 0x24, 0x0c, 0x53,
                                               0x55, 0x8b, 0xd9, 0x8b, 0x6b, 0x28, 0x8b, 0x4c};
static const uint32_t save_4[] = {0x460540, 0x4ad8a0, 0x4adab0, 0x9850a0, 0xa5d150};
REWRITE_COPIES("StructArrayField<4>::Save", save_4, save_scaled_original, save<4>, &Calls<4>::save);
static const uint32_t save_8[] = {0x460750, 0xa536b0, 0xa5cf40, 0xae2ca0};
REWRITE_COPIES("StructArrayField<8>::Save", save_8, save_scaled_original, save<8>, &Calls<8>::save);

// ListField::Save(object, writer): a list field (the list's first node at the field's offset; nodes {next, ?, item},
// the last one's next null). Saves the length (its own size, slot 2, less 4), the count of nodes before the last,
// keeping the running checksum, then has each field save each of those nodes' items.
static volatile long g_list_saves;
static void __fastcall list_save(StructArrayField *f, void *, uint8_t *object, SaveWriter *w) {
    g_list_saves++;
    uint32_t **list = (uint32_t **)(object + f->offset);
    uint32_t *node = *list;
    uint32_t *next = (uint32_t *)(uintptr_t)node[0];
    int32_t count = 0;
    while (next && node) {
        node = (uint32_t *)(uintptr_t)node[0];
        next = (uint32_t *)(uintptr_t)node[0];
        count++;
    }
    typedef int32_t(__fastcall * OwnSize)(StructArrayField *, void *, uint8_t *);
    int32_t length = ((OwnSize)f->vtable[2])(f, nullptr, object) - 4;
    write_checked(w, &length, 4);
    write_checked(w, &count, 4);
    for (node = *list; node[0] && node; node = (uint32_t *)(uintptr_t)node[0])
        for (int32_t j = 0; j < f->field_count; j++) {
            Field *x = f->fields[j];
            ((void(__fastcall *)(Field *, void *, uint8_t *, SaveWriter *))x->vtable[3])(
                x, nullptr, (uint8_t *)(uintptr_t)node[2], w);
        }
}
static const uint8_t list_save_original[] = {0x51, 0x8b, 0x54, 0x24, 0x08, 0x53, 0x55, 0x56,
                                             0x8b, 0xe9, 0x57, 0x8b, 0x7d, 0x28, 0x8b, 0x04};
static const uint32_t list_save_copies[] = {0x533250, 0x54cfa0, 0x54d240, 0x54d450, 0x54d750,
                                            0x593980, 0x593b90, 0x5c26f0, 0xb3a780};
REWRITE_COPIES("ListField::Save", list_save_copies, list_save_original, list_save, &g_list_saves);

// EngineEntryField::Size(offset): how many bytes an array field saves as, where the array {data, count} sits at the
// field descriptor's +0x28 pointer plus the given offset. The length and count (8 bytes) plus, for each 4-byte entry,
// every one of the field's fields' Size (slot 2) of its address. Like StructArrayField<4>::Size with the array found
// through a pointer and an offset.
static volatile long g_entry_sizes;
static int32_t __fastcall entry_size(uint8_t *self, void *, int32_t offset) {
    g_entry_sizes++;
    uint8_t *array = *(uint8_t **)(self + 0x28) + offset;
    int32_t count = *(int32_t *)(array + 4);
    uint8_t *data = *(uint8_t **)array;
    int32_t field_count = *(int32_t *)(self + 0x18);
    Field **fields = *(Field ***)(self + 0xc);
    int32_t total = 8;
    for (int32_t i = 0; i < count; i++)
        for (int32_t j = 0; j < field_count; j++) total += field_size(fields[j], data + i * 4);
    return total;
}
static const uint8_t entry_size_original[] = {0x83, 0xec, 0x08, 0x53, 0x8b, 0x5c, 0x24, 0x10};
static const uint32_t entry_size_copies[] = {0x460660, 0x4ad9c0, 0x4adbd0, 0x9851c0, 0xa5ce50, 0xa5d270};
REWRITE_COPIES("EngineEntryField::Size", entry_size_copies, entry_size_original, entry_size, &g_entry_sizes);
