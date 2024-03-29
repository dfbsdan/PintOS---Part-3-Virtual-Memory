#ifndef VM_FILE_H
#define VM_FILE_H
#include "vm/vm.h"
#include "filesys/file.h"

struct page;
enum vm_type;

struct file_page {
	struct page *page;
	struct file *file;
	off_t offset;
	size_t length;
	struct hash_elem um_elem;
};

void vm_file_init (void);
bool file_map_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap(void *addr, size_t length, int writable,
		struct file *file, off_t offset);
void do_munmap (void *va);
#endif
