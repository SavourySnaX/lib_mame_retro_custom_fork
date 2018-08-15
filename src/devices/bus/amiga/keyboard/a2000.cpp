// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Amiga 2000 Keyboard

	Manufactured by Cherry

	Very simple electronics:
	* Philips crystal labelled 6000.000
	* U1 Philips MAB 8039HL 11P (microcontroller)
	* U2 RCA CD74HCT373E (latches low address bits from data bus)
	* U3 Fairchild 74LS373 PC (buffers keyboard inputs on data bus)
	* U4 2k*8 EPROM with label 467

	Six-position single inline header to host:
	* Pin 1 goes to U1 /INT and P26 (space for pull-up unpopulated)
	* Pin 2 goes to U1 T1 and P27 (space for pull-up unpopulated)
	* Pin 3 connected to ground/Vss
	* Pin 4 absent (key)
	* Pin 5 connected to Vdd/Vcc positive supply rail
	* Pin 6 connected to ground/Vss

	The program is supposed to support running from internal ROM (on an
	8049) or from external ROM as is the case for with the example we
	have.  To run from external ROM, EA and T0 are tied high (J6
	populated); to run from internal ROM, EA and T0 are tied low (J7
	populated).  The program reads keys using movx a,@r0 when running
	from external ROM, or ins a,bus when running from internal ROM.

***************************************************************************/

#include "emu.h"
#include "a2000.h"

#include "cpu/mcs48/mcs48.h"

#define LOG_GENERAL (1U << 0)
#define LOG_COMM    (1U << 1)
#define LOG_SCAN    (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_COMM)
#include "logmacro.h"

#define LOGCOMM(...)    LOGMASKED(LOG_COMM, __VA_ARGS__)
#define LOGSCAN(...)    LOGMASKED(LOG_SCAN, __VA_ARGS__)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_NS(A2000_KBD_US, bus::amiga::keyboard, a2000_kbd_us_device, "a2000kbd_us", "Amiga 2000 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_NS(A2000_KBD_GB, bus::amiga::keyboard, a2000_kbd_gb_device, "a2000kbd_gb", "Amiga 2000 Keyboard (UK)")



namespace bus { namespace amiga { namespace keyboard {

namespace {

INPUT_PORTS_START(a2000_common_keyboard)
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)          PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)         PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(8)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)         PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_SHIFT_2)             PORT_NAME("Right Alt")

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)   PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)   PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)       PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)       PORT_CHAR(UCHAR_MAMEKEY(6_PAD))

	PORT_START("ROW6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)       PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)       PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)       PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)       PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)       PORT_CHAR(UCHAR_MAMEKEY(4_PAD))

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(LALT))       PORT_NAME("Left Alt")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)       PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)             PORT_NAME("Left Shift")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))   PORT_NAME("Ctrl")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)        PORT_CHAR(UCHAR_MAMEKEY(LWIN))       PORT_NAME("Left Amiga")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR(9)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)        PORT_CHAR(UCHAR_MAMEKEY(RWIN))       PORT_NAME("Right Amiga")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))     PORT_NAME("Right Shift")

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)          PORT_CHAR(UCHAR_MAMEKEY(F7))

	PORT_START("ROW10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)          PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("ROW11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)          PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT))     PORT_NAME("Help")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)         PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)       PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
INPUT_PORTS_END

INPUT_PORTS_START(a2000_us_keyboard)
	PORT_INCLUDE(a2000_common_keyboard)

	PORT_MODIFY("ROW0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR(0x00bb)                    // »
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR(0x00bd)                    // ½
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(0x00ad) PORT_CHAR(0x00af)  // soft hyphen ¯
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(0x02c6)                    // ˆ
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(0x00b6)                    // ¶
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(0x00a4) PORT_CHAR(0x00a5)  // ¤ ¥

	PORT_MODIFY("ROW1")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR(0x00bc)                    // ¼
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(0x00ba)                    // º
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(0x02cb)                    // ˋ
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(0x00fe) PORT_CHAR(0x00de)  // þ Þ

	PORT_MODIFY("ROW2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR(0x00b2)                    // ²
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(0x00d7) PORT_CHAR(0x00f7)  // × ÷
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(0x00df) PORT_CHAR(0x00a7)  // ß §
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(0x00b0)                    // °

	PORT_MODIFY("ROW3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR(0x00b9)                    // ¹
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(0x00b1) PORT_CHAR(0x00ac)  // ± ¬
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(0x00e6) PORT_CHAR(0x00c6)  // æ Æ
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(0x00e5) PORT_CHAR(0x00c5)  // å Å

	PORT_MODIFY("ROW4")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR(0x00b3)                    // ³
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(0x00e7) PORT_CHAR(0x00c7)  // ç Ç
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(0x00f0) PORT_CHAR(0x00d0)  // ð Ð
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(0x00a9)                    // ©

	PORT_MODIFY("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)     PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))    PORT_NAME("Keypad (")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)     PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))    PORT_NAME("Keypad )")

	PORT_MODIFY("ROW8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR(0x00a2)                    // ¢
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(0x00aa)                    // ª
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(0x00b4)                    // ´
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(0x00ae)                    // ®

	PORT_MODIFY("ROW9")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(0x00b7)                    // ·
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(0x00a8)                    // ¨
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(0x00a1) PORT_CHAR(0x00a6)  // ¡ ¦

	PORT_MODIFY("ROW10")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR(0x00ab)                    // «
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(0x00a3)                    // £
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(0x00f8) PORT_CHAR(0x00d8)  // ø Ø

	PORT_MODIFY("ROW11")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR(0x00be)                    // ¾
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR(0x00b8) PORT_CHAR(0x00bf)  // ¸ ¿
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(0x02dc)                    // ˜
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(0x00b5)                    // µ
INPUT_PORTS_END

INPUT_PORTS_START(a2000_gb_keyboard)
	PORT_INCLUDE(a2000_us_keyboard)

	PORT_MODIFY("ROW2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR(0x00b2)                    // ²

	PORT_MODIFY("ROW4")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR(0x00a3) PORT_CHAR(0x00b3)                 // £ ³

	PORT_MODIFY("ROW10")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('#') PORT_CHAR('@') PORT_CHAR('\'') PORT_CHAR('"')
INPUT_PORTS_END


ROM_START(a2000kbd)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD("467.u4", 0x0000, 0x0800, CRC(fb92a773) SHA1(e787dc05de227f30a47ac5b9ee7a355c2e9e693b))
ROM_END

} // anonymous namespace



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// ======================> a2000_kbd_device

a2000_kbd_device::a2000_kbd_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_amiga_keyboard_interface(mconfig, *this)
	, m_rows(*this, "ROW%u", 0U)
	, m_mcu(*this, "u1")
	, m_row_drive(0U)
	, m_host_kdat(true)
	, m_mcu_kdat(true)
	, m_mcu_kclk(true)
{
}

WRITE_LINE_MEMBER(a2000_kbd_device::kdat_w)
{
	if (bool(state) != m_host_kdat)
	{
		LOGCOMM("host DATA %u -> %u\n", m_host_kdat ? 1 : 0, state ? 1 : 0);
		m_host_kdat = bool(state);
		if (m_mcu_kdat)
			m_mcu->set_input_line(MCS48_INPUT_IRQ, state ? CLEAR_LINE : ASSERT_LINE);
	}
}

READ8_MEMBER(a2000_kbd_device::mcu_bus_r)
{
	uint8_t result(0U);
	for (unsigned i = 0U; m_rows.size() > i; ++i)
	{
		if (BIT(m_row_drive, i))
			result |= uint8_t(m_rows[i]->read());
	}
	LOGSCAN("read bus: offset = %02X, row drive = %04X\n, result = %02X", offset, m_row_drive, result);
	return result;
}

WRITE8_MEMBER(a2000_kbd_device::mcu_p1_w)
{
	m_row_drive = (m_row_drive & 0x1f00U) | uint16_t(data);
}

WRITE8_MEMBER(a2000_kbd_device::mcu_p2_w)
{
	m_row_drive = (m_row_drive & 0x00ffU) | (uint16_t(data & 0x1fU) << 8);

	machine().output().set_value("led_kbd_caps", BIT(~data, 5));

	if (bool(BIT(data, 6) != m_mcu_kdat))
	{
		m_mcu_kdat = BIT(data, 6);
		LOGCOMM("keyboard DATA %u -> %u\n", m_mcu_kdat ? 0U : 1U, m_mcu_kdat ? 1U : 0U);
		m_host->kdat_w(m_mcu_kdat ? 1 : 0);
		if (m_host_kdat)
			m_mcu->set_input_line(MCS48_INPUT_IRQ, m_mcu_kdat ? CLEAR_LINE : ASSERT_LINE);
	}

	if (bool(BIT(data, 7) != m_mcu_kclk))
	{
		m_mcu_kclk = BIT(data, 7);
		LOGCOMM("keyboard CLOCK %u -> %u\n", m_mcu_kclk ? 0U : 1U, m_mcu_kclk ? 1U : 0U);
		m_host->kclk_w(m_mcu_kclk ? 1 : 0);
	}
}

tiny_rom_entry const *a2000_kbd_device::device_rom_region() const
{
	return ROM_NAME(a2000kbd);
}

void a2000_kbd_device::device_add_mconfig(machine_config &config)
{
	auto &mcu(I8039(config, "u1", 6_MHz_XTAL));
	mcu.set_addrmap(AS_PROGRAM, &a2000_kbd_device::program_map);
	mcu.set_addrmap(AS_IO, &a2000_kbd_device::ext_map);
	mcu.p1_out_cb().set(FUNC(a2000_kbd_device::mcu_p1_w));
	mcu.p2_out_cb().set(FUNC(a2000_kbd_device::mcu_p2_w));
	mcu.bus_in_cb().set(FUNC(a2000_kbd_device::mcu_bus_r));
	mcu.t0_in_cb().set_constant(1);
	mcu.t1_in_cb().set([this] () { return m_mcu_kclk ? 1 : 0; });
}

void a2000_kbd_device::device_start()
{
	save_item(NAME(m_row_drive));
	save_item(NAME(m_host_kdat));
	save_item(NAME(m_mcu_kdat));
	save_item(NAME(m_mcu_kclk));

	m_row_drive = 0U;
	m_host_kdat = true;
	m_mcu_kdat = true;
	m_mcu_kclk = true;
}

void a2000_kbd_device::device_reset()
{
}

void a2000_kbd_device::program_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).rom().region("mcu", 0);
}

void a2000_kbd_device::ext_map(address_map &map)
{
	map.global_mask(0x00ff);
	map(0x0000, 0x00ff).r(FUNC(a2000_kbd_device::mcu_bus_r));
}


// ======================> a2000_kbd_us_device

a2000_kbd_us_device::a2000_kbd_us_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: a2000_kbd_device(mconfig, A2000_KBD_US, tag, owner, clock)
{
}

ioport_constructor a2000_kbd_us_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2000_us_keyboard);
}


// ======================> a2000_kbd_gb_device

a2000_kbd_gb_device::a2000_kbd_gb_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: a2000_kbd_device(mconfig, A2000_KBD_GB, tag, owner, clock)
{
}

ioport_constructor a2000_kbd_gb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a2000_gb_keyboard);
}

} } } // namespace bus::amiga::keyboard
