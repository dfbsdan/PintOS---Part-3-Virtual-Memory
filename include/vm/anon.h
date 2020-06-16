#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

/* Types of anon_pages. */
enum anon_type {STACK, OTHER};//////////////////////////////////////////////////More types might be required

struct anon_page {
  struct page *page;          /* Pointer to the owner page. */
  struct hash_elem swap_elem; /* Element used in the swap table to keep track of
                               * the page's swap slot. */
  size_t idx;                 /* Index of the swap slot. */
  enum anon_type a_type;
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif
