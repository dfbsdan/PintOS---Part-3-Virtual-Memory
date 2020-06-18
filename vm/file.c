/* file.c: Implementation of memory mapped file object (mmaped object). */

#include "vm/vm.h"
#include "filesys/file.h"

static vm_initializer mmap_init;
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
	ASSERT (page && kva);
	ASSERT (vm_is_page_addr (page->va));
	ASSERT (VM_TYPE (type) == VM_FILE);
	ASSERT (page->frame && page->frame->kva == kva);
	/* Set up the handler */
	page->operations = &file_ops;
	return true;
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

/* Destory the file mapped page, which must NOT be in the current thread's spt.
 * PAGE will be freed by the caller. */
static void
file_map_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
	ASSERT (!spt_find_page (&thread_current ()->spt, page->va));
	/* TODO: writeback all the modified contents to the storage. */
}

/* Do the mmap */
void *
do_mmap (void *addr, size_t length, int writable, struct file *file,
		off_t offset) {
	struct file_page *m_elem;
	size_t page_cnt;
	void *uaddr = addr;

	ASSERT (vm_is_page_addr (addr) && length && file);

	page_cnt = (length % PGSIZE)? 1 + length / PGSIZE: length / PGSIZE;
	for (size_t i = 0; i < page_cnt; i++) {
		if (i != 0)
			/* Make sure that FILE is not destroyed until all pages are removed. */
			ASSERT (file_dup2 (file));
		/* Set up aux data and page. */
		m_elem = (struct file_page*)malloc (sizeof (struct file_page));
		if (m_elem) {
			m_elem->file = file;
			m_elem->offset = offset;
			m_elem->length = (length > PGSIZE)? PGSIZE: length;
			if (vm_alloc_page_with_initializer (VM_FILE, uaddr, writable,
					mmap_init, m_elem)) {
				uaddr += PGSIZE;
				offset += PGSIZE;
				length -= m_elem->length;
				continue;
			}
			free (m_elem);
		}
		file_close (file);
		return NULL;
	}
	ASSERT (length == 0);
	return addr;
}

/* Helper for the initialization of a file mapped page. */
static bool
mmap_init (struct page *page, void *aux_) {
	struct file_page *aux = (struct file_page*)aux_;
	struct file_page *file_page;

	ASSERT (page && vm_is_page_addr (page->va));
	ASSERT (page->frame && page->frame->kva
			== pml4_get_page (thread_current ()->pml4, page->va));
	ASSERT (VM_TYPE (type) == VM_FILE);
	ASSERT (aux && aux->file);

	file_page = &page->file;
	file_page->file = aux->file;
	file_page->offset = aux->offset;
	file_page->length = aux->length;
	ASSERT (file_page->length > 0 && file_page->length <= PGSIZE);
	free (aux);
	/* Read the data and fill the rest of the page with zeroes. */
	if ((size_t)file_read_at (ile_page->file, page->frame->kva, file_page->length,
			file_page->offset) == file_page->length) {
		if (file_page->length < PGSIZE)
			memset (page->frame->kva + file_page->length, 0,
					PGSIZE - file_page->length);
		return true;
	}
	///////////////////////////////////////////////////////////////////////////////Do sth else?
	return false;
}

/* Do the munmap */
void
do_munmap (void *addr) {
}
