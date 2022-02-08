#pragma once

struct ThreadData {
	ULONG ThreadId;
	int Priority;
};

#define CTRL_CODE( DeviceType, Function, Method, Access) ((DeviceType << 16) | (Access << 14) | (Function << 2) | (Method))

#define WKD_PRIORITY_BOOSTER_DEVICE 0x8000

#define IOCTL_WKD_PRIORITY_BOOSTER_SET_PRIORITY CTRL_CODE(WKD_PRIORITY_BOOSTER_DEVICE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)