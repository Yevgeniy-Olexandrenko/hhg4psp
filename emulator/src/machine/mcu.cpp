#include "commons.h"
#include "mcu.h"

MCU::MCU(const u08* rom, IO& io)
	: m_io(io)
{
	std::memcpy(m_rom, rom, k_mcuROMSize);
	PowerOn();
}

void MCU::PowerOn()
{
//	TODO: carefully check behavior with documentation
	m_stack = 0;
	m_pc = 0;
	m_prev_pc = 0;
	m_op = 0;
	m_prev_op = 0;
	m_param = 0;
	m_acc = 0;
	m_bl = 0;
	m_bm = 0;
	m_c = 0;
	m_skip = false;
	m_div = 0;
	m_1s = false;
	m_bp = 0;
	m_halt = false;
	memset(m_ox, 0, sizeof(m_ox));
	memset(m_o, 0, sizeof(m_o));
	m_cn = 0;
	m_mx = 0;
	m_cb = 0;
	m_rsub = false;

	m_paramRead = false;
}

bool MCU::IsACLOccur()
{
	if (m_io.ReadAcl())	Reset();
	return m_aclClocks > 0;
}

void MCU::Reset()
{
//	TODO: carefully check behavior with documentation
	m_skip = false;
	m_halt = false;
	m_op = 0;
	m_prev_op = 0;
	do_branch_zero_cb(0x0f, 0x00);
	m_prev_pc = m_pc;
	m_bp = 1;
	m_io.WriteR(0);

	push_stack();
	op_idiv();
	m_1s = true;
	m_rsub = false;

	m_aclClocks = u16(0.5f * k_mcuClockFreqHz);
}

void MCU::Clock()
{
	if (m_aclClocks > 0) m_aclClocks--;
	IncrementDivider();

	if ((m_div & 0x0001) != 0)
	{
		if (IsACLOccur() || (m_halt && !IsWakeUpOccur()))
		{
			// got nothing to do
			return;
		}

		if (!m_paramRead)
		{
			// remember previous state
			m_prev_op = m_op;
			m_prev_pc = m_pc;

			// fetch next opcode
			m_op = AccessROM(m_pc);
			IncrementPC();

			// LBL and prefix opcodes are 2 bytes
			m_paramRead = (m_op == 0x5e || m_op == 0x5f);
		}
		else
		{
			// fetch opcode param
			m_param = AccessROM(m_pc);
			IncrementPC();

			m_paramRead = false;
		}

		if (!m_paramRead)
		{
			// handle opcode if it's not skipped
			if (m_skip)
			{
				m_skip = false;
				m_op = 0; // fake nop
			}
			else
			{
				ExecuteInstruction();
			}
		}
	}
}

void MCU::IncrementPC()
{
	// PL(program counter low 6 bits) is a simple LFSR: newbit = (bit0==bit1)
	// PU,PM(high bits) specify page, PL specifies steps within page
	int feed = ((m_pc >> 1 ^ m_pc) & 1) ? 0 : 0x20;
	m_pc = feed | (m_pc >> 1 & 0x1f) | (m_pc & ~0x3f);
}

bool MCU::IsWakeUpOccur()
{
	// this is necessary because the MCU can wake up on K input activity
	// set K input lines active state
	Flg k_active = ((m_io.ReadK() & 0xf) != 0);

	// in halt mode, wake up after 1S signal or K input
	if (k_active || m_1s)
	{
		// note: official doc warns that Bl/Bm and the stack are undefined
		// after waking up, but we leave it unchanged
		m_halt = false;
		do_branch_zero_cb(0x00, 0x00);
		return true;
	}
	return false;
}

void MCU::ExecuteInstruction()
{
	switch (m_op & 0xf0)
	{
	case 0x20: op_lax(); break;
	case 0x30: op_adx(); break;
	case 0x40: op_lb();  break;
	case 0x70: op_ssr(); break;
	case 0x80:
	case 0x90:
	case 0xa0:
	case 0xb0: op_tr();  break;
	case 0xc0:
	case 0xd0:
	case 0xe0:
	case 0xf0: op_trs(); break;
	default:
		switch (m_op & 0xfc)
		{
		case 0x04: op_rm();   break;
		case 0x0c: op_sm();   break;
		case 0x10: op_exc();  break;
		case 0x14: op_exci(); break;
		case 0x18: op_lda();  break;
		case 0x1c: op_excd(); break;
		case 0x54: op_tmi();  break;
		default:
			switch (m_op)
			{
			case 0x00: op_skip();  break;
			case 0x01: op_atr();   break;
			case 0x02: op_sbm();   break;
			case 0x03: op_atbp();  break;
			case 0x08: op_add();   break;
			case 0x09: op_add11(); break;
			case 0x0a: op_coma();  break;
			case 0x0b: op_exbla(); break;
			case 0x50: op_ta();   break;
			case 0x51: op_tb();    break;
			case 0x52: op_tc();    break;
			case 0x53: op_tam();   break;
			case 0x58: op_tis();   break;
			case 0x59: op_ptw();   break;
			case 0x5a: op_ta0();   break;
			case 0x5b: op_tabl();  break;
			case 0x5c: op_tw();    break;
			case 0x5d: op_dtw();   break;
			case 0x5f: op_lbl();   break;
			case 0x60: op_comcn(); break;
			case 0x61: op_pdtw();  break;
			case 0x62: op_wr();    break;
			case 0x63: op_ws();    break;
			case 0x64: op_incb();  break;
			case 0x65: op_idiv();  break;
			case 0x66: op_rc();    break;
			case 0x67: op_sc();    break;
			case 0x68: op_rmf();   break;
			case 0x69: op_smf();   break;
			case 0x6a: op_kta();   break;
			case 0x6b: op_rbm();   break;
			case 0x6c: op_decb();  break;
			case 0x6d: op_comcb(); break;
			case 0x6e: op_rtn0();  break;
			case 0x6f: op_rtn1();  break;
			case 0x5e:
				m_op = m_op << 8 | m_param;
				switch (m_param)
				{
				case 0x00: op_cend();  break;
				case 0x04: op_dta();   break;
				default: op_illegal(); break;
				}
				break;
			default: op_illegal(); break;
			}
			break;
		}
		break;
	}
}

void MCU::IncrementDivider()
{
	m_div = (m_div + 1) & 0x7fff;

	// 1S signal on overflow(falling edge of F1)
	if (m_div == 0) m_1s = true;

	//	update lcd every 15.625 ms
	if ((m_div & 0x01FF) == 0)
	{
		for (int o = 0; o < k_mcuLcdOCount; o++)
		{
			u08 h0segments = m_bp ? m_o[o] : 0;
			u08 h1segments = m_bp ? m_ox[o] : 0;
			m_io.WriteLCD(o, h1segments << 4 | h0segments);
		}
	}
}

//	ПЗУ имеет объём 1856 байт (организация 29 страниц по 64 байта). Однако в ОЭВМ использован 6 - битный полиномиальный счётчик команд,
//	а он имеет 63 состояния, и одно «запрещённое», в котором он не работает. Поэтому для программы используется 63 байта на каждой странице,
//	а 64й байт содержит команду загрузки исходного(нулевого) состояния счётчика. Итого доступно 29х63 = 1827 байт.
//	Полиномиальный 6 - битный счётчик – единственный «автоматический» счётчик адреса команд. Для адресации всего ПЗУ используется 4 - разрядный
//	регистр страниц, и бит выбора банка. Они изменяются только по командам программы.
//	При выборе банка 0 доступны первые 16 страниц ПЗУ.В банке 1 – 13 страниц.Выбирая страницы 14, 15, 16, получаем доступ всё равно к 13й
//	странице.Так устроен дешифратор страниц ОЭВМ.
u08 MCU::AccessROM(u16 address)
{
	if ((address & 0x0400) && (address & 0x03c0) > 0x0300)
	{
		address &= 0x043f;
		address |= 0x0300;
	}
	return m_rom[address];
}

//	ОЗУ имеет объём 65 полубайт. Организация – 5 страниц по 13 полубайт. Также как и ПЗУ для выбора ячейки использует регистр страниц ОЗУ(2 бита),
//	и бит выбора банка. В нулевом банке 4 страницы, в 1 - м – одна.
//	Особенность: если выбрать полубайт 14, 15, 16 – то при чтении получим значение полубайта 13 текущей страницы. А запись в старшие 3 полубайта
//	не имеет действия – ОЗУ не изменяется. Эта особенность используется в программе «Ну, погоди!».
//	Если в банке 1 выбрать страницы 1, 2, 3 – получим доступ к нулевой странице 1 - го банка.
u08 MCU::AccessRAM(u16 address)
{
	int page = (address & 0x70) >> 4;
	int byte = (address & 0x0f);
	if (page > 0x04) page = 0x04;
	if (byte > 0x0c) byte = 0x0c;
	return m_ram[page * 13 + byte];
}

void MCU::AccessRAM(u16 address, u08 data)
{
	int page = (address & 0x70) >> 4;
	int byte = (address & 0x0f);
	if (page > 0x04) page = 0x04;
	if (byte > 0x0c) byte = 0x0c;
	m_ram[page * 13 + byte] = data;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void MCU::op_lax()
{
	// LAX x: load ACC with immediate value, skip any next LAX
	if ((m_op & ~0xf) != (m_prev_op & ~0xf))
	{
		m_acc = m_op & 0xf;
	}
}

void MCU::op_adx()
{
	// ADX x: add immediate value to ACC, skip next on carry except if x = 10
	m_acc += (m_op & 0xf);
	m_skip = ((m_op & 0xf) != 10 && (m_acc & 0x10) != 0);
	m_acc &= 0xf;
}

void MCU::op_lb()
{
	// LB x: load BM/BL with 4-bit immediate value (partial)
	m_bm = m_op & 3;
	m_bl = (m_op >> 2 & 3) | ((m_op & 0xc) ? 8 : 0);
}

void MCU::op_ssr()
{
	// SSR x: set stack upper bits, also sets E flag for next opcode
	set_su(m_op & 0xf);
}

void MCU::op_tr()
{
	// TR x: jump (long or short)
	m_pc = (m_pc & ~0x3f) | (m_op & 0x3f);
	if (!m_rsub)
	{
		do_branch(m_cb, get_su(), m_pc & 0x3f);
	}
}

void MCU::op_trs()
{
	// TRS x: call subroutine
	if (!m_rsub)
	{
		m_rsub = true;
		u08 su = get_su();
		push_stack();
		do_branch(0x01, 0x00, m_op & 0x3f);

		// E flag was set?
		if ((m_prev_op & 0xf0) == 0x70)
		{
			do_branch(m_cb, su, m_pc & 0x3f);
		}
	}
	else
	{
		m_pc = (m_pc & ~0xff) | (m_op << 2 & 0xc0) | (m_op & 0xf);
	}
}

void MCU::op_rm()
{
	// RM x: reset RAM bit
	ram_w(ram_r() & ~bitmask(m_op));
}

void MCU::op_sm()
{
	// SM x: set RAM bit
	ram_w(ram_r() | bitmask(m_op));
}

void MCU::op_exc()
{
	// EXC x: exchange ACC with RAM, xor BM with x
	u08 a = m_acc;
	m_acc = ram_r();
	ram_w(a);
	m_bm ^= (m_op & 3);
}

void MCU::op_exci()
{
	// EXCI x: EXC x, INCB
	op_exc();
	op_incb();
}

void MCU::op_lda()
{
	// LDA x: load ACC with RAM, xor BM with x
	m_acc = ram_r();
	m_bm ^= (m_op & 3);
}

void MCU::op_excd()
{
	// EXCD x: EXC x, DECB
	op_exc();
	op_decb();
}

void MCU::op_tmi()
{
	// TMI x: skip next if RAM bit is set
	m_skip = ((ram_r() & bitmask(m_op)) != 0);
}

void MCU::op_skip()
{
	// SKIP: no operation
}

void MCU::op_atr()
{
	// ATR: output ACC to R
	u08 out = (~m_acc & 0xf);
	m_io.WriteR(out);
}

void MCU::op_sbm()
{
	// SBM: set RAM address high bit
	m_bm |= 4;
}

void MCU::op_atbp()
{
	// ATBP: output ACC to BP(internal LCD backplate signal) and set CN with ACC3
	m_bp = m_acc & 1;
	m_cn = m_acc >> 3 & 1;
}

void MCU::op_add()
{
	// ADD: add RAM to ACC
	m_acc = (m_acc + ram_r()) & 0xf;
}

void MCU::op_add11()
{
	// ADD11: add RAM and carry to ACC and carry, skip next on carry
	m_acc += ram_r() + m_c;
	m_c = m_acc >> 4 & 1;
	m_skip = (m_c == 1);
	m_acc &= 0xf;
}

void MCU::op_coma()
{
	// COMA: complement ACC
	m_acc ^= 0xf;
}

void MCU::op_exbla()
{
	// EXBLA: exchange BL with ACC
	u08 a = m_acc;
	m_acc = m_bl;
	m_bl = a;
}

void MCU::op_ta()
{
	// TAL: skip next if BA pin is set
	m_skip = m_io.ReadAlpha();
}

void MCU::op_tb()
{
	// TB: skip next if B(beta) pin is set
	m_skip = m_io.ReadBeta();
}

void MCU::op_tc()
{
	// TC: skip next if no carry
	m_skip = !m_c;
}

void MCU::op_tam()
{
	// TAM: skip next if ACC equals RAM
	m_skip = (m_acc == ram_r());
}

void MCU::op_tis()
{
	// TIS: skip next if 1S(gamma flag) is clear, reset it after
	m_skip = !m_1s;
	m_1s = false;
}

void MCU::op_ptw()
{
	// PTW: partial transfer W' to W
	m_o[k_mcuLcdOCount - 1] = m_ox[k_mcuLcdOCount - 1];
	m_o[k_mcuLcdOCount - 2] = m_ox[k_mcuLcdOCount - 2];
}

void MCU::op_ta0()
{
	// TA0: skip next if ACC is clear
	m_skip = !m_acc;
}

void MCU::op_tabl()
{
	// TABL: skip next if ACC equals BL
	m_skip = (m_acc == m_bl);
}

void MCU::op_tw()
{
	// TW: transfer W' to W
	for (int i = 0; i < k_mcuLcdOCount; i++)
	{
		m_o[i] = m_ox[i];
	}
}

void MCU::op_dtw()
{
	// DTW: shift digit into W'
	shift_w();
	m_ox[k_mcuLcdOCount - 1] = get_digit();
}

void MCU::op_lbl()
{
	// LBL xy: load BM/BL with 8-bit immediate value
	m_bl = (m_param & 0x0f);
	m_bm = (m_param & 0x70) >> 4;
}

void MCU::op_comcn()
{
	// COMCN: complement CN flag
	m_cn ^= 1;
}

void MCU::op_pdtw()
{
	// PDTW: partial shift digit into W'
	m_ox[k_mcuLcdOCount - 2] = m_ox[k_mcuLcdOCount - 1];
	m_ox[k_mcuLcdOCount - 1] = get_digit();
}

void MCU::op_wr()
{
	// WR: shift ACC into W', reset last bit
	shift_w();
	m_ox[k_mcuLcdOCount - 1] = m_acc & 7;
}

void MCU::op_ws()
{
	// WR: shift ACC into W', set last bit
	shift_w();
	m_ox[k_mcuLcdOCount - 1] = m_acc | 8;
}

void MCU::op_incb()
{
	// INCB: increment BL, skip next on overflow on 3rd bit!
	m_bl = (m_bl + 1) & 0xf;
	m_skip = (m_bl == 8);
}

void MCU::op_idiv()
{
	// IDIV: reset divider low 9 bits
	m_div &= 0x3f;
}

void MCU::op_rc()
{
	// RC: reset carry
	m_c = 0;
}

void MCU::op_sc()
{
	// SC: set carry
	m_c = 1;
}

void MCU::op_rmf()
{
	// RMF: reset m' flag, also clears ACC
	m_mx = 0;
	m_acc = 0;
}

void MCU::op_smf()
{
	// SMF: set m' flag
	m_mx = 1;
}

void MCU::op_kta()
{
	// KTA: input K to ACC
	m_acc = m_io.ReadK() & 0xf;
}

void MCU::op_rbm()
{
	// RBM: reset RAM address high bit
	m_bm &= ~4;
}

void MCU::op_decb()
{
	// DECB: decrement BL, skip next on overflow
	m_bl = (m_bl - 1) & 0xf;
	m_skip = (m_bl == 0xf);
}

void MCU::op_comcb()
{
	// COMCB: complement CB
	m_cb ^= 1;
}

void MCU::op_rtn0()
{
	// RTN0(RTN): return from subroutine
	pop_stack();
	m_rsub = false;
}

void MCU::op_rtn1()
{
	// RTN1: return from subroutine, skip next
	op_rtn0();
	m_skip = true;
}

void MCU::op_cend()
{
	// CEND: stop clock (halt the cpu and go into low-power mode)
	m_halt = true;
}

void MCU::op_dta()
{
	// DTA: transfer divider low 4 bits to ACC
	m_acc = m_div >> 11 & 0xf;
}

void MCU::op_illegal()
{
	::printf("Unknown opcode $%02X at $%04X\n", m_op, m_prev_pc);
}

void MCU::set_su(u08 su)
{
	m_stack = (m_stack & ~0x3c0) | (su << 6 & 0x3c0);
}

u08 MCU::get_su()
{
	return m_stack >> 6 & 0xf; 
}

void MCU::push_stack()
{
	m_stack = m_pc;
}

void MCU::pop_stack()
{
	m_pc = m_stack & 0x07ff;
}

void MCU::do_branch(u08 pu, u08 pm, u08 pl)
{
	// set new PC(Pu/Pm/Pl)
	m_pc = ((pu << 10) | (pm << 6 & 0x3c0) | (pl & 0x03f)) & 0x07ff;
}

void MCU::do_branch_zero_cb(u08 pm, u08 pl)
{
	m_cb = 0;
	do_branch(m_cb, pm, pl);
}

u08 MCU::bitmask(u16 param)
{
	// bitmask from immediate opcode param
	return 1 << (param & 3);
}

u08 MCU::ram_r()
{
	u08 address = (m_bm << 4 | m_bl) & 0x7f;
	return AccessRAM(address) & 0xf;
}

void MCU::ram_w(u08 data)
{
	u08 address = (m_bm << 4 | m_bl) & 0x7f;
	AccessRAM(address, data & 0xf);
}

void MCU::shift_w()
{
	// shifts internal W' latches
	for (int i = 0; i < k_mcuLcdOCount - 1; i++)
	{
		m_ox[i] = m_ox[i + 1];
	}
}

u08 MCU::get_digit()
{
	// default digit segments PLA
	static const u08 lut_digits[0x20] =
	{
		0xe, 0x0, 0xc, 0x8, 0x2, 0xa, 0xe, 0x2, 0xe, 0xa, 0x0, 0x0, 0x2, 0xa, 0x2, 0x2,
		0xb, 0x9, 0x7, 0xf, 0xd, 0xe, 0xe, 0xb, 0xf, 0xf, 0x4, 0x0, 0xd, 0xe, 0x4, 0x0
	};
	return lut_digits[m_cn << 4 | m_acc] | (~m_cn & m_mx);
}
