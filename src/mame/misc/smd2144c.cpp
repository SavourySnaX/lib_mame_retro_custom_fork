// license:BSD-3-Clause
// copyright-holders:

/*
'SMD2144C' PCB by unknown manufacturer

1x HD64F2144FA20 (H8S/2144)
1x 16.000 MHz XTAL
1x Xilinx XC9572XL
1x Xilinx XC9536
1x Analog Devices ADM691AR Microprocessor Supervisory Circuit
2x BS62LV256SC-70 Very Low Power CMOS SRAM (32K X 8 bit)

These games appear to be Tetris clones, but it wouldn't be surprising
if they had stealth gambling games, too.
Both dumped PCBs shared the same main CPU ROM and 2 of the 4 GFX ROMs. Same game with different GFX.
*/


#include "emu.h"

#include "cpu/h8/h8s2245.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class smd2144c_state : public driver_device
{
public:
	smd2144c_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{}

	void smd2144c(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map);
};


uint32_t smd2144c_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void smd2144c_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}


static INPUT_PORTS_START( smd2144c )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// no dips on PCB
INPUT_PORTS_END


void smd2144c_state::smd2144c(machine_config &config)
{
	H8S2245(config, m_maincpu, 16_MHz_XTAL); // actually HD64F2144FA20 (TODO: H8S/2100 series not implemented yet)
	m_maincpu->set_addrmap(AS_PROGRAM, &smd2144c_state::program_map);

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(smd2144c_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_888, 256); // TODO: wrong

	SPEAKER(config, "mono").front_center(); // TODO: sound? directly from H8?
}


ROM_START( thecastle )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "tetrix 0a98.u3", 0x00000, 0x80000, CRC(7dc03a3f) SHA1(a2928dea91a68542fce932f986f288d269b3def3) ) // labeled Tetrix but game pics show 'The Castle'

	ROM_REGION(0x400000, "gfx", 0) // TODO: ROM loading not verified
	ROM_LOAD( "mx27c8000.ic11", 0x000000, 0x100000, CRC(10cee0cb) SHA1(57daa974b9de38921a3c75edcf759cc803f22ec8) )
	ROM_LOAD( "mx27c8000.ic12", 0x100000, 0x100000, CRC(d5940d05) SHA1(5a3e8eccaf1934d9ae6c0e01bb1745225929a627) )
	ROM_LOAD( "mx27c8000.ic13", 0x200000, 0x100000, CRC(757d12b7) SHA1(8a7794c2bce70226dd047972a47fa1afec4bb0f1) )
	ROM_LOAD( "mx27c8000.ic14", 0x300000, 0x100000, CRC(f1a0898d) SHA1(32c2ed41e2f04e3b7d756a15f6e476baeb641669) )
ROM_END

ROM_START( therock )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "the rock 1920.u3", 0x00000, 0x80000, CRC(7dc03a3f) SHA1(a2928dea91a68542fce932f986f288d269b3def3) ) // same as the castle set

	ROM_REGION(0x400000, "gfx", 0) // TODO: ROM loading not verified
	ROM_LOAD( "mx27c8000.ic11", 0x000000, 0x100000, CRC(bc89920d) SHA1(904d4fae2f7d0172b06aca9ffa48a1bdfa42bf58) )
	ROM_LOAD( "mx27c8000.ic12", 0x100000, 0x100000, CRC(ad496cb3) SHA1(ded21b9c60404a692d3c6d3f579fb7ecb0575a13) )
	ROM_LOAD( "mx27c8000.ic13", 0x200000, 0x100000, CRC(757d12b7) SHA1(8a7794c2bce70226dd047972a47fa1afec4bb0f1) ) // same as the castle set
	ROM_LOAD( "mx27c8000.ic14", 0x300000, 0x100000, CRC(f1a0898d) SHA1(32c2ed41e2f04e3b7d756a15f6e476baeb641669) ) // same as the castle set
ROM_END

} // Anonymous namespace


GAME( 200?, thecastle, 0,         smd2144c, smd2144c, smd2144c_state, empty_init, ROT0, "<unknown>", "The Castle", MACHINE_IS_SKELETON )
GAME( 200?, therock,   thecastle, smd2144c, smd2144c, smd2144c_state, empty_init, ROT0, "<unknown>", "The Rock",   MACHINE_IS_SKELETON )
