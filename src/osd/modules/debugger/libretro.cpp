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
#include "debug/dvmemory.h"
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

enum FormatSwitch
{
	AsmRightColumnNone=0x0000,
	AsmRightColumnRawOpcodes=0x0001,
	AsmRightColumnEncyptedOpcodes=0x0002,
	AsmRightColumnComments=0x0003,
	DataFormat1ByteHex=0x1000,
	DataFormat2ByteHex=0x1001,
	DataFormat4ByteHex=0x1002,
	DataFormat8ByteHex=0x1003,
	DataFormat1ByteOctal=0x1004,
	DataFormat2ByteOctal=0x1005,
	DataFormat4ByteOctal=0x1006,
	DataFormat8ByteOctal=0x1007,
	DataFormat32BitFloat=0x1008,
	DataFormat64BitFloat=0x1009,
	DataFormat80BitFloat=0x100A,
	HexAddress=0x2000,
	DecAddress=0x2001,
	OctAddress=0x2002,
	LogicalAddress=0x3000,
	PhysicalAddress=0x3001,
};

class debug_libretro;

struct retro_debug_view_t
{
	debug_libretro* _this;
	const char* expression;
	debug_view* view;
	int kind;
	int x, y;
	int w, h;
};

class debug_libretro : public osd_module, public debug_module
{
public:
	debug_libretro() :
		osd_module(OSD_DEBUG_PROVIDER, "libretro"), debug_module(),
		m_machine(nullptr),
		main_cpu(nullptr),
		main_memory(nullptr),
		program_address_space(nullptr),
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
	retro_debug_view_t* viewAllocData(int kind);
	void viewFreeData(retro_debug_view_t* view);
	const debug_view_char* requestViewData(const retro_debug_view_t* view);
	void updateFromExpressionData(retro_debug_view_t* view);
	void viewDataFormatData(const retro_debug_view_t* view, FormatSwitch format);
	const char* remoteCommandData(const char* command);
private:
	void initialise();

	std::string m_buffer;
	running_machine *m_machine;
    device_t *main_cpu;
    device_memory_interface* main_memory;
    address_space *program_address_space;

	bool m_initialised;
};

void debug_libretro::init_debugger(running_machine &machine)
{
	m_machine = &machine;
}

struct debugger_view_t
{
	retro_debug_view_t* (* AllocCallback)(debug_libretro*,int kind);
	void (* FreeCallback)(debug_libretro*,retro_debug_view_t* view);
	const debug_view_char* (* Update)(debug_libretro*,retro_debug_view_t* view);
	void (* ProcessChar)(debug_libretro*,retro_debug_view_t* view, int key);
	void (* UpdateExpression)(debug_libretro*,retro_debug_view_t* view);
	void (* DataFormat)(debug_libretro*,retro_debug_view_t* view, int format);
	debug_libretro* _this;
};

struct debugger_command_t
{
	const char* (* Callback)(debug_libretro*,const char* command);
	debug_libretro* _this;
};

static retro_debug_view_t* viewAlloc(debug_libretro* _this,int kind)
{
	return _this->viewAllocData(kind);
}

static void viewFree(debug_libretro* _this,retro_debug_view_t* view)
{
	_this->viewFreeData(view);
}

static const debug_view_char* viewRequest(debug_libretro* _this,retro_debug_view_t* view)
{
	return _this->requestViewData(view);
}

static void viewUpdateFromExpression(debug_libretro* _this, retro_debug_view_t* view)
{
	_this->updateFromExpressionData(view);
}

static void viewProcessChar(debug_libretro* _this, retro_debug_view_t* view, int key)
{
	view->view->process_char(key);
}

static void viewDataFormat(debug_libretro* _this, retro_debug_view_t* view, int format)
{
	_this->viewDataFormatData(view,(FormatSwitch)format);
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

	{
		debugger_view_t t;
		t.AllocCallback=viewAlloc;
		t.FreeCallback=viewFree;
		t.Update=viewRequest;
		t.UpdateExpression=viewUpdateFromExpression;
		t.ProcessChar=viewProcessChar;
		t.DataFormat=viewDataFormat;
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

retro_debug_view_t* debug_libretro::viewAllocData(int kind)
{
	retro_debug_view_t* view = new retro_debug_view_t();
	view->_this = this;
	view->kind = kind;
	view->view = m_machine->debug_view().alloc_view((debug_view_type)kind, nullptr, this);
	return view;
}

void debug_libretro::viewFreeData(retro_debug_view_t* view)
{
	m_machine->debug_view().free_view(*(view->view));
	delete view;
}
	
void debug_libretro::updateFromExpressionData(retro_debug_view_t* view)
{
	debug_view* v = view->view;
	switch (view->kind)
	{
		case DVT_DISASSEMBLY:
			downcast<debug_view_disasm*>(v)->set_expression(view->expression);
			break;
		case DVT_MEMORY:
			downcast<debug_view_memory*>(v)->set_expression(view->expression);
			break;
		default:
			break;
	}
	if(v->cursor_supported())
	{
		v->set_cursor_visible(true);
		view->y=v->visible_position().y;
		v->set_cursor_position(debug_view_xy(view->x,view->y));
	}
}

void debug_libretro::viewDataFormatData(const retro_debug_view_t* view, FormatSwitch format)
{
	auto v=view->view;

	switch (format)
	{
		case AsmRightColumnComments:
			downcast<debug_view_disasm*>(v)->set_right_column(DASM_RIGHTCOL_COMMENTS);
			break;
		case AsmRightColumnEncyptedOpcodes:
			downcast<debug_view_disasm*>(v)->set_right_column(DASM_RIGHTCOL_ENCRYPTED);
			break;
		case AsmRightColumnRawOpcodes:
			downcast<debug_view_disasm*>(v)->set_right_column(DASM_RIGHTCOL_RAW);
			break;
		case AsmRightColumnNone:
			downcast<debug_view_disasm*>(v)->set_right_column(DASM_RIGHTCOL_NONE);
			break;
		case DataFormat1ByteHex:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::HEX_8BIT);
			break;
		case DataFormat2ByteHex:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::HEX_16BIT);
			break;
		case DataFormat4ByteHex:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::HEX_32BIT);
			break;
		case DataFormat8ByteHex:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::HEX_64BIT);
			break;
		case DataFormat1ByteOctal:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::OCTAL_8BIT);
			break;
		case DataFormat2ByteOctal:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::OCTAL_16BIT);
			break;
		case DataFormat4ByteOctal:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::OCTAL_32BIT);
			break;
		case DataFormat8ByteOctal:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::OCTAL_64BIT);
			break;
		case DataFormat32BitFloat:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::FLOAT_32BIT);
			break;
		case DataFormat64BitFloat:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::FLOAT_64BIT);
			break;
		case DataFormat80BitFloat:
			downcast<debug_view_memory*>(v)->set_data_format(debug_view_memory::data_format::FLOAT_80BIT);
			break;
		case HexAddress:
			downcast<debug_view_memory*>(v)->set_address_radix(16);
			break;
		case DecAddress:
			downcast<debug_view_memory*>(v)->set_address_radix(10);
			break;
		case OctAddress:
			downcast<debug_view_memory*>(v)->set_address_radix(8);
			break;
		case LogicalAddress:
			downcast<debug_view_memory*>(v)->set_physical(false);
			break;
		case PhysicalAddress:
			downcast<debug_view_memory*>(v)->set_physical(true);
			break;
	}
}

const debug_view_char* debug_libretro::requestViewData(const retro_debug_view_t* view)
{
	debug_view* v = view->view;
	debug_view_xy vsize;
	vsize.x = view->w;
	vsize.y = view->h;
	v->set_visible_size(vsize);
	return v->viewdata();
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
