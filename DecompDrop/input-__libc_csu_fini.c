unsigned long __anvill_reg_RAX;
unsigned int __libc_csu_fini();
unsigned int __libc_csu_fini() {
    return ((unsigned int)__anvill_reg_RAX);
}
