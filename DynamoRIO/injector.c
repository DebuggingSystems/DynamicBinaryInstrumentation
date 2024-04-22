#include <Windows.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: <PID>\n");
    return 0;
  }

  const DWORD ProcessId = strtol(argv[1], NULL, 10);
  const HANDLE Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
  fprintf(stdout, "Injector: Handle: %p\n", Handle);
  LPVOID Address = VirtualAllocEx(Handle, NULL, 0x100, MEM_COMMIT | MEM_RESERVE,
                                  PAGE_EXECUTE_READWRITE);
  if (Address == NULL)
    return 0;
  fprintf(stdout, "Injector: Allocated address: %p\n", Address);
  const UCHAR Buffer[0x100] = {0};
  SIZE_T Written = 0;
  fprintf(stdout, "Injector: Writing %llu bytes to address: %p\n",
          sizeof(Buffer), Address);
  if (!WriteProcessMemory(Handle, Address, &Buffer, sizeof(Buffer), &Written)) {
    return 0;
  }

  if (!CreateRemoteThread(Handle, NULL, 0, (LPTHREAD_START_ROUTINE)Address,
                          NULL, 0, NULL)) {
    return 0;
  }
  fprintf(stdout, "Injector: Creating thread on %p.\n", Address);
  return 0;
}