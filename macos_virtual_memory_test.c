#include <stdio.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <assert.h>

typedef struct Test Test;
struct Test {
  int i;
  float f;
};

int main(void) {
  mach_vm_address_t base = 0x200000000;

  vm_size_t page;
  host_page_size(mach_host_self(), &page);
  printf("page size = %llu\n", (unsigned long long)page);

  mach_vm_size_t size = page;

  kern_return_t kr = mach_vm_allocate(
    mach_task_self(),
    &base,
    size,
    VM_FLAGS_FIXED
  );

  if(kr != KERN_SUCCESS) {
    fprintf(stderr, "mach_vm_allocate failed: %s (%d)\n",
      mach_error_string(kr), kr);
    return 1;
  }

  kr = mach_vm_protect(
    mach_task_self(),
    base,
    size,
    FALSE,
    VM_PROT_READ | VM_PROT_WRITE
  );

  if(kr != KERN_SUCCESS) {
    fprintf(stderr, "mach_vm_allocate failed: %s (%d)\n",
      mach_error_string(kr), kr);
    return 1;
  }

  Test *t = (Test *)base;
  t->i = 123;
  t->f = 4.56f;

  printf("ptr = %p -> {%d, %f}\n", t, t->i, t->f);

  mach_vm_deallocate(mach_task_self(), base, size);

  return 0;
}
