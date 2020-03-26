//
// Created by x3vikan on 6/19/18.
//

#include "multiboot.h"

char * ramdisk = NULL;
struct multiboot * mboot_ptr = NULL;

struct multiboot *
copy_multiboot(
        struct multiboot *mboot_ptr
) {
    struct multiboot *new_header = (struct multiboot *)kmalloc(sizeof(struct multiboot));
    memcpy(new_header, mboot_ptr, sizeof(struct multiboot));
    return new_header;
}

void
dump_multiboot(
        struct multiboot *mboot_ptr
) {
    kprintf("MULTIBOOT header at 0x%x:\n", (uintptr_t)mboot_ptr);
    kprintf("Flags : 0x%x ",  mboot_ptr->flags);
    kprintf("Mem Lo: 0x%x ",  mboot_ptr->mem_lower);
    kprintf("Mem Hi: 0x%x ",  mboot_ptr->mem_upper);
    kprintf("Boot d: 0x%x\n", mboot_ptr->boot_device);
    kprintf("cmdlin: 0x%x ",  mboot_ptr->cmdline);
    kprintf("Mods  : 0x%x ",  mboot_ptr->mods_count);
    kprintf("Addr  : 0x%x ",  mboot_ptr->mods_addr);
    kprintf("Syms  : 0x%x\n", mboot_ptr->num);
    kprintf("Syms  : 0x%x ",  mboot_ptr->size);
    kprintf("Syms  : 0x%x ",  mboot_ptr->addr);
    kprintf("Syms  : 0x%x ",  mboot_ptr->shndx);
    kprintf("MMap  : 0x%x\n", mboot_ptr->mmap_length);
    kprintf("Addr  : 0x%x ",  mboot_ptr->mmap_addr);
    kprintf("Drives: 0x%x ",  mboot_ptr->drives_length);
    kprintf("Addr  : 0x%x ",  mboot_ptr->drives_addr);
    kprintf("Config: 0x%x\n", mboot_ptr->config_table);
    kprintf("Loader: 0x%x ",  mboot_ptr->boot_loader_name);
    kprintf("APM   : 0x%x ",  mboot_ptr->apm_table);
    kprintf("VBE Co: 0x%x ",  mboot_ptr->vbe_control_info);
    kprintf("VBE Mo: 0x%x\n", mboot_ptr->vbe_mode_info);
    kprintf("VBE In: 0x%x ",  mboot_ptr->vbe_mode);
    kprintf("VBE se: 0x%x ",  mboot_ptr->vbe_interface_seg);
    kprintf("VBE of: 0x%x ",  mboot_ptr->vbe_interface_off);
    kprintf("VBE le: 0x%x\n", mboot_ptr->vbe_interface_len);
    if (mboot_ptr->flags & (1 << 2)) {
        kprintf("Started with: %s\n", (char *)mboot_ptr->cmdline);
    }
    if (mboot_ptr->flags & (1 << 9)) {
        kprintf("Booted from: %s\n", (char *)mboot_ptr->boot_loader_name);
    }
    if (mboot_ptr->flags & (1 << 0)) {
        kprintf("%dkB lower memory\n", mboot_ptr->mem_lower);
        kprintf("%dkB higher memory ", mboot_ptr->mem_upper);
        int mem_mb = mboot_ptr->mem_upper / 1024;
        kprintf("(%dMB)\n", mem_mb);
    }
    if (mboot_ptr->flags & (1 << 3)) {
        kprintf("Found %d module(s).\n", mboot_ptr->mods_count);
        if (mboot_ptr->mods_count > 0) {
            uint32_t i;
            for (i = 0; i < mboot_ptr->mods_count; ++i ) {
                uint32_t module_start = *((uint32_t*)mboot_ptr->mods_addr + 8 * i);
                uint32_t module_end   = *(uint32_t*)(mboot_ptr->mods_addr + 8 * i + 4);
                kprintf("Module %d is at 0x%x:0x%x\n", i+1, module_start, module_end);
            }
        }
    }
}

void do_multiboot(struct multiboot* mboot){
    init_paging(mboot->mem_lower+mboot->mem_upper);
    if (mboot->flags & (1 << 6)) {
        kprintf("x86: multiboot: parsing memory map...\n");
        mboot_memmap_t *mmap = (void *)mboot->mmap_addr;
        while((uintptr_t)mmap < mboot->mmap_addr + mboot->mmap_length) {
            if (mmap->type == 2) {
                for (uint64_t i = 0; i < mmap->length; i += 0x1000) {
                    if (mmap->base_addr + i > 0xFFFFFFFF);
                    mark_region((mmap->base_addr + i) & 0xFFFFF000);
                }
            }
            mmap = (mboot_memmap_t *) ((uintptr_t)mmap + mmap->size + sizeof(uintptr_t));
        }
    }
    kprintf("x86: multiboot: parse complete\n");
}