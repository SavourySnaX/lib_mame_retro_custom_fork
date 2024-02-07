// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
//============================================================
//
//  libretro.c - MAME debug module for libretro debugging extension
//
//============================================================

#include "emu.h"
#include "debug_module.h"

#include "debug/debugvw.h"
#include "debug/dvdisasm.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"
#include "debug/textbuf.h"
#include "debugger.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"


namespace osd {

namespace {

class debug_libretro : public osd_module, public debug_module
{
public:
	debug_libretro() :
		osd_module(OSD_DEBUG_PROVIDER, "libretro"), debug_module(),
		m_machine(nullptr)
	{
	}

	virtual ~debug_libretro() { }

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		return 0;
	}
	virtual void exit() override { }

	virtual void init_debugger(running_machine &machine) override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;
	virtual void debugger_update() override;

private:
	running_machine *m_machine;
};

void debug_libretro::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

void debug_libretro::wait_for_debugger(device_t &device, bool firststop)
{
	// Called when we are stopped, firststop indicates we have just stopped?
	/*
	while (m_machine->debugger().cpu().is_stopped())
	{
		osd_sleep(osd_ticks_per_second() / 10);
	}

	m_machine->debugger().console().get_visible_cpu()->debug()->go();*/

	device.machine().osd().update(false);	// do refresh
	osd_sleep(osd_ticks_per_second() / 10);	// short delay 
}

void debug_libretro::debugger_update()
{
	// Called to update debugger state - comms back to libretro here
}

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(DEBUG_LIBRETRO, osd::debug_libretro)
