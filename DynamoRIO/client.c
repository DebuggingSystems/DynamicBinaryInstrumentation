#include <dr_api.h>
#include <drmgr.h>
#include <drwrap.h>

static void event_exit(void);
static void module_load_event(void *drcontext, const module_data_t *mod,
                              bool loaded);

static void open_process_post(void *wrapcxt, void *user_data);
static void virtual_alloc_ex_post(void *wrapcxt, void *user_data);
static void write_process_memory_pre(void *wrapcxt, OUT void **user_data);
static void create_remote_thread_pre(void *wrapcxt, OUT void **user_data);

DR_EXPORT void dr_client_main(client_id_t id, int argc, const char *argv[]) {
  drmgr_init();
  drwrap_init();
  dr_enable_console_printing();
  dr_register_exit_event(event_exit);
  drmgr_register_module_load_event(module_load_event);
}

static void event_exit() {
  drwrap_exit();
  drmgr_exit();
}

static void module_load_event(void *drcontext, const module_data_t *mod,
                              bool loaded) {
  (void)drcontext;
  (void)loaded;

  dr_fprintf(STDOUT, "Module: %s\n", mod->full_path);

  const app_pc open_process =
      (app_pc)(app_pc)dr_get_proc_address(mod->handle, "OpenProcess");
  const app_pc virtual_alloc_ex =
      (app_pc)dr_get_proc_address(mod->handle, "VirtualAllocEx");
  const app_pc write_process_memory =
      (app_pc)dr_get_proc_address(mod->handle, "WriteProcessMemory");
  const app_pc create_remote_thread =
      (app_pc)dr_get_proc_address(mod->handle, "CreateRemoteThread");

  /* We are interested in the returned handle */
  if (open_process != NULL &&
      drwrap_wrap(open_process, NULL, open_process_post)) {
    dr_fprintf(STDOUT, "Wrapped OpenProcess  @" PFX "\n", open_process);
  }

  /* We are interested in the address of the allocated memory */
  if (virtual_alloc_ex != NULL &&
      drwrap_wrap(virtual_alloc_ex, NULL, virtual_alloc_ex_post)) {
    dr_fprintf(STDOUT, "Wrapped VirtualAllocEx  @" PFX "\n", virtual_alloc_ex);
  }

  /* We are interested in the data being written */
  if (write_process_memory != NULL &&
      drwrap_wrap(write_process_memory, write_process_memory_pre, NULL)) {
    dr_fprintf(STDOUT, "Wrapped WriteProcessMemory  @" PFX "\n",
               write_process_memory);
  }

  /* We are interested in the address of the created thread */
  if (create_remote_thread != NULL &&
      drwrap_wrap(create_remote_thread, create_remote_thread_pre, NULL)) {
    dr_fprintf(STDOUT, "Wrapped CreateRemoteThread  @" PFX "\n",
               create_remote_thread);
  }
}

static void open_process_post(void *wrapcxt, void *user_data) {
  (void)user_data;
  void *handle = drwrap_get_retval(wrapcxt);
  dr_fprintf(STDOUT, "OpenProcess returned: %d\n", handle);
}

static void virtual_alloc_ex_post(void *wrapcxt, void *user_data) {
  (void)user_data;
  void *address = drwrap_get_retval(wrapcxt);
  dr_fprintf(STDOUT, "VirtualAllocEx returned: " PFX "\n", address);
}
static void write_process_memory_pre(void *wrapcxt, OUT void **user_data) {
  (void)user_data;
  /* hProcess */
  void *handle = drwrap_get_arg(wrapcxt, 0);
  /* lpBaseAddress */
  void *address = drwrap_get_arg(wrapcxt, 1);
  /* lpBuffer */
  void *buffer = drwrap_get_arg(wrapcxt, 2);
  /* nSize */
  size_t size = (size_t)drwrap_get_arg(wrapcxt, 3);
  dr_fprintf(STDOUT,
             "WriteProcessMemory: Wrote %llu bytes @" PFX " to " PFX
             " on process handle %d\n",
             size, buffer, address, handle);
}
static void create_remote_thread_pre(void *wrapcxt, OUT void **user_data) {
  (void)user_data;
  /* hProcess */
  void *handle = drwrap_get_arg(wrapcxt, 0);
  /* lpStartAddress */
  void *start_address = drwrap_get_arg(wrapcxt, 3);
  dr_fprintf(STDOUT,
             "CreateRemoteThread: Create thread @" PFX
             " using process handle " PFX "\n",
             start_address, handle);
}