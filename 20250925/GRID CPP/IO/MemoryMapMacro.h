#pragma once
#ifndef MEMORY_MAP_MACRO_H
#define MEMORY_MAP_MACRO_H

// Macro to read a file into memory using memory mapping.
// The macro opens the file, gets its size, creates a read-only file mapping,
// maps a view of the file into memory, and then creates a std::string_view over it.
#define READ_FILE_MEMORY(file_name, mappedFile)                  \
    do {                                                         \
        /* Open the file for reading. */                         \
        (mappedFile).hFile = CreateFileA(                        \
            (file_name),                                         \
            GENERIC_READ,                                        \
            FILE_SHARE_READ,                                     \
            nullptr,                                             \
            OPEN_EXISTING,                                       \
            FILE_ATTRIBUTE_NORMAL,                               \
            nullptr                                              \
        );                                                       \
        if ((mappedFile).hFile == INVALID_HANDLE_VALUE) {        \
            std::cerr << "Error opening file: " << GetLastError() \
                      << std::endl;                              \
            break;                                               \
        }                                                        \
                                                                 \
        /* Get the file size. */                                 \
        DWORD fileSize = GetFileSize((mappedFile).hFile, nullptr); \
        if (fileSize == INVALID_FILE_SIZE) {                     \
            std::cerr << "Error getting file size: "            \
                      << GetLastError() << std::endl;            \
            CloseHandle((mappedFile).hFile);                     \
            break;                                               \
        }                                                        \
                                                                 \
        /* Create a read-only file mapping. */                   \
        (mappedFile).hMapping = CreateFileMappingA(              \
            (mappedFile).hFile,                                  \
            nullptr,                                             \
            PAGE_READONLY,                                       \
            0,                                                   \
            fileSize,                                            \
            nullptr                                              \
        );                                                       \
        if ((mappedFile).hMapping == nullptr) {                  \
            std::cerr << "Error creating file mapping: "         \
                      << GetLastError() << std::endl;            \
            CloseHandle((mappedFile).hFile);                     \
            break;                                               \
        }                                                        \
                                                                 \
        /* Map a view of the file into memory. */                \
        (mappedFile).pMap = MapViewOfFile(                       \
            (mappedFile).hMapping,                               \
            FILE_MAP_READ,                                       \
            0,                                                   \
            0,                                                   \
            fileSize                                             \
        );                                                       \
        if ((mappedFile).pMap == nullptr) {                      \
            std::cerr << "Error mapping view of file: "          \
                      << GetLastError() << std::endl;            \
            CloseHandle((mappedFile).hMapping);                  \
            CloseHandle((mappedFile).hFile);                     \
            break;                                               \
        }                                                        \
                                                                 \
        /* Create a non-owning string_view over the mapped memory. */ \
        (mappedFile).view = std::string_view(                    \
            static_cast<const char*>((mappedFile).pMap),         \
            fileSize                                             \
        );                                                       \
    } while (0)


// Macro to write a file using memory mapping.
// This macro opens (or creates) the output file, sets its size to the given fileSize,
// creates a writeable mapping and view, and optionally copies data from an input mapping.
// It then sets up the std::string_view to access the output file's memory.
#define WRITE_FILE_MEMORY(output_file_name, mappedFile, fileSize)         \
    do {                                                                  \
        /* Open (or create) the output file with read/write access */       \
        (mappedFile).hFile = CreateFileA(                                 \
            (output_file_name),                                          \
            GENERIC_READ | GENERIC_WRITE,                                \
            0,                                                           \
            nullptr,                                                     \
            CREATE_ALWAYS,                                               \
            FILE_ATTRIBUTE_NORMAL,                                       \
            nullptr                                                      \
        );                                                                \
        if ((mappedFile).hFile == INVALID_HANDLE_VALUE) {                 \
            std::cerr << "Error opening output file: " << GetLastError()   \
                      << std::endl;                                         \
            break;                                                        \
        }                                                                 \
                                                                          \
        /* Set the file pointer and extend the file size */               \
        LARGE_INTEGER _wf_li;                                             \
        _wf_li.QuadPart = (fileSize);                                     \
        if (!SetFilePointerEx((mappedFile).hFile, _wf_li, nullptr, FILE_BEGIN)) {  \
            std::cerr << "Error setting file pointer on output file: "    \
                      << GetLastError() << std::endl;                     \
            CloseHandle((mappedFile).hFile);                              \
            break;                                                        \
        }                                                                 \
        if (!SetEndOfFile((mappedFile).hFile)) {                          \
            std::cerr << "Error setting end of file for output file: "     \
                      << GetLastError() << std::endl;                     \
            CloseHandle((mappedFile).hFile);                              \
            break;                                                        \
        }                                                                 \
                                                                          \
        /* Create a writeable file mapping for the output file */         \
        (mappedFile).hMapping = CreateFileMappingA(                       \
            (mappedFile).hFile,                                           \
            nullptr,                                                      \
            PAGE_READWRITE,                                               \
            0,                                                            \
            (fileSize),                                                   \
            nullptr                                                       \
        );                                                                \
        if ((mappedFile).hMapping == nullptr) {                           \
            std::cerr << "Error creating output file mapping: "           \
                      << GetLastError() << std::endl;                     \
            CloseHandle((mappedFile).hFile);                              \
            break;                                                        \
        }                                                                 \
                                                                          \
        /* Map a view of the output file into memory */                   \
        (mappedFile).pMap = MapViewOfFile(                                \
            (mappedFile).hMapping,                                        \
            FILE_MAP_WRITE,                                               \
            0,                                                            \
            0,                                                            \
            (fileSize)                                                    \
        );                                                                \
        if ((mappedFile).pMap == nullptr) {                               \
            std::cerr << "Error mapping output file view: "               \
                      << GetLastError() << std::endl;                     \
            CloseHandle((mappedFile).hMapping);                           \
            CloseHandle((mappedFile).hFile);                              \
            break;                                                        \
        }                                                                 \
                                                                          \
        /* Create a string_view over the mapped memory for convenience */   \
        (mappedFile).view = std::string_view(                             \
            static_cast<const char*>((mappedFile).pMap),                  \
            (fileSize)                                                    \
        );                                                                \
    } while (0)


// Macro to free the memory-mapped file resources.
// It unmaps the view and closes both the mapping and file handles.
#define FREE_FILE_MEMORY(mappedFile)                            \
    do {                                                        \
        if ((mappedFile).pMap != nullptr) {                     \
            UnmapViewOfFile((mappedFile).pMap);                 \
        }                                                       \
        if ((mappedFile).hMapping != nullptr) {                 \
            CloseHandle((mappedFile).hMapping);                 \
        }                                                       \
        if ((mappedFile).hFile != INVALID_HANDLE_VALUE) {       \
            CloseHandle((mappedFile).hFile);                    \
        }                                                       \
    } while (0)

#endif
