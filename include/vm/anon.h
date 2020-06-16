#ifndef VM_ANON_H
#define VM_ANON_H
#include "vm/vm.h"
struct page;
enum vm_type;

struct anon_page {
  struct page *page;          /* Pointer to the owner page. */
  struct hash_elem swap_elem; /* Element used in the swap table to keep track of
                               * the page's swap slot. */
  size_t idx;                 /* Index of the swap slot. */
};

void vm_anon_init (void);
bool anon_initializer (struct page *page, enum vm_type type, void *kva);

#endif
