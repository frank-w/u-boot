OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(bl2_entrypoint)
MEMORY {
    RAM (rwx): ORIGIN = 0x201000, LENGTH = (0x201000 + 0x80000 - 0x1000) - 0x201000
}
SECTIONS {
    RAM_REGION_START = ORIGIN(RAM);
    RAM_REGION_LENGTH = LENGTH(RAM);
    . = 0x201000;
    ASSERT(. == ALIGN(((1) << (12))),
        "BL2_BASE address is not aligned on a page boundary.")
    .ro . : {
        __RO_START__ = .;
        __TEXT_RESIDENT_START__ = .;
        *bl2_el3_entrypoint.o(.text*)
        *(.text.asm.*)
        __TEXT_RESIDENT_END__ = .;
        *(SORT_BY_ALIGNMENT(.text*))
        *(SORT_BY_ALIGNMENT(.rodata*))
        . = ALIGN(8); __RT_SVC_DESCS_START__ = .; KEEP(*(.rt_svc_descs)) __RT_SVC_DESCS_END__ = .; . = ALIGN(8); __FCONF_POPULATOR_START__ = .; KEEP(*(.fconf_populator)) __FCONF_POPULATOR_END__ = .; . = ALIGN(8); __PMF_SVC_DESCS_START__ = .; KEEP(*(.pmf_svc_descs)) __PMF_SVC_DESCS_END__ = .; . = ALIGN(8); __PARSER_LIB_DESCS_START__ = .; KEEP(*(.img_parser_lib_descs)) __PARSER_LIB_DESCS_END__ = .; . = ALIGN(8); __CPU_OPS_START__ = .; KEEP(*(.cpu_ops)) __CPU_OPS_END__ = .; . = ALIGN(8); __GOT_START__ = .; *(.got) __GOT_END__ = .;
        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        . = ALIGN(((1) << (12)));
        __RO_END__ = .;
    } >RAM
    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
        "cpu_ops not defined for this platform.")
    __RW_START__ = .;
    .data . : ALIGN(1) { __DATA_START__ = .; *(SORT_BY_ALIGNMENT(.data*)) __DATA_END__ = .; } >RAM AT>RAM
    __DATA_RAM_START__ = __DATA_START__;
    __DATA_RAM_END__ = __DATA_END__;
    .rela.dyn : ALIGN(8) { __RELA_START__ = .; *(.rela*) __RELA_END__ = .; } >RAM
    .stacks (NOLOAD) : { __STACKS_START__ = .; *(.tzfw_normal_stacks) __STACKS_END__ = .; } >RAM
    .bss (NOLOAD) : ALIGN(16) { __BSS_START__ = .; *(SORT_BY_ALIGNMENT(.bss*)) *(COMMON) . = ALIGN((1 << 6)); __PMF_TIMESTAMP_START__ = .; KEEP(*(.pmf_timestamp_array)) . = ALIGN((1 << 6)); __PMF_PERCPU_TIMESTAMP_END__ = .; __PERCPU_TIMESTAMP_SIZE__ = ABSOLUTE(. - __PMF_TIMESTAMP_START__); . = . + (__PERCPU_TIMESTAMP_SIZE__ * ((0 + 4) - 1)); __PMF_TIMESTAMP_END__ = .; . = ALIGN(16); __BASE_XLAT_TABLE_START__ = .; *(.base_xlat_table) __BASE_XLAT_TABLE_END__ = .; __BSS_END__ = .; } >RAM
    .xlat_table (NOLOAD) : { __XLAT_TABLE_START__ = .; *(.xlat_table) __XLAT_TABLE_END__ = .; } >RAM
    .coherent_ram (NOLOAD) : ALIGN(((1) << (12))) {
        __COHERENT_RAM_START__ = .;
        *(.tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        . = ALIGN(((1) << (12)));
        __COHERENT_RAM_END__ = .;
    } >RAM
    __RW_END__ = .;
    __BL2_END__ = .;
    /DISCARD/ : {
        *(.dynsym .dynstr .hash .gnu.hash)
    }
    __BSS_SIZE__ = SIZEOF(.bss);
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;
    RAM_REGION_END = .;
    ASSERT(. <= (0x201000 + 0x80000 - 0x1000), "BL2 image has exceeded its limit.")
}
