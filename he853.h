#ifndef HE853_H__
#define HE853_H__

/* Timings

FName StartBit_HTime StartBit_LTime EndBit_HTime EndBit_LTime DataBit0_HTime DataBit0_LTime
AnBan  320            4800           0            0            320            960
UK     320            9700           0            0            320            960
EU     260            8600           0            0            260            260

FName  DataBit1_HTime DataBit1_LTime DataBit_Count Frame_Count Remark
AnBan  960            320            28            7           (??6Frame)??3?1??????V1.2
UK     960            320            24            18          (??16Frame)
EU     260            1300           57            7           (??6Frame)

*/
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <hidapi/hidapi.h> // http://www.signal11.us/oss/hidapi/

// Setting RUN_DRY to 1 will only emulate opening a device and nothing get's send
#define RUN_DRY 0
// Setting DEBUG to 1 gives you information about what is being send out
#define DEBUG   0

#if DEBUG == 1
# define DEBUG_PRINTF(x) printf x
#else
# define DEBUG_PRINTF(x) do {} while (0)
#endif

struct He853Timings {
	char* 		ProtocolName; // static char just to display
	uint16_t	BaseTime; // T, following Time fields are multiplied by BaseTime to form resulting times in microsecs... use 1 if you don't want mappings
	uint16_t	StartBitHighTime;
	uint16_t	StartBitLowTime;
	uint16_t	EndBitHighTime;
	uint16_t	EndBitLowTime;
	uint16_t	DataBit0HighTime;
	uint16_t	DataBit0LowTime;
	uint16_t	DataBit1HighTime;
	uint16_t	DataBit1LowTime;
	uint8_t		DataBitCount;
	uint8_t		FrameCount;
};

class HE853Controller {
private:
	hid_device *handle;
	uint8_t anban_cnt;
  bool m_initialized;
	char name;

public:
	HE853Controller();
	~HE853Controller();

private:
	bool sendOutputReports(uint8_t* buf, uint16_t nReports);
	bool readDeviceStatus();
	char readDeviceName(void);
	bool sendRfData(He853Timings *t, uint8_t* data, uint8_t nDataBytes);
	bool sendRfData_AnBan(uint16_t deviceCode, uint8_t cmd);
	bool sendRfData_EU(uint16_t deviceCode, bool cmd);
	bool sendRfData_UK(uint16_t deviceCode, bool cmd);
	bool execRfCommand();

public:
	bool getDeviceStatus(void);
	char getDeviceName(void);
	bool sendAnBan(uint16_t deviceId, uint8_t command);
	bool sendUK(uint16_t deviceId, bool command);
	bool sendEU(uint16_t deviceId, bool command);
	bool sendAll(uint16_t deviceId, uint8_t command);
	bool sendKaku(uint16_t deviceId, bool command);
	bool sendKakuNew(uint16_t deviceId, bool command);
};

#endif
