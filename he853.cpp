#include "he853.h"

HE853Controller::HE853Controller()
{
	const char name[] = "HE853 USB device (04d9:01357 Holtek Semiconductor, Inc.)";
	const unsigned short idVendor = 0x4d9;
	const unsigned short idProduct = 0x1357;
	anban_cnt = 0;
	m_initialized = false;

#if RUN_DRY == 0
	struct hid_device_info *devs;
	devs = hid_enumerate(idVendor, idProduct);
	if (!devs) {
		printf("%s not found\n", name);
		hid_free_enumeration(devs);
	}
	else
	{
		hid_free_enumeration(devs);
		handle = hid_open(idVendor, idProduct, NULL);
		if (!handle) {
			printf("%s not fully accessible\n", name);
		}
		else
		{
			m_initialized = true;
		}
	}
#else
	m_initialized = true;
#endif
}

HE853Controller::~HE853Controller()
{
}

bool HE853Controller::sendOutputReports(uint8_t* buf, uint16_t nReports)
{
		int rv = 0;

#if DEBUG == 1
		DEBUG_PRINTF(("\nsendToStick:\n"));
		DEBUG_PRINTF(("  buffer: "));
		for(uint16_t h=0; h < sizeof(buf); h++ ) {
        DEBUG_PRINTF((" %02X", buf[h]));
		}
		DEBUG_PRINTF(("\n"));
#endif

		for (uint16_t i=0; i < nReports; i++) {
			DEBUG_PRINTF(("  Report %d report %02X first byte %02X\n", i, buf[i*9], buf[i*9+1]));
#if RUN_DRY == 0
			rv = hid_write(handle, buf + (i * 9), 9);
#else
			DEBUG_PRINTF(("  Dry run - nothing send\n"));
#endif
		}
		return (bool) rv;
}

bool HE853Controller::readDeviceStatus()
{
	uint8_t buf[9];
	if (!m_initialized) return false;

	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x00; // report id = 0, as it seems to be the only report
	buf[1] = 0x06;
	buf[2] = 0x01;
#if RUN_DRY == 0
	return sendOutputReports(buf, 1);
#else
	return true;
#endif
}

char HE853Controller::readDeviceName()
{
	return name;
}

#define MicroToTicksHe853(t,x) ((((t)->BaseTime) * ((t)->x)) / 10)
#define MicroToTicksMSB(t, x) (uint8_t) ((MicroToTicksHe853(t,x) >> 8) & 0xFF)
#define MicroToTicksLSB(t, x) (uint8_t) (MicroToTicksHe853(t,x) & 0xFF)
#define MicroToTicks(t, x) MicroToTicksLSB(t, x)

bool HE853Controller::sendRfData(He853Timings *t, uint8_t* data, uint8_t nDataBytes) {
	uint8_t rfCmdBuf[32 + 4 + 9]; // rename to output report cmdbuf + execute
	uint8_t i = 0;

	memset(rfCmdBuf, 0x00, sizeof(rfCmdBuf));

	rfCmdBuf[0*8+0+0] = 0x0;  // report id = 0, as it seems to be the only report
	rfCmdBuf[0*8+1+0] = 0x01; // index
		// StartBit_HTime
		rfCmdBuf[0*8+1+1] = MicroToTicksMSB(t,StartBitHighTime);
		rfCmdBuf[0*8+1+2] = MicroToTicksLSB(t,StartBitHighTime);
		// StartBit_LTime
		rfCmdBuf[0*8+1+3] = MicroToTicksMSB(t,StartBitLowTime);
		rfCmdBuf[0*8+1+4] = MicroToTicksLSB(t,StartBitLowTime);
		// EndBit_HTime
		rfCmdBuf[0*8+1+5] = MicroToTicksMSB(t,EndBitHighTime);
		rfCmdBuf[0*8+1+6] = MicroToTicksLSB(t,EndBitHighTime);
		// EndBit_LTime
		rfCmdBuf[0*8+1+7] = MicroToTicksMSB(t,EndBitLowTime);

	rfCmdBuf[1*9+0 + 0] = 0x0;  // report id = 0, as it seems to be the only report
	rfCmdBuf[1*9+1 + 0] = 0x2; // index

		// EndBit_LTime
		rfCmdBuf[1*9+1+1] = MicroToTicksLSB(t,EndBitLowTime);
		// DataBit0_HTime
		rfCmdBuf[1*9+1+2] = MicroToTicks(t,DataBit0HighTime);
		// DataBit0_LTime
		rfCmdBuf[1*9+1+3] = MicroToTicks(t,DataBit0LowTime);
		// DataBit1_HTime
		rfCmdBuf[1*9+1+4] = MicroToTicks(t,DataBit1HighTime);
		// DataBit1_LTime
		rfCmdBuf[1*9+1+5] = MicroToTicks(t,DataBit1LowTime);
		// DataBit_Count
		rfCmdBuf[1*9+1+6] = t->DataBitCount;
		// Frame_Count
		rfCmdBuf[1*9+1+7] = t->FrameCount;

	rfCmdBuf[2*9+0+0] = 0x0;  // report id = 0, as it seems to be the only report
	rfCmdBuf[2*9+1+0] = 0x03;
	DEBUG_PRINTF(("\nProtocol %s, %d Bytes, %d Bits:\n  ", t->ProtocolName, nDataBytes, t->DataBitCount));
	for (; i< nDataBytes && i < 7; i++) {
		DEBUG_PRINTF(("%02X ", data[i]));
		rfCmdBuf[2*9+1+1 + i] = data[i];
	}

	rfCmdBuf[3*9+0+0] = 0x0;  // report id = 0, as it seems to be the only report
	rfCmdBuf[3*9+1+0] = 0x04;

	DEBUG_PRINTF(("\n  "));
	for (;i< nDataBytes && i < 7+7; i++) {
		DEBUG_PRINTF(("%02X ", data[i]));
		rfCmdBuf[3*9+1+1 + i-7] = data[i];
	}

	rfCmdBuf[4*9+0+0] = 0x0;  // report id = 0, as it seems to be the only report
	rfCmdBuf[4*9+1+0] = 0x05; // Exec 'command'

	DEBUG_PRINTF(("\n"));

	return sendOutputReports(rfCmdBuf, 5);
}

const char cAnBan[] = "AnBan";
static struct He853Timings AnBanTimings = {
	(char *)&cAnBan,
	1, // no T
	320, 4800,
	0, 0,
	320, 960,
	960, 320,
	28,
	7
} ;

const char cUK[] = "UK";
static struct He853Timings UKTimings = {
	(char *)&cUK,
	1, // no T
	320, 9700,
	0,0,
	320, 960,
	960, 320,
	24,
	18
};

const char cEU[] = "EU";
static struct He853Timings EUTimings = {
	(char *)&cEU,
	1, // no T
	260, 8600,
	0, 0,
	260, 260,
	260, 1300,
	57,
	7
};


/*********************************************************************************************\
* Deze routine berekent de RAW pulsen uit een CMD_KAKU plaatst deze in de buffer RawSignal
*
* KAKU
* Encoding volgens Princeton PT2262 / MOSDESIGN M3EB / Domia Lite spec.
* Pulse (T) is 350us PWDM
* 0 = T,3T,T,3T, 1 = T,3T,3T,T, short 0 = T,3T,T,T
*
* KAKU ondersteund:
*   on/off       ---- 000x Off/On
*   all on/off   ---- 001x AllOff/AllOn // is group (unit code bestaat uit short 0s)
\*********************************************************************************************/

const char cKAKU[] = "KAKU";
static struct He853Timings KakuTimings = {
	(char *)&cKAKU,
	350, // Base Time us
	0, 0,
	1,32,
	1, 3,
	3, 1,
	24,
	7
};


/*********************************************************************************************\
* NewKAKU
* Encoding volgens Arduino Home Easy pagina
* Pulse (T) is 275us PDM
* 0 = T,T,T,4T, 1 = T,4T,T,T, dim = T,T,T,T op bit 27
*
* NewKAKU ondersteund:
*   on/off       ---- 000x Off/On
*   all on/off   ---- 001x AllOff/AllOn
*   dim absolute xxxx 0110 Dim16        // dim op bit 27 + 4 extra bits voor dim level
*
*  NewKAKU (org.)        = AAAAAAAAAAAAAAAAAAAAAAAAAACCUUUU(LLLL) -> A=KAKU_adres, C=commando, U=KAKU-Unit, L=extra dimlevel bits (optioneel)
*  Bit                   = 01234567890123456789012345678901 2345  -> Bit-0 gaat als eerste door de ether.
*                                    1111111111222222222233 3333
*
\*********************************************************************************************/

const char cKAKUNEW[] = "KAKUNEW";
static struct He853Timings KakuNewTimings = {
	(char *)&cKAKUNEW,
	275, // Base time in us
	1, 8,
	1,32,
	1, 1, //0 = 01, Dim = 00
	1, 4, // 1 = 10
	64,
	18
};


bool HE853Controller::sendRfData_AnBan(uint16_t deviceCode, uint8_t cmd)
{
	uint8_t data[4];
	unsigned int tb_fx[16] = { 0x609, 0x306, 0x803, 0xa08,
				   0x00a, 0x200, 0xc02, 0x40c,
				   0xe04, 0x70e, 0x507, 0x105,
				   0xf01, 0xb0f, 0xd0b, 0x90d };

	uint8_t gbuf[7];
	uint8_t kbuf[7];
	uint8_t cbuf[7];
	uint8_t idx;

	uint32_t temp;

	anban_cnt++;
	gbuf[0] = 1;
	gbuf[1] = (anban_cnt << 2) & 15;
	if (cmd > 0) { // if not off
		gbuf[1] |= 2;
	}
	gbuf[2] = deviceCode & 15;
	gbuf[3] = (deviceCode >> 4) & 15;
	gbuf[4] = (deviceCode >> 8) & 15;
	gbuf[5] = (deviceCode >> 12) & 15;
	if (cmd >= 9 || cmd == 0) {
		gbuf[6] = 0;
	} else {
		gbuf[6] = cmd - 1;
		gbuf[6] |= 8;
	}

	idx = gbuf[0];
	kbuf[0] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[1] ^ kbuf[0];
	kbuf[1] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[2] ^ kbuf[1];
	kbuf[2] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[3] ^ kbuf[2];
	kbuf[3] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[4] ^ kbuf[3];
	kbuf[4] = (uint8_t) (tb_fx[idx] >> 8);
	idx = gbuf[5] ^ kbuf[4];
	kbuf[5] = (uint8_t) (tb_fx[idx] >> 8);
	kbuf[6] = (uint8_t) gbuf[6];

	idx = kbuf[0];
	cbuf[0] = (uint8_t) tb_fx[idx];
	idx = kbuf[1] ^ cbuf[0];
	cbuf[1] = (uint8_t) tb_fx[idx];
	idx = kbuf[2] ^ cbuf[1];
	cbuf[2] = (uint8_t) tb_fx[idx];
	idx = kbuf[3] ^ cbuf[2];
	cbuf[3] = (uint8_t) tb_fx[idx];
	idx = kbuf[4] ^ cbuf[3];
	cbuf[4] = (uint8_t) tb_fx[idx];
	idx = kbuf[5] ^ cbuf[4];
	cbuf[5] = (uint8_t) tb_fx[idx];
	cbuf[6] = (uint8_t) (kbuf[6] ^ 9);

	temp = (cbuf[6] << 0x18) | (cbuf[5] << 0x14) |
	       (cbuf[4] << 0x10) | (cbuf[3] << 0x0c) |
	       (cbuf[2] << 0x08) | (cbuf[1] << 0x04) | cbuf[0];
	temp = (temp >> 2) | ((temp & 3) << 0x1a);

	data[0] = (uint8_t) (temp >> 20);
	data[1] = (uint8_t) (temp >> 12);
	data[2] = (uint8_t) (temp >> 4);
	data[3] = (uint8_t) (temp << 4);

	DEBUG_PRINTF(("\nBuffer debug for %s:\n", AnBanTimings.ProtocolName));
	DEBUG_PRINTF(("  cbuf: %02x %02x %02x %02x %02x %02x %02x %02x\n", cbuf[0], cbuf[1], cbuf[2], cbuf[3], cbuf[4], cbuf[5], cbuf[6], cbuf[7]));
	DEBUG_PRINTF(("  gbuf: %02x %02x %02x %02x %02x %02x %02x %02x\n", gbuf[0], gbuf[1], gbuf[2], gbuf[3], gbuf[4], gbuf[5], gbuf[6], gbuf[7]));
	DEBUG_PRINTF(("  kbuf: %02x %02x %02x %02x %02x %02x %02x %02x\n", kbuf[0], kbuf[1], kbuf[2], kbuf[3], kbuf[4], kbuf[5], kbuf[6], kbuf[7]));

	return sendRfData(&AnBanTimings, data, sizeof(data));
}

bool HE853Controller::sendRfData_EU(uint16_t deviceCode, bool cmd)
{
	uint8_t data[8];
	int i = 0;

	uint8_t tb_fx[16] = { 0x07, 0x0b, 0x0d, 0x0e,
    		  	      0x13, 0x15, 0x16, 0x19,
			      0x1a, 0x1c, 0x03, 0x05,
			      0x06, 0x09, 0x0a, 0x0c };

	uint8_t buf[4] = { 0x00,
			(uint8_t) ((deviceCode >> 8) & 0xff),
			(uint8_t) (deviceCode & 0xff),
						0x00 };

	if (cmd == true)
		buf[3] |= 0x10;

	uint8_t gbuf[8] = { (uint8_t) (buf[0] >> 4),
 			    (uint8_t) (buf[0] & 15),
			    (uint8_t) (buf[1] >> 4),
			    (uint8_t) (buf[1] & 15),
			    (uint8_t) (buf[2] >> 4),
			    (uint8_t) (buf[2] & 15),
			    (uint8_t) (buf[3] >> 4),
			    (uint8_t) (buf[3] & 15) };

	uint8_t kbuf[8];
	for (i = 0; i < 8; i++) {
		kbuf[i] = (uint8_t) ((tb_fx[gbuf[i]] | 0x40) & 0x7f);
	}
	kbuf[0] |= 0x80;

	uint64_t t64 = 0;
	t64 = kbuf[0];
	for (i = 1; i < 8; i++)
	{
		t64 = (t64 << 7) | kbuf[i];
	}
	t64 = t64 << 7;

	data[0] = (uint8_t) (t64 >> 56);
	data[1] = (uint8_t) (t64 >> 48);
	data[2] = (uint8_t) (t64 >> 40);
	data[3] = (uint8_t) (t64 >> 32);
	data[4] = (uint8_t) (t64 >> 24);
	data[5] = (uint8_t) (t64 >> 16);
	data[6] = (uint8_t) (t64 >> 8);
	data[7] = (uint8_t) t64;

	DEBUG_PRINTF(("\nBuffer debug for %s:\n", EUTimings.ProtocolName));
	DEBUG_PRINTF(("  buf:  %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]));
	DEBUG_PRINTF(("  gbuf: %02x %02x %02x %02x %02x %02x %02x %02x\n", gbuf[0], gbuf[1], gbuf[2], gbuf[3], gbuf[4], gbuf[5], gbuf[6], gbuf[7]));
	DEBUG_PRINTF(("  kbuf: %02x %02x %02x %02x %02x %02x %02x %02x\n", kbuf[0], kbuf[1], kbuf[2], kbuf[3], kbuf[4], kbuf[5], kbuf[6], kbuf[7]));

	return sendRfData(&EUTimings, data, sizeof(data));
}

bool HE853Controller::sendRfData_UK(uint16_t deviceCode, bool cmd)
{
	uint8_t data[3];
	int8_t i = 0;
	uint16_t buf[8];

	for (i = 0; i < 8; i++) {
		buf[i] = deviceCode % 3;
		deviceCode /= 3;

		switch (buf[i]) {
			case 0:
				buf[i] = 0;
				break;
			case 1:
				buf[i] = 3;
				break;
			case 2:
				buf[i] = 1;
				break;
		}
	}

	uint16_t addr = 0;
	for (i = 7; i >= 0; i--) {
		addr = (uint16_t) ((addr << 2) | buf[i]);
	}

	uint8_t sbuf = 0x14;
	if (cmd == true)
		sbuf |= 0x01;

	data[0] = (uint8_t) (addr >> 8);
	data[1] = (uint8_t) addr;
	data[2] = sbuf;

	DEBUG_PRINTF(("\nBuffer debug for %s:\n", UKTimings.ProtocolName));
	DEBUG_PRINTF(("  buf: %02x %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]));

	return sendRfData(&UKTimings, data, sizeof(data));
}

bool HE853Controller::sendKaku(uint16_t deviceCode, bool cmd)
{
	uint8_t data[3];

	// C3 ON
	data[0] = 0b00010000;
	data[1] = 0b00010000;
	data[2] = 0b00010101;

	sendRfData(&KakuTimings, data, sizeof(data));

	return true;
}

bool HE853Controller::sendKakuNew(uint16_t deviceCode, bool cmd)
{
	uint8_t data[8];

	// C3 ON
	data[0] = 0x55; // 0b01010101;
	data[1] = 0xA9; // 0b10101001;
	data[2] = 0x96; // 0b10010110;
	data[3] = 0xA9; // 0b10101001;
	data[4] = 0x95; // 0b10010110;
	data[5] = 0x5A; // 0b01011010;
	data[6] = 0x96; // 0b10010110;
	data[7] = 0x55; // 0b01011001;

	sendRfData(&KakuNewTimings, data, sizeof(data));

	return true;
}

bool HE853Controller::getDeviceStatus(void)
{
	return readDeviceStatus();
}

char HE853Controller::getDeviceName(void)
{
	return readDeviceName();
}

bool HE853Controller::sendAnBan(uint16_t deviceId, uint8_t command)
{
	return sendRfData_AnBan(deviceId, command);
}

bool HE853Controller::sendUK(uint16_t deviceId, bool command)
{
	return sendRfData_UK(deviceId, command);
}

bool HE853Controller::sendEU(uint16_t deviceId, bool command)
{
	return sendRfData_EU(deviceId, command);
}

// This is for the lazy - better just for test purposes
bool HE853Controller::sendAll(uint16_t deviceId, uint8_t command)
{
	return sendAnBan(deviceId, command) &&
		sendEU(deviceId, (bool)command) &&
		sendKaku(deviceId, (bool)command) &&
		sendKakuNew(deviceId, (bool)command) &&
		sendUK(deviceId, (bool)command);
}
