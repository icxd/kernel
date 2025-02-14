#pragma once

#include <LibCore/Types.hpp>

#define IDE0_DATA 0x1F0
#define IDE0_STATUS 0x1F7
#define IDE0_COMMAND 0x1F7
#define BUSY 0x80
#define DRDY 0x40
#define DRQ 0x08
#define IDENTIFY_DRIVE 0xEC
#define READ_SECTORS 0x21

#define IDE0_DISK0 0
#define IDE0_DISK1 1
#define IDE1_DISK0 2
#define IDE1_DISK1 3

struct IDEDrive {
  u16 cylinders, heads, sectors_per_track;
};

extern void ide_init();
extern IDEDrive drive[4];

namespace Disk {
  void initialize();
  bool read_sectors(u32 sector_index, u16 count, u8 *buffer);
} // namespace Disk
