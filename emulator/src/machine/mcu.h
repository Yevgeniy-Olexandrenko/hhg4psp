#pragma once

#include "constants.h"

class MCU
{

public:
	class IO
	{
	public:
		virtual Flg  ReadAcl() const = 0;
		virtual Flg  ReadAlpha() const = 0;
		virtual Flg  ReadBeta() const = 0;
		virtual u08  ReadK() const = 0;
		virtual void WriteR(u08 data) = 0;
		virtual void WriteLCD(int o, u08 s) = 0;
	};

	MCU(const u08* rom, IO& io);

	void Reset();
	void Clock();

private:
	void PowerOn();
	bool IsACLOccur();
	void IncrementPC();
	bool IsWakeUpOccur();
	void ExecuteInstruction();
	void IncrementDivider();

	u08  AccessROM(u16 address);
	u08  AccessRAM(u16 address);
	void AccessRAM(u16 addres, u08 data);

private:
	void op_lax();   void op_adx();   void op_lb();    void op_ssr();
	void op_tr();    void op_trs();   void op_rm();    void op_sm();
	void op_exc();   void op_exci();  void op_lda();   void op_excd();
	void op_tmi();   void op_skip();  void op_atr();   void op_sbm();
	void op_atbp();  void op_add();   void op_add11(); void op_coma();
	void op_exbla(); void op_ta();    void op_tb();    void op_tc();
	void op_tam();   void op_tis();   void op_ptw();   void op_ta0();
	void op_tabl();  void op_tw();    void op_dtw();   void op_lbl();
	void op_comcn(); void op_pdtw();  void op_wr();    void op_ws();
	void op_incb();  void op_idiv();  void op_rc();    void op_sc();
	void op_rmf();   void op_smf();   void op_kta();   void op_rbm();
	void op_decb();  void op_comcb(); void op_rtn0();  void op_rtn1();
	void op_cend();  void op_dta();   void op_illegal();

	void set_su(u08 su);
	u08  get_su();
	void push_stack();
	void pop_stack();
	void do_branch(u08 pu, u08 pm, u08 pl);
	void do_branch_zero_cb(u08 pm, u08 pl);
	u08  bitmask(u16 param);
	u08  ram_r();
	void ram_w(u08 data);
	void shift_w();
	u08  get_digit();

public:
	IO& m_io;
	u16 m_aclClocks;
	Flg m_paramRead;

	u08 m_rom[k_mcuROMSize];		//	rom 29x64x8
	u08 m_ram[k_mcuRAMSize];		//	ram 5x13x4

	u08 m_cb;
	u16 m_pc, m_prev_pc;	//	programm counter
	u16 m_op, m_prev_op;	//	operation code
	u08 m_param;			//	operation parameter
	Flg m_halt;				//	mcu is halted flag
	Flg m_skip;				//	skip next opcode flag

	u16 m_stack;
	Flg m_rsub;
		
	u08 m_ox[k_mcuLcdOCount];   // W' latch, max 9
	u08 m_o[k_mcuLcdOCount];    // W latch
	u08 m_cn;
	u08 m_bp;
	u08 m_mx;

	u08 m_acc;
	u08 m_bl;
	u08 m_bm;
	u08 m_c;
	
	u16 m_div;
	Flg m_1s;
};
