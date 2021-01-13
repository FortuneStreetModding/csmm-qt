#ifndef ARCHIVEUTIL_H
#define ARCHIVEUTIL_H

#include <archive.h>
#include <archive_entry.h>
#include <QString>

inline int copyData(archive *input, archive *output) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(input, &buff, &size, &offset);
        if (r == ARCHIVE_EOF) {
            return ARCHIVE_OK;
        }
        if (r != ARCHIVE_OK) {
            return r;
        }
        r = archive_write_data_block(output, buff, size, offset);
        if (r != ARCHIVE_OK) {
            return r;
        }
    }
}

// InputToOutputFile maps the input file string to the output file string, or empty if it
// should not extract that file
template<typename InputToOutputFile>
int extractFileTo(archive *input, archive *output, InputToOutputFile fileFunction) {
    for (;;) {
        archive_entry *entry;
        int r = archive_read_next_header(input, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r != ARCHIVE_OK) {
            return r;
        }
        QString result = fileFunction(archive_entry_pathname_utf8(entry));
        if (!result.isEmpty()) {
            archive_entry_set_pathname(entry, result.toUtf8().data());
            r = archive_write_header(output, entry);
            if (r != ARCHIVE_OK) {
                return r;
            }
            if (archive_entry_size(entry) > 0) {
                r = copyData(input, output);
                if (r != ARCHIVE_OK) {
                    return r;
                }
            }
            r = archive_write_finish_entry(output);
            if (r != ARCHIVE_OK) {
                return r;
            }
        }
    }
    return ARCHIVE_OK;
}


#endif // ARCHIVEUTIL_H
