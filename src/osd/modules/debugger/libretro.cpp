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

#include "libretro_shared.h"

namespace osd {

namespace {

class debug_libretro : public osd_module, public debug_module
{
public:
	debug_libretro() :
		osd_module(OSD_DEBUG_PROVIDER, "libretro"), debug_module(),
		m_machine(nullptr),
		main_cpu(nullptr),
		main_memory(nullptr),
		program_address_space(nullptr),
		m_disasm_view(nullptr),
		m_register_view(nullptr),
		m_initialised(false)
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

	// debugger callbacks
	const debug_view_char* requestViewData(int x, int y, int w, int h, int kind);
	const char* remoteCommandData(const char* command);
private:
	void initialise();

	std::string m_buffer;
	running_machine *m_machine;
    device_t *main_cpu;
    device_memory_interface* main_memory;
    address_space *program_address_space;

    debug_view_disasm* m_disasm_view;
    debug_view* m_register_view;

	
	bool m_initialised;
};

void debug_libretro::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

struct debugger_view_t
{
	const debug_view_char* (* Callback)(debug_libretro*,int,int,int,int,int);
	debug_libretro* _this;
};

struct debugger_command_t
{
	const char* (* Callback)(debug_libretro*,const char* command);
	debug_libretro* _this;
};

static const debug_view_char* viewRequest(debug_libretro* _this,int x,int y, int w, int h, int kind)
{
	return _this->requestViewData(x,y,w,h,kind);
}

static const char* remoteCommand(debug_libretro* _this, const char* command)
{
	return _this->remoteCommandData(command);
}

void debug_libretro::initialise()
{
	if (debugger_cb==nullptr)
	{
		log_cb(RETRO_LOG_INFO,"Waiting for debugger connection");
		return;
	}

	main_cpu = device_interface_enumerator<cpu_device>(m_machine->root_device()).first();
	main_memory = &main_cpu->memory();
	program_address_space = &main_memory->space(AS_PROGRAM);

	auto disasm_view = m_machine->debug_view().alloc_view(DVT_DISASSEMBLY, nullptr, this);
	m_disasm_view=downcast<debug_view_disasm*>(disasm_view);
	m_disasm_view->set_expression("curpc");
	m_register_view = m_machine->debug_view().alloc_view(DVT_STATE, nullptr, this);

	{
		debugger_view_t t;
		t.Callback=viewRequest;
		t._this=this;
		debugger_cb(0,&t);
	}
	{
		debugger_command_t t;
		t.Callback=remoteCommand;
		t._this=this;
		debugger_cb(1,&t);
	}

	m_initialised = true;
}

void debug_libretro::wait_for_debugger(device_t &device, bool firststop)
{
	if (!m_initialised)
	{
		initialise();
	}

	/*
	while (m_machine->debugger().cpu().is_stopped())
	{
		osd_sleep(osd_ticks_per_second() / 10);
	}

	m_machine->debugger().console().get_visible_cpu()->debug()->go();*/

	device.machine().osd().update(false);	// do refresh
	osd_sleep(osd_ticks_per_second() / 10);	// short delay 
}

const debug_view_char* debug_libretro::requestViewData(int x, int y, int w, int h, int kind)
{
	debug_view* view;
	switch (kind)	
	{
		case 0:
			m_disasm_view->set_expression("curpc");
			view=m_disasm_view;
			break;
		case 1:
			view=m_register_view;
			break;
		default:
			return nullptr;
	}
    debug_view_xy vsize;
    vsize.x = w;
    vsize.y = h;
    view->set_visible_size(vsize);
    debug_view_xy vpos;
    vpos.x = x;
    vpos.y = y;
    view->set_visible_position(vpos);
 	return view->viewdata();
}

const char* debug_libretro::remoteCommandData(const char* command)
{
	text_buffer &textbuf = m_machine->debugger().console().get_console_textbuf();
	text_buffer_clear(textbuf);
	m_machine->debugger().console().execute_command(std::string_view(command, strlen(command)), false);
	uint32_t nlines = text_buffer_num_lines(textbuf);
	m_buffer.clear();
	std::string reply;
	if (nlines != 0)
	{
		for (uint32_t i = 0; i < nlines; i++)
		{
			const char *line = text_buffer_get_seqnum_line(textbuf, i);
			m_buffer+=std::string(line)+"\n";
		}
	}
	return m_buffer.c_str();
}


void debug_libretro::debugger_update()
{
	if (!m_initialised)
	{
		return;
	}
}

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(DEBUG_LIBRETRO, osd::debug_libretro)
