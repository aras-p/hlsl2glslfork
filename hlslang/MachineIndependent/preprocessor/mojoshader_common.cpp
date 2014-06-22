#define __MOJOSHADER_INTERNAL__ 1
#include "mojoshader_internal.h"

// Stripped down code from Mojoshader, originally under zlib license.


typedef struct HashItem
{
    const void *key;
    const void *value;
    struct HashItem *next;
} HashItem;

struct HashTable
{
    HashItem **table;
    uint32 table_len;
    int stackable;
    void *data;
    HashTable_HashFn hash;
    HashTable_KeyMatchFn keymatch;
    HashTable_NukeFn nuke;
    MOJOSHADER_hlslang_malloc m;
    MOJOSHADER_hlslang_free f;
    void *d;
};

static inline uint32 calc_hash(const HashTable *table, const void *key)
{
    return table->hash(key, table->data) & (table->table_len-1);
} // calc_hash

int hash_find(const HashTable *table, const void *key, const void **_value)
{
    HashItem *i;
    void *data = table->data;
    const uint32 hash = calc_hash(table, key);
    HashItem *prev = NULL;
    for (i = table->table[hash]; i != NULL; i = i->next)
    {
        if (table->keymatch(key, i->key, data))
        {
            if (_value != NULL)
                *_value = i->value;

            // Matched! Move to the front of list for faster lookup next time.
            //  (stackable tables have to remain in the same order, though!)
            if ((!table->stackable) && (prev != NULL))
            {
                assert(prev->next == i);
                prev->next = i->next;
                i->next = table->table[hash];
                table->table[hash] = i;
            } // if

            return 1;
        } // if

        prev = i;
    } // for

    return 0;
} // hash_find


int hash_insert(HashTable *table, const void *key, const void *value)
{
    HashItem *item = NULL;
    const uint32 hash = calc_hash(table, key);
    if ( (!table->stackable) && (hash_find(table, key, NULL)) )
        return 0;

    // !!! FIXME: grow and rehash table if it gets too saturated.
    item = (HashItem *) table->m(sizeof (HashItem), table->d);
    if (item == NULL)
        return -1;

    item->key = key;
    item->value = value;
    item->next = table->table[hash];
    table->table[hash] = item;

    return 1;
} // hash_insert

HashTable *hash_create(void *data, const HashTable_HashFn hashfn,
              const HashTable_KeyMatchFn keymatchfn,
              const HashTable_NukeFn nukefn,
              const int stackable,
              MOJOSHADER_hlslang_malloc m, MOJOSHADER_hlslang_free f, void *d)
{
    const uint32 initial_table_size = 256;
    const uint32 alloc_len = sizeof (HashItem *) * initial_table_size;
    HashTable *table = (HashTable *) m(sizeof (HashTable), d);
    if (table == NULL)
        return NULL;
    memset(table, '\0', sizeof (HashTable));

    table->table = (HashItem **) m(alloc_len, d);
    if (table->table == NULL)
    {
        f(table, d);
        return NULL;
    } // if

    memset(table->table, '\0', alloc_len);
    table->table_len = initial_table_size;
    table->stackable = stackable;
    table->data = data;
    table->hash = hashfn;
    table->keymatch = keymatchfn;
    table->nuke = nukefn;
    table->m = m;
    table->f = f;
    table->d = d;
    return table;
} // hash_create

void hash_destroy(HashTable *table)
{
    uint32 i;
    void *data = table->data;
    MOJOSHADER_hlslang_free f = table->f;
    void *d = table->d;
    for (i = 0; i < table->table_len; i++)
    {
        HashItem *item = table->table[i];
        while (item != NULL)
        {
            HashItem *next = item->next;
            table->nuke(item->key, item->value, data);
            f(item, d);
            item = next;
        } // while
    } // for

    f(table->table, d);
    f(table, d);
} // hash_destroy

int hash_remove(HashTable *table, const void *key)
{
    HashItem *item = NULL;
    HashItem *prev = NULL;
    void *data = table->data;
    const uint32 hash = calc_hash(table, key);
    for (item = table->table[hash]; item != NULL; item = item->next)
    {
        if (table->keymatch(key, item->key, data))
        {
            if (prev != NULL)
                prev->next = item->next;
            else
                table->table[hash] = item->next;

            table->nuke(item->key, item->value, data);
            table->f(item, table->d);
            return 1;
        } // if

        prev = item;
    } // for

    return 0;
} // hash_remove


// this is djb's xor hashing function.
static inline uint32 hash_string_djbxor(const char *str, size_t len)
{
    register uint32 hash = 5381;
    while (len--)
        hash = ((hash << 5) + hash) ^ *(str++);
    return hash;
} // hash_string_djbxor

static inline uint32 hash_string(const char *str, size_t len)
{
    return hash_string_djbxor(str, len);
} // hash_string



// The string cache...   !!! FIXME: use StringMap internally for this.

typedef struct StringBucket
{
    char *string;
    struct StringBucket *next;
} StringBucket;

struct StringCache
{
    StringBucket **hashtable;
    uint32 table_size;
    MOJOSHADER_hlslang_malloc m;
    MOJOSHADER_hlslang_free f;
    void *d;
};


const char *stringcache(StringCache *cache, const char *str)
{
    return stringcache_len(cache, str, strlen(str));
} // stringcache

static const char *stringcache_len_internal(StringCache *cache,
                                            const char *str,
                                            const unsigned int len,
                                            const int addmissing)
{
    const uint8 hash = hash_string(str, len) & (cache->table_size-1);
    StringBucket *bucket = cache->hashtable[hash];
    StringBucket *prev = NULL;
    while (bucket)
    {
        const char *bstr = bucket->string;
        if ((strncmp(bstr, str, len) == 0) && (bstr[len] == 0))
        {
            // Matched! Move this to the front of the list.
            if (prev != NULL)
            {
                assert(prev->next == bucket);
                prev->next = bucket->next;
                bucket->next = cache->hashtable[hash];
                cache->hashtable[hash] = bucket;
            } // if
            return bstr; // already cached
        } // if
        prev = bucket;
        bucket = bucket->next;
    } // while

    // no match!
    if (!addmissing)
        return NULL;

    // add to the table.
    bucket = (StringBucket *) cache->m(sizeof (StringBucket), cache->d);
    if (bucket == NULL)
        return NULL;
    bucket->string = (char *) cache->m(len + 1, cache->d);
    if (bucket->string == NULL)
    {
        cache->f(bucket, cache->d);
        return NULL;
    } // if
    memcpy(bucket->string, str, len);
    bucket->string[len] = '\0';
    bucket->next = cache->hashtable[hash];
    cache->hashtable[hash] = bucket;
    return bucket->string;
} // stringcache_len_internal

const char *stringcache_len(StringCache *cache, const char *str,
                            const unsigned int len)
{
    return stringcache_len_internal(cache, str, len, 1);
} // stringcache_len


StringCache *stringcache_create(MOJOSHADER_hlslang_malloc m, MOJOSHADER_hlslang_free f, void *d)
{
    const uint32 initial_table_size = 256;
    const size_t tablelen = sizeof (StringBucket *) * initial_table_size;
    StringCache *cache = (StringCache *) m(sizeof (StringCache), d);
    if (!cache)
        return NULL;
    memset(cache, '\0', sizeof (StringCache));

    cache->hashtable = (StringBucket **) m(tablelen, d);
    if (!cache->hashtable)
    {
        f(cache, d);
        return NULL;
    } // if
    memset(cache->hashtable, '\0', tablelen);

    cache->table_size = initial_table_size;
    cache->m = m;
    cache->f = f;
    cache->d = d;
    return cache;
} // stringcache_create

void stringcache_destroy(StringCache *cache)
{
    if (cache == NULL)
        return;

    MOJOSHADER_hlslang_free f = cache->f;
    void *d = cache->d;
    size_t i;

    for (i = 0; i < cache->table_size; i++)
    {
        StringBucket *bucket = cache->hashtable[i];
        cache->hashtable[i] = NULL;
        while (bucket)
        {
            StringBucket *next = bucket->next;
            f(bucket->string, d);
            f(bucket, d);
            bucket = next;
        } // while
    } // for

    f(cache->hashtable, d);
    f(cache, d);
} // stringcache_destroy


// We chain errors as a linked list with a head/tail for easy appending.
//  These get flattened before passing to the application.
typedef struct ErrorItem
{
    MOJOSHADER_hlslang_error error;
    struct ErrorItem *next;
} ErrorItem;

struct ErrorList
{
    ErrorItem head;
    ErrorItem *tail;
    int count;
    MOJOSHADER_hlslang_malloc m;
    MOJOSHADER_hlslang_free f;
    void *d;
};

ErrorList *errorlist_create(MOJOSHADER_hlslang_malloc m, MOJOSHADER_hlslang_free f, void *d)
{
    ErrorList *retval = (ErrorList *) m(sizeof (ErrorList), d);
    if (retval != NULL)
    {
        memset(retval, '\0', sizeof (ErrorList));
        retval->tail = &retval->head;
        retval->m = m;
        retval->f = f;
        retval->d = d;
    } // if

    return retval;
} // errorlist_create


int errorlist_add(ErrorList *list, const char *fname,
                  const int errpos, const char *str)
{
    return errorlist_add_fmt(list, fname, errpos, "%s", str);
} // errorlist_add


int errorlist_add_fmt(ErrorList *list, const char *fname,
                      const int errpos, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    const int retval = errorlist_add_va(list, fname, errpos, fmt, ap);
    va_end(ap);
    return retval;
} // errorlist_add_fmt


int errorlist_add_va(ErrorList *list, const char *_fname,
                     const int errpos, const char *fmt, va_list va)
{
    ErrorItem *error = (ErrorItem *) list->m(sizeof (ErrorItem), list->d);
    if (error == NULL)
        return 0;

    char *fname = NULL;
    if (_fname != NULL)
    {
        fname = (char *) list->m(strlen(_fname) + 1, list->d);
        if (fname == NULL)
        {
            list->f(error, list->d);
            return 0;
        } // if
        strcpy(fname, _fname);
    } // if

    char scratch[128];
    va_list ap;
    va_copy(ap, va);
    const int len = vsnprintf(scratch, sizeof (scratch), fmt, ap);
    va_end(ap);

    char *failstr = (char *) list->m(len + 1, list->d);
    if (failstr == NULL)
    {
        list->f(error, list->d);
        list->f(fname, list->d);
        return 0;
    } // if

    // If we overflowed our scratch buffer, that's okay. We were going to
    //  allocate anyhow...the scratch buffer just lets us avoid a second
    //  run of vsnprintf().
    if (len < sizeof (scratch))
        strcpy(failstr, scratch);  // copy it over.
    else
    {
        va_copy(ap, va);
        vsnprintf(failstr, len + 1, fmt, ap);  // rebuild it.
        va_end(ap);
    } // else

    error->error.error = failstr;
    error->error.filename = fname;
    error->error.error_position = errpos;
    error->next = NULL;

    list->tail->next = error;
    list->tail = error;

    list->count++;
    return 1;
} // errorlist_add_va


int errorlist_count(ErrorList *list)
{
    return list->count;
} // errorlist_count


MOJOSHADER_hlslang_error *errorlist_flatten(ErrorList *list)
{
    if (list->count == 0)
        return NULL;

    int total = 0;
    MOJOSHADER_hlslang_error *retval = (MOJOSHADER_hlslang_error *)
            list->m(sizeof (MOJOSHADER_hlslang_error) * list->count, list->d);
    if (retval == NULL)
        return NULL;

    ErrorItem *item = list->head.next;
    while (item != NULL)
    {
        ErrorItem *next = item->next;
        // reuse the string allocations
        memcpy(&retval[total], &item->error, sizeof (MOJOSHADER_hlslang_error));
        list->f(item, list->d);
        item = next;
        total++;
    } // while

    assert(total == list->count);
    list->count = 0;
    list->head.next = NULL;
    list->tail = &list->head;
    return retval;
} // errorlist_flatten


void errorlist_destroy(ErrorList *list)
{
    if (list == NULL)
        return;

    MOJOSHADER_hlslang_free f = list->f;
    void *d = list->d;
    ErrorItem *item = list->head.next;
    while (item != NULL)
    {
        ErrorItem *next = item->next;
        f((void *) item->error.error, d);
        f((void *) item->error.filename, d);
        f(item, d);
        item = next;
    } // while
    f(list, d);
} // errorlist_destroy


typedef struct BufferBlock
{
    uint8 *data;
    size_t bytes;
    struct BufferBlock *next;
} BufferBlock;

struct Buffer
{
    size_t total_bytes;
    BufferBlock *head;
    BufferBlock *tail;
    size_t block_size;
    MOJOSHADER_hlslang_malloc m;
    MOJOSHADER_hlslang_free f;
    void *d;
};

Buffer *buffer_create(size_t blksz, MOJOSHADER_hlslang_malloc m,
                      MOJOSHADER_hlslang_free f, void *d)
{
    Buffer *buffer = (Buffer *) m(sizeof (Buffer), d);
    if (buffer != NULL)
    {
        memset(buffer, '\0', sizeof (Buffer));
        buffer->block_size = blksz;
        buffer->m = m;
        buffer->f = f;
        buffer->d = d;
    } // if
    return buffer;
} // buffer_create


int buffer_append(Buffer *buffer, const void *_data, size_t len)
{
    const uint8 *data = (const uint8 *) _data;

    // note that we make the blocks bigger than blocksize when we have enough
    //  data to overfill a fresh block, to reduce allocations.
    const size_t blocksize = buffer->block_size;

    if (len == 0)
        return 1;

    if (buffer->tail != NULL)
    {
        const size_t tailbytes = buffer->tail->bytes;
        const size_t avail = (tailbytes >= blocksize) ? 0 : blocksize - tailbytes;
        const size_t cpy = (avail > len) ? len : avail;
        if (cpy > 0)
        {
            memcpy(buffer->tail->data + tailbytes, data, cpy);
            len -= cpy;
            data += cpy;
            buffer->tail->bytes += cpy;
            buffer->total_bytes += cpy;
            assert(buffer->tail->bytes <= blocksize);
        } // if
    } // if

    if (len > 0)
    {
        assert((!buffer->tail) || (buffer->tail->bytes >= blocksize));
        const size_t bytecount = len > blocksize ? len : blocksize;
        const size_t malloc_len = sizeof (BufferBlock) + bytecount;
        BufferBlock *item = (BufferBlock *) buffer->m(malloc_len, buffer->d);
        if (item == NULL)
            return 0;

        item->data = ((uint8 *) item) + sizeof (BufferBlock);
        item->bytes = len;
        item->next = NULL;
        if (buffer->tail != NULL)
            buffer->tail->next = item;
        else
            buffer->head = item;
        buffer->tail = item;

        memcpy(item->data, data, len);
        buffer->total_bytes += len;
    } // if

    return 1;
} // buffer_append

int buffer_append_fmt(Buffer *buffer, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    const int retval = buffer_append_va(buffer, fmt, ap);
    va_end(ap);
    return retval;
} // buffer_append_fmt

int buffer_append_va(Buffer *buffer, const char *fmt, va_list va)
{
    char scratch[256];

    va_list ap;
    va_copy(ap, va);
    const int len = vsnprintf(scratch, sizeof (scratch), fmt, ap);
    va_end(ap);

    // If we overflowed our scratch buffer, heap allocate and try again.

    if (len == 0)
        return 1;  // nothing to do.
    else if (len < sizeof (scratch))
        return buffer_append(buffer, scratch, len);

    char *buf = (char *) buffer->m(len + 1, buffer->d);
    if (buf == NULL)
        return 0;
    va_copy(ap, va);
    vsnprintf(buf, len + 1, fmt, ap);  // rebuild it.
    va_end(ap);
    const int retval = buffer_append(buffer, scratch, len);
    buffer->f(buf, buffer->d);
    return retval;
} // buffer_append_va

size_t buffer_size(Buffer *buffer)
{
    return buffer->total_bytes;
} // buffer_size

void buffer_empty(Buffer *buffer)
{
    BufferBlock *item = buffer->head;
    while (item != NULL)
    {
        BufferBlock *next = item->next;
        buffer->f(item, buffer->d);
        item = next;
    } // while
    buffer->head = buffer->tail = NULL;
    buffer->total_bytes = 0;
} // buffer_empty

char *buffer_flatten(Buffer *buffer)
{
    char *retval = (char *) buffer->m(buffer->total_bytes + 1, buffer->d);
    if (retval == NULL)
        return NULL;
    BufferBlock *item = buffer->head;
    char *ptr = retval;
    while (item != NULL)
    {
        BufferBlock *next = item->next;
        memcpy(ptr, item->data, item->bytes);
        ptr += item->bytes;
        buffer->f(item, buffer->d);
        item = next;
    } // while
    *ptr = '\0';

    assert(ptr == (retval + buffer->total_bytes));

    buffer->head = buffer->tail = NULL;
    buffer->total_bytes = 0;

    return retval;
} // buffer_flatten

void buffer_destroy(Buffer *buffer)
{
    if (buffer != NULL)
    {
        MOJOSHADER_hlslang_free f = buffer->f;
        void *d = buffer->d;
        buffer_empty(buffer);
        f(buffer, d);
    } // if
} // buffer_destroy


// end of mojoshader_common.c ...

