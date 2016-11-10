#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x98397cc5, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x992847d2, "kmalloc_caches" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x3a013b7d, "remove_wait_queue" },
	{ 0x347d750e, "input_unregister_handle" },
	{ 0x21a21ea4, "device_destroy" },
	{ 0xd71a9f7b, "__register_chrdev" },
	{ 0x47d0592e, "mutex_unlock" },
	{ 0x3793022e, "input_register_handler" },
	{ 0xe9608caa, "mutex_trylock" },
	{ 0xfa1ac32b, "input_set_abs_params" },
	{ 0x42f5b707, "input_event" },
	{ 0xffd5a395, "default_wake_function" },
	{ 0x465c4fb3, "input_unregister_handler" },
	{ 0xa86a1c12, "input_close_device" },
	{ 0x5f5602c6, "current_task" },
	{ 0xa62925c6, "__mutex_init" },
	{ 0x50eedeb8, "printk" },
	{ 0xbda451a8, "class_unregister" },
	{ 0xb4390f9a, "mcount" },
	{ 0x102821d6, "input_open_device" },
	{ 0xf1faf509, "device_create" },
	{ 0xfaaf0164, "input_register_device" },
	{ 0x4292364c, "schedule" },
	{ 0x69768c4a, "input_free_device" },
	{ 0x156b0a42, "kmem_cache_alloc_trace" },
	{ 0x4d158c03, "input_register_handle" },
	{ 0xe45f60d8, "__wake_up" },
	{ 0x1d2e87c6, "do_gettimeofday" },
	{ 0xd7bd3af2, "add_wait_queue" },
	{ 0x37a0cba, "kfree" },
	{ 0x2e60bace, "memcpy" },
	{ 0x3234bc00, "input_unregister_device" },
	{ 0xf62d269d, "class_destroy" },
	{ 0xd6e17aa6, "__class_create" },
	{ 0xf836806, "input_allocate_device" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("input:b*v*p*e*-e*k*r*a*m*l*s*f*w*");

MODULE_INFO(srcversion, "48A2A23A431B9B0038CB29C");
