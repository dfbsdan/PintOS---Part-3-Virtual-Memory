/* file.c: Implementation of memory mapped file object (mmaped object). */

#include "vm/vm.h"

static bool file_map_swap_in (struct page *page, void *kva);
static bool file_map_swap_out (struct page *page);
static void file_map_destroy (struct page *page);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_map_swap_in,
	.swap_out = file_map_swap_out,
	.destroy = file_map_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file mapped page */
bool
file_map_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_map_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_map_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file mapped page. PAGE will be freed by the caller. */
static void
file_map_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Do the mmap */
bool *
do_mmap (void *addr, size_t length, int writable,
		struct file *file, off_t offset) {
	struct thread *curr = thread_current();
	struct mmap_elem *m_elem = (struct mmap_desc*) malloc(sizeof(struct mmap_desc));
	m_elem->file = file;
	m_elem->addr = addr;
	list_init(&m_elem->page_list);
	size_t pointer;
	void *uaddr;
	for (pointer = 0;, pointer<length;, pointer += PGSIZE){
		uaddr = addr+pointer;
		vm_alloc_page_with_initializer(type, uaddr, writable, vm_initializer *init UNUSED, void *aux UNUSED);
		//init, aux for mmaping? aux needs to hold offset+pointer, file, length
		struct page *page = spt_find_page(&thread_current()->spt, uaddr);
		list_push_back(&m_elem->page_list, &page->mmap_elem);
	}

	list_push_front(&curr->mmaped_list, &m_elem->elem);
}

/* Do the munmap */
void
do_munmap (void *addr) {
}
