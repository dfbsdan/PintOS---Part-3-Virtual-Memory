/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include <hash.h>

static hash_hash_func spt_hash_func;
static hash_less_func spt_less_func;

/* Checks if a given address corresponds to the one of a page. */
static bool
is_page_addr (void *va) {
	return pg_round_down (va) == va;
}

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt, void *va) {
	struct spt_entry temp;
	struct hash_elem *elem;

	ASSERT (spt);
	ASSERT (is_page_addr (va)); //Debugging purposes: May be incorrect

	temp->page_va = va;
	elem = hash_find(&spt->table, &temp->h_elem);
	return (!elem)? NULL: hash_entry(elem, struct spt_entry, h_elem)->upage;
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt, struct page *page) {
	struct spt_entry *entry;

	ASSERT (spt);
	ASSERT (page);
	ASSERT (page->va);
	ASSERT (is_page_addr (page->va)); //Debugging purposes: May be incorrect

	entry = (struct spt_entry*)malloc (sizeof (struct spt_entry));
	if (!entry)
		return false;
	entry->page_va = page->va;
	entry->upage = page;
	if (hash_insert (&spt->table, &entry->h_elem)) {
		//The page is already in the table
		free (entry);
		return false;
	}
	return true;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame;
	void *kva;

	kva = palloc_get_page (PAL_USER);
	if (!kva)
		PANIC ("TODO: Evict a frame");

	ASSERT (is_page_addr (kva)); //Debugging purposes: May be incorrect

	frame = (struct frame*)malloc (sizeof (struct frame));
	ASSERT (frame);
	frame->kva = kva;
	frame->page = NULL;
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va) {
	struct page *page;

	ASSERT (va);
	ASSERT (is_page_addr (va)); //Debugging purposes: May be incorrect

	page = (struct page*)malloc (sizeof (struct page));
	if (!page)
		return false;
	page->va = va;
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();
	uint64_t *pml4 = thread_current ()->pml4;
	bool writable = true;																													//Is this correct?

	ASSERT (page && page->va);
	ASSERT (is_page_addr (page->va)); //Debugging purposes: May be incorrect

	/* Set links */
	frame->page = page;
	page->frame = frame;
	/* Insert page table entry to map page's VA to frame's PA. */
	return (pml4_set_page (pml4, page->va, frame->kva, writable))?
			swap_in (page, frame->kva): false;
}

/* Hash function for a supplemental_page_table entry holding a hash_elem E. */
static unsigned
spt_hash_func (const struct hash_elem *e, void *aux UNUSED) {
	struct spt_entry *entry;
	void **page_va;

	ASSERT (e);

	entry = hash_entry (e, struct spt_entry, h_elem);
	page_va = &entry->page_va;
	ASSERT (is_page_addr (*page_va)); //Debugging purposes: May be incorrect
	return hash_bytes (page_va, sizeof (*page_va));
}

/* Default function for comparison between two hash elements A and B that belong
 * to a supplemental_page_table entry (spt_entry).
 * Returns TRUE if A belongs to a spt_entry whose page_va value is less than
	 B's. */
static bool
spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED) {
	struct spt_entry *a_entry;
	struct spt_entry *b_entry;

	ASSERT (a && b);

	a_entry = hash_entry (a, struct spt_entry, h_elem);
	b_entry = hash_entry (b, struct spt_entry, h_elem);
	ASSERT (is_page_addr (a_entry->page_va)); //Debugging purposes: May be incorrect
	ASSERT (is_page_addr (b_entry->page_va)); //Debugging purposes: May be incorrect
	return a_entry->page_va < b_entry->page_va;
}

/* Initialize new supplemental page table */
bool
supplemental_page_table_init (struct supplemental_page_table *spt) {
	return hash_init (&spt->table, spt_hash_func, spt_less_func, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage.
	 * TODO: Handle page table initialization error (i.e. no lists allocated with
	 	hash_init())*/
}
