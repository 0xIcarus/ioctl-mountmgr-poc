# MountMgr IOCTL – Proof-of-Concept LPE

**What I'm trying to achieve:**  
Send one crafted `IOCTL_MOUNTMGR_CREATE_POINT` request to `mountmgr.sys` >> corrupt kernel pool >> local `NT AUTHORITY\SYSTEM`.

> *This repo contains a simple poc that triggers only the crash path, so if you want, you can clone this repo and use it for testing*

---

## 1)  Discovery & Reverse-Engineering

| Step | Process | Tooling |
|------|-------------|---------|
| **Enumerate writable devices** | `Process Explorer` showed `\Device\MountPointManager` is world-read (`R`). | Sysinternals |
| **Locate IOCTL handler** | Loaded `mountmgr.sys` in **Ghidra** >> traced `DriverEntry` >> `FUN_1c001a078` where the IRP tables are initialised. | Ghidra |
| **Map vulnerable code path** | Found `FUN_1c0009010` (**IRP\_MJ_DEVICE_CONTROL dispatcher**) >> `case 0x6DC000` calls `FUN_1c000e458` (Create Point). | Ghidra |
| **Identify bug** | In `FUN_1c000e458` the driver: <br>1. **allocates** pool: `size = MaxLength`<br>2. **copies** user data: `count = Length`<br> >> Large *Length*, tiny *MaxLength*   = overflow. | Static diffing |
| **Confirm at runtime** | Enabled **Driver Verifier** for `mountmgr.sys` (`verifier /standard /driver mountmgr.sys`) >> crafted header values >> instant BSoD (**BAD\_POOL\_HEADER**, corrupt block in `mountmgr.sys`). | Driver Verifier + WinDbg |

---

## 2)  Crash-only PoC (this repo)

```c
w[0] = 0xFFF0;
w[1] = 0x0004;
w[2] = 0xFFF0;
w[3] = 0x0004;
DeviceIoControl(h, IOCTL_MGR_CREATE, buf, sizeof(buf), 0, 0, &io, 0);
