/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <stdint.h>
#include <string>

enum {
    INTERRUPT_SOURCE_NMI = 0,
    INTERRUPT_SOURCE_IRQ0,
    INTERRUPT_SOURCE_IRQ1,
    INTERRUPT_SOURCE_FRT0_ICI,
    INTERRUPT_SOURCE_FRT0_OCIA,
    INTERRUPT_SOURCE_FRT0_OCIB,
    INTERRUPT_SOURCE_FRT0_FOVI,
    INTERRUPT_SOURCE_FRT1_ICI,
    INTERRUPT_SOURCE_FRT1_OCIA,
    INTERRUPT_SOURCE_FRT1_OCIB,
    INTERRUPT_SOURCE_FRT1_FOVI,
    INTERRUPT_SOURCE_FRT2_ICI,
    INTERRUPT_SOURCE_FRT2_OCIA,
    INTERRUPT_SOURCE_FRT2_OCIB,
    INTERRUPT_SOURCE_FRT2_FOVI,
    INTERRUPT_SOURCE_TIMER_CMIA,
    INTERRUPT_SOURCE_TIMER_CMIB,
    INTERRUPT_SOURCE_TIMER_OVI,
    INTERRUPT_SOURCE_ANALOG,
    INTERRUPT_SOURCE_UART_RX,
    INTERRUPT_SOURCE_UART_TX,
    INTERRUPT_SOURCE_WDT,
    INTERRUPT_SOURCE_IRQ2,
    INTERRUPT_SOURCE_IRQ3,
    INTERRUPT_SOURCE_MAX
};

enum {
    EXCEPTION_SOURCE_ADDRESS_ERROR = 0,
    EXCEPTION_SOURCE_INVALID_INSTRUCTION,
    EXCEPTION_SOURCE_TRACE,
};

enum {
    DEV_P1DDR = 0x00,
    DEV_P5DDR = 0x08,
    DEV_P6DDR = 0x09,
    DEV_P7DDR = 0x0c,
    DEV_P7DR = 0x0e,
    DEV_FRT1_TCR = 0x10,
    DEV_FRT1_TCSR = 0x11,
    DEV_FRT1_FRCH = 0x12,
    DEV_FRT1_FRCL = 0x13,
    DEV_FRT1_OCRAH = 0x14,
    DEV_FRT1_OCRAL = 0x15,
    DEV_FRT1_OCRBH = 0x16,
    DEV_FRT1_OCRBL = 0x17,
    DEV_FRT2_TCR = 0x20,
    DEV_FRT2_TCSR = 0x21,
    DEV_FRT2_FRCH = 0x22,
    DEV_FRT2_FRCL = 0x23,
    DEV_FRT2_OCRAH = 0x24,
    DEV_FRT2_OCRAL = 0x25,
    DEV_FRT2_OCRBH = 0x26,
    DEV_FRT2_OCRBL = 0x27,
    DEV_FRT3_TCR = 0x30,
    DEV_FRT3_TCSR = 0x31,
    DEV_FRT3_FRCH = 0x32,
    DEV_FRT3_FRCL = 0x33,
    DEV_FRT3_OCRAH = 0x34,
    DEV_FRT3_OCRAL = 0x35,
    DEV_FRT3_OCRBH = 0x36,
    DEV_FRT3_OCRBL = 0x37,
    DEV_PWM1_TCR = 0x40,
    DEV_PWM1_DTR = 0x41,
    DEV_PWM2_TCR = 0x44,
    DEV_PWM2_DTR = 0x45,
    DEV_PWM3_TCR = 0x48,
    DEV_PWM3_DTR = 0x49,
    DEV_TMR_TCR = 0x50,
    DEV_TMR_TCSR = 0x51,
    DEV_TMR_TCORA = 0x52,
    DEV_TMR_TCORB = 0x53,
    DEV_TMR_TCNT = 0x54,
    DEV_SMR = 0x58,
    DEV_BRR = 0x59,
    DEV_SCR = 0x5a,
    DEV_TDR = 0x5b,
    DEV_SSR = 0x5c,
    DEV_RDR = 0x5d,
    DEV_ADDRAH = 0x60,
    DEV_ADDRAL = 0x61,
    DEV_ADDRBH = 0x62,
    DEV_ADDRBL = 0x63,
    DEV_ADDRCH = 0x64,
    DEV_ADDRCL = 0x65,
    DEV_ADDRDH = 0x66,
    DEV_ADDRDL = 0x67,
    DEV_ADCSR = 0x68,
    DEV_IPRA = 0x70,
    DEV_IPRB = 0x71,
    DEV_IPRC = 0x72,
    DEV_IPRD = 0x73,
    DEV_DTEA = 0x74,
    DEV_DTEB = 0x75,
    DEV_DTEC = 0x76,
    DEV_DTED = 0x77,
    DEV_WCR = 0x78,
    DEV_RAME = 0x79,
    DEV_P1CR = 0x7c,
    DEV_P9DDR = 0x7e,
    DEV_P9DR = 0x7f,
};

const uint16_t sr_mask = 0x870f;
enum {
    STATUS_T = 0x8000,
    STATUS_N = 0x08,
    STATUS_Z = 0x04,
    STATUS_V = 0x02,
    STATUS_C = 0x01,
    STATUS_INT_MASK = 0x700
};

enum {
    VECTOR_RESET = 0,
    VECTOR_RESERVED1, // UNUSED
    VECTOR_INVALID_INSTRUCTION,
    VECTOR_DIVZERO,
    VECTOR_TRAP,
    VECTOR_RESERVED2, // UNUSED
    VECTOR_RESERVED3, // UNUSED
    VECTOR_RESERVED4, // UNUSED
    VECTOR_ADDRESS_ERROR,
    VECTOR_TRACE,
    VECTOR_RESERVED5, // UNUSED
    VECTOR_NMI,
    VECTOR_RESERVED6, // UNUSED
    VECTOR_RESERVED7, // UNUSED
    VECTOR_RESERVED8, // UNUSED
    VECTOR_RESERVED9, // UNUSED
    VECTOR_TRAPA_0,
    VECTOR_TRAPA_1,
    VECTOR_TRAPA_2,
    VECTOR_TRAPA_3,
    VECTOR_TRAPA_4,
    VECTOR_TRAPA_5,
    VECTOR_TRAPA_6,
    VECTOR_TRAPA_7,
    VECTOR_TRAPA_8,
    VECTOR_TRAPA_9,
    VECTOR_TRAPA_A,
    VECTOR_TRAPA_B,
    VECTOR_TRAPA_C,
    VECTOR_TRAPA_D,
    VECTOR_TRAPA_E,
    VECTOR_TRAPA_F,
    VECTOR_INTERRUPT_80,          // IRQ0
    VECTOR_INTERRUPT_84,          // 532:IRQ1, 510:WDT
    VECTOR_INTERNAL_INTERRUPT_88, // UNUSED
    VECTOR_INTERNAL_INTERRUPT_8C, // UNUSED
    VECTOR_INTERNAL_INTERRUPT_90, // 532:FRT1 ICI, 510:IRQ1
    VECTOR_INTERNAL_INTERRUPT_94, // 532:FRT1 OCIA, 510:IRQ2
    VECTOR_INTERNAL_INTERRUPT_98, // 532:FRT1 OCIB, 510:IRQ3
    VECTOR_INTERNAL_INTERRUPT_9C, // 532:FRT1 FOVI, 510:UNUSED
    VECTOR_INTERNAL_INTERRUPT_A0, // 532:FRT2 ICI, 510:FRT1 ICI
    VECTOR_INTERNAL_INTERRUPT_A4, // 532:FRT2 OCIA, 510:FRT1 OCIA
    VECTOR_INTERNAL_INTERRUPT_A8, // 532:FRT2 OCIB, 510:FRT1 OCIB
    VECTOR_INTERNAL_INTERRUPT_AC, // 532:FRT2 FOVI, 510:FRT1 FOVI
    VECTOR_INTERNAL_INTERRUPT_B0, // 532:FRT3 ICI, 510:FRT2 ICI
    VECTOR_INTERNAL_INTERRUPT_B4, // 532:FRT3 OCIA, 510:FRT2 OCIA
    VECTOR_INTERNAL_INTERRUPT_B8, // 532:FRT3 OCIB, 510:FRT2 OCIB
    VECTOR_INTERNAL_INTERRUPT_BC, // 532:FRT3 FOVI, 510:FRT2 FOVI
    VECTOR_INTERNAL_INTERRUPT_C0, // CMIA
    VECTOR_INTERNAL_INTERRUPT_C4, // CMIB
    VECTOR_INTERNAL_INTERRUPT_C8, // OVI
    VECTOR_INTERNAL_INTERRUPT_CC, // UNUSED
    VECTOR_INTERNAL_INTERRUPT_D0, // SCI1 ERI
    VECTOR_INTERNAL_INTERRUPT_D4, // SCI1 RXI
    VECTOR_INTERNAL_INTERRUPT_D8, // SCI1 TXI
    VECTOR_INTERNAL_INTERRUPT_DC, // UNUSED
    VECTOR_INTERNAL_INTERRUPT_E0, // 532:ADI, 510:SCI2 ERI
    VECTOR_INTERNAL_INTERRUPT_E4, // 510:SCI2 RXI
    VECTOR_INTERNAL_INTERRUPT_E8, // 510:SCI2 TXI
    VECTOR_INTERNAL_INTERRUPT_EC, // 510:UNUSED
    VECTOR_INTERNAL_INTERRUPT_F0, // 510:ADI
};


static const int ROM2_SIZE = 0x100000;
static const int SRAM_SIZE = 0x10000;


struct MCU {
    void ProgGen_Prepare(const uint8_t *rom2_data, size_t rom2_size);
    uint32_t* ProgGen_Generate(uint8_t group, uint8_t type, uint8_t params[21], const uint8_t * saveStateData);

    struct mcu_core {
        uint16_t r[8];
        uint16_t pc;
        uint16_t sr;
        uint8_t cp, dp, ep, tp, br;
        uint8_t sleep;
        uint8_t ex_ignore;
        int32_t exception_pending;
        uint8_t interrupt_pending[INTERRUPT_SOURCE_MAX];
        uint8_t trapa_pending[16];
        uint64_t cycles;
    };

    mcu_core mcu;

    uint8_t rom2[ROM2_SIZE];
    uint8_t sram[SRAM_SIZE];

    int rom2_mask = ROM2_SIZE - 1;

    bool done = false;

    uint32_t lsp_temp[0x200] = {0};
    uint8_t lsp_regs[16] = {0};

    uint8_t dev_register[0x80];
    uint8_t dev_IRQCR; // 510 only

    int ga_int[8];
    int ga_int_enable = 0;
    int ga_int_trigger = 0;
    int ga_lcd_counter = 0;

    uint8_t io_sd = 0x00;

    bool initial = true;
    uint32_t mcu_button_pressed = 0;

    int adf_rd = 0;

    int ssr_rd = 0;

    uint64_t encoder_int_delay;
    uint64_t lcd_delay;
    bool lcd_ready;

    inline uint32_t MCU_GetAddress(uint8_t page, uint16_t address) {
        return (page << 16) + address;
    }
    
    inline uint8_t MCU_ReadCode(void) {
        return MCU_Read(MCU_GetAddress(mcu.cp, mcu.pc), true);
    }
    
    inline uint8_t MCU_ReadCodeAdvance(void) {
        uint8_t ret = MCU_ReadCode();
        mcu.pc++;
        return ret;
    }
    
    inline void MCU_SetRegisterByte(uint8_t reg, uint8_t val)
    {
        mcu.r[reg] = val;
    }
    
    inline uint32_t MCU_GetVectorAddress(uint32_t vector)
    {
        return MCU_Read32(vector * 4, true);
    }
    
    inline uint32_t MCU_GetPageForRegister(uint32_t reg)
    {
        if (reg >= 6)
            return mcu.tp;
        else if (reg >= 4)
            return mcu.ep;
        return mcu.dp;
    }
    
    inline void MCU_ControlRegisterWrite(uint32_t reg, uint32_t siz, uint32_t data)
    {
        if (siz)
        {
            if (reg == 0)
            {
                mcu.sr = data;
                mcu.sr &= sr_mask;
            }
            else if (reg == 5) // FIXME: undocumented
            {
                mcu.dp = data & 0xff;
            }
            else if (reg == 4) // FIXME: undocumented
            {
                mcu.ep = data & 0xff;
            }
            else if (reg == 3) // FIXME: undocumented
            {
                mcu.br = data & 0xff;
            }
            else if (reg == 1) // FIXME: undocumented
            {
                mcu.sr = data;
            }
            else
            {
                MCU_ErrorTrap();
            }
        }
        else
        {
            if (reg == 1)
            {
                mcu.sr &= ~0xff;
                mcu.sr |= data & 0xff;
                mcu.sr &= sr_mask;
            }
            else if (reg == 3)
            {
                mcu.br = data;
            }
            else if (reg == 4)
            {
                mcu.ep = data;
            }
            else if (reg == 5)
            {
                mcu.dp = data;
            }
            else if (reg == 7)
            {
                mcu.tp = data;
            }
            else
            {
                MCU_ErrorTrap();
            }
        }
    }
    
    inline uint32_t MCU_ControlRegisterRead(uint32_t reg, uint32_t siz)
    {
        uint32_t ret = 0;
        if (siz)
        {
            if (reg == 0)
            {
                ret = mcu.sr & sr_mask;
            }
            else if (reg == 5) // FIXME: undocumented
            {
                ret = mcu.dp | (mcu.dp << 8);
            }
            else if (reg == 4) // FIXME: undocumented
            {
                ret = mcu.ep | (mcu.ep << 8);
            }
            else if (reg == 3) // FIXME: undocumented
            {
                ret = mcu.br | (mcu.br << 8);
            }
            else if (reg == 1) // FIXME: undocumented
            {
                ret = mcu.sr;
            }
            else
            {
                MCU_ErrorTrap();
            }
            ret &= 0xffff;
        }
        else
        {
            if (reg == 1)
            {
                ret = mcu.sr & sr_mask;
            }
            else if (reg == 3)
            {
                ret = mcu.br;
            }
            else if (reg == 4)
            {
                ret = mcu.ep;
            }
            else if (reg == 5)
            {
                ret = mcu.dp;
            }
            else if (reg == 7)
            {
                ret = mcu.tp;
            }
            else
            {
                MCU_ErrorTrap();
            }
            ret &= 0xff;
        }
        return ret;
    }
    
    inline void MCU_SetStatus(uint32_t condition, uint32_t mask)
    {
        if (condition)
            mcu.sr |= mask;
        else
            mcu.sr &= ~mask;
    }
    
    inline void MCU_PushStack(uint16_t data)
    {
        if (mcu.r[7] & 1)
            MCU_Interrupt_Exception(EXCEPTION_SOURCE_ADDRESS_ERROR);
        mcu.r[7] -= 2;
        MCU_Write16(mcu.tp << 16 | mcu.r[7], data);
    }
    
    inline uint16_t MCU_PopStack(void)
    {
        uint16_t ret;
        if (mcu.r[7] & 1)
            MCU_Interrupt_Exception(EXCEPTION_SOURCE_ADDRESS_ERROR);
        ret = MCU_Read16(mcu.tp << 16 | mcu.r[7], true);
        mcu.r[7] += 2;
        return ret;
    }

    uint8_t MCU_Read(uint32_t address, bool code);
    uint16_t MCU_Read16(uint32_t address, bool code);
    uint32_t MCU_Read32(uint32_t address, bool code);
    void MCU_Write(uint32_t address, uint8_t value);
    void MCU_Write16(uint32_t address, uint16_t value);

    void MCU_ErrorTrap(void);
    
    void loadState(const uint8_t * saveStateData);

    void MCU_DeviceWrite_532(uint32_t address, uint8_t data);
    uint8_t MCU_DeviceRead_532(uint32_t address);
    void MCU_DeviceWrite_510(uint32_t address, uint8_t value);
    uint8_t MCU_DeviceRead_510(uint32_t address);
    void MCU_DeviceReset(void);
    void MCU_ReadInstruction(void);
    void MCU_Reset(void);
    void MCU_Run();
    void MCU_GA_SetGAInt(int line, int value);


    // ========================
    // TIMER

    struct frt_t {
        uint8_t tcr;
        uint8_t tcsr;
        uint16_t frc;
        uint16_t ocra;
        uint16_t ocrb;
        uint16_t icr;
        uint8_t status_rd;
    };
    
    struct mcu_timer_t {
        uint8_t tcr;
        uint8_t tcsr;
        uint8_t tcora;
        uint8_t tcorb;
        uint8_t tcnt;
        uint8_t status_rd;
    };
    
    uint64_t timer_cycles;
    uint8_t timer_tempreg;
    frt_t frt[3];
    mcu_timer_t timer;
    uint8_t dev_WDT_TCSR;
    uint8_t dev_WDT_TCNT;
    
    void TIMER_Reset();
    
    void TIMER_Write(uint32_t address, uint8_t data);
    uint8_t TIMER_Read(uint32_t address);
    void TIMER_Clock(uint64_t cycles);
    
    void TIMER2_Write(uint32_t address, uint8_t data);
    uint8_t TIMER_Read2(uint32_t address);


    // ========================
    // INTERRUPT
    
    void MCU_Interrupt_Start(int32_t mask);
    void MCU_Interrupt_StartVector(uint32_t vector, int32_t mask);
    void MCU_Interrupt_SetRequest(uint32_t interrupt, uint32_t value);
    void MCU_Interrupt_Exception(uint32_t exception);
    void MCU_Interrupt_TRAPA(uint32_t vector);
    void MCU_Interrupt_Handle(void);


    
    // ========================
    // OPCODES

    // void (*MCU_Operand_Table[256])(uint8_t operand);
    // void (*MCU_Opcode_Table[32])(uint8_t opcode, uint8_t opcode_reg);

    int32_t MCU_SUB_Common(int32_t t1, int32_t t2, int32_t c_bit, uint32_t siz);
    int32_t MCU_ADD_Common(int32_t t1, int32_t t2, int32_t c_bit, uint32_t siz);
    void MCU_Operand_Nop(uint8_t operand);
    void MCU_Operand_Sleep(uint8_t operand);
    void MCU_Operand_NotImplemented(uint8_t operand);
    void MCU_LDM(uint8_t operand);
    void MCU_STM(uint8_t operand);
    void MCU_TRAPA(uint8_t operand);
    void MCU_LINK(uint8_t operand);
    void MCU_UNLK(uint8_t operand);
    void MCU_Jump_PJSR(uint8_t operand);
    void MCU_Jump_JSR(uint8_t operand);
    void MCU_Jump_RTE(uint8_t operand);
    void MCU_Jump_Bcc(uint8_t operand);
    void MCU_Jump_RTS(uint8_t operand);
    void MCU_Jump_RTD(uint8_t operand);
    void MCU_Jump_JMP(uint8_t operand);
    void MCU_Jump_BSR(uint8_t operand);
    void MCU_Jump_PJMP(uint8_t operand);
    uint32_t MCU_Operand_Read(void);
    void MCU_Operand_Write(uint32_t data);
    void MCU_Operand_General(uint8_t operand);
    void MCU_SetStatusCommon(uint32_t val, uint32_t siz);
    void MCU_Opcode_Short_NotImplemented(uint8_t opcode);
    void MCU_Opcode_Short_MOVE(uint8_t opcode);
    void MCU_Opcode_Short_MOVI(uint8_t opcode);
    void MCU_Opcode_Short_MOVF(uint8_t opcode);
    void MCU_Opcode_Short_MOVL(uint8_t opcode);
    void MCU_Opcode_Short_MOVS(uint8_t opcode);
    void MCU_Opcode_Short_CMP(uint8_t opcode);
    void MCU_Opcode_NotImplemented(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_MOVG_Immediate(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BSET_ORC(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BCLR_ANDC(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BTST(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_CLR(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_LDC(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_STC(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BSET(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BCLR(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_MOVG(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BTSTI(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_BNOTI(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_OR(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_CMP(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_ADDQ(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_ADD(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_SUB(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_SUBS(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_AND(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_SHLR(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_MULXU(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_DIVXU(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_ADDS(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_XOR(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_ADDX(uint8_t opcode, uint8_t opcode_reg);
    void MCU_Opcode_SUBX(uint8_t opcode, uint8_t opcode_reg);

    using OperandFunc = void (MCU::*)(uint8_t operand);
    static constexpr OperandFunc MCU_Operand_Table[256] = {
        &MCU::MCU_Operand_Nop, // 00
        &MCU::MCU_Jump_JMP, // 01
        &MCU::MCU_LDM, // 02
        &MCU::MCU_Jump_PJSR, // 03
        &MCU::MCU_Operand_General, // 04
        &MCU::MCU_Operand_General, // 05
        &MCU::MCU_Jump_JMP, // 06
        &MCU::MCU_Jump_JMP, // 07
        &MCU::MCU_TRAPA, // 08
        &MCU::MCU_Operand_NotImplemented, // 09
        &MCU::MCU_Jump_RTE, // 0A
        &MCU::MCU_Operand_NotImplemented, // 0B
        &MCU::MCU_Operand_General, // 0C
        &MCU::MCU_Operand_General, // 0D
        &MCU::MCU_Jump_BSR, // 0E
        &MCU::MCU_UNLK, // 0F
        &MCU::MCU_Jump_JMP, // 10
        &MCU::MCU_Jump_JMP, // 11
        &MCU::MCU_STM, // 12
        &MCU::MCU_Jump_PJMP, // 13
        &MCU::MCU_Jump_RTD, // 14
        &MCU::MCU_Operand_General, // 15
        &MCU::MCU_Operand_NotImplemented, // 16
        &MCU::MCU_LINK, // 17
        &MCU::MCU_Jump_JSR, // 18
        &MCU::MCU_Jump_RTS, // 19
        &MCU::MCU_Operand_Sleep, // 1A
        &MCU::MCU_Operand_NotImplemented, // 1B
        &MCU::MCU_Jump_RTD, // 1C
        &MCU::MCU_Operand_General, // 1D
        &MCU::MCU_Jump_BSR, // 1E
        &MCU::MCU_LINK, // 1F
        &MCU::MCU_Jump_Bcc, // 20
        &MCU::MCU_Jump_Bcc, // 21
        &MCU::MCU_Jump_Bcc, // 22
        &MCU::MCU_Jump_Bcc, // 23
        &MCU::MCU_Jump_Bcc, // 24
        &MCU::MCU_Jump_Bcc, // 25
        &MCU::MCU_Jump_Bcc, // 26
        &MCU::MCU_Jump_Bcc, // 27
        &MCU::MCU_Jump_Bcc, // 28
        &MCU::MCU_Jump_Bcc, // 29
        &MCU::MCU_Jump_Bcc, // 2A
        &MCU::MCU_Jump_Bcc, // 2B
        &MCU::MCU_Jump_Bcc, // 2C
        &MCU::MCU_Jump_Bcc, // 2D
        &MCU::MCU_Jump_Bcc, // 2E
        &MCU::MCU_Jump_Bcc, // 2F
        &MCU::MCU_Jump_Bcc, // 30
        &MCU::MCU_Jump_Bcc, // 31
        &MCU::MCU_Jump_Bcc, // 32
        &MCU::MCU_Jump_Bcc, // 33
        &MCU::MCU_Jump_Bcc, // 34
        &MCU::MCU_Jump_Bcc, // 35
        &MCU::MCU_Jump_Bcc, // 36
        &MCU::MCU_Jump_Bcc, // 37
        &MCU::MCU_Jump_Bcc, // 38
        &MCU::MCU_Jump_Bcc, // 39
        &MCU::MCU_Jump_Bcc, // 3A
        &MCU::MCU_Jump_Bcc, // 3B
        &MCU::MCU_Jump_Bcc, // 3C
        &MCU::MCU_Jump_Bcc, // 3D
        &MCU::MCU_Jump_Bcc, // 3E
        &MCU::MCU_Jump_Bcc, // 3F
        &MCU::MCU_Opcode_Short_CMP, // 40
        &MCU::MCU_Opcode_Short_CMP, // 41
        &MCU::MCU_Opcode_Short_CMP, // 42
        &MCU::MCU_Opcode_Short_CMP, // 43
        &MCU::MCU_Opcode_Short_CMP, // 44
        &MCU::MCU_Opcode_Short_CMP, // 45
        &MCU::MCU_Opcode_Short_CMP, // 46
        &MCU::MCU_Opcode_Short_CMP, // 47
        &MCU::MCU_Opcode_Short_CMP, // 48
        &MCU::MCU_Opcode_Short_CMP, // 49
        &MCU::MCU_Opcode_Short_CMP, // 4A
        &MCU::MCU_Opcode_Short_CMP, // 4B
        &MCU::MCU_Opcode_Short_CMP, // 4C
        &MCU::MCU_Opcode_Short_CMP, // 4D
        &MCU::MCU_Opcode_Short_CMP, // 4E
        &MCU::MCU_Opcode_Short_CMP, // 4F
        &MCU::MCU_Opcode_Short_MOVE, // 50
        &MCU::MCU_Opcode_Short_MOVE, // 51
        &MCU::MCU_Opcode_Short_MOVE, // 52
        &MCU::MCU_Opcode_Short_MOVE, // 53
        &MCU::MCU_Opcode_Short_MOVE, // 54
        &MCU::MCU_Opcode_Short_MOVE, // 55
        &MCU::MCU_Opcode_Short_MOVE, // 56
        &MCU::MCU_Opcode_Short_MOVE, // 57
        &MCU::MCU_Opcode_Short_MOVI, // 58
        &MCU::MCU_Opcode_Short_MOVI, // 59
        &MCU::MCU_Opcode_Short_MOVI, // 5A
        &MCU::MCU_Opcode_Short_MOVI, // 5B
        &MCU::MCU_Opcode_Short_MOVI, // 5C
        &MCU::MCU_Opcode_Short_MOVI, // 5D
        &MCU::MCU_Opcode_Short_MOVI, // 5E
        &MCU::MCU_Opcode_Short_MOVI, // 5F
        &MCU::MCU_Opcode_Short_MOVL, // 60
        &MCU::MCU_Opcode_Short_MOVL, // 61
        &MCU::MCU_Opcode_Short_MOVL, // 62
        &MCU::MCU_Opcode_Short_MOVL, // 63
        &MCU::MCU_Opcode_Short_MOVL, // 64
        &MCU::MCU_Opcode_Short_MOVL, // 65
        &MCU::MCU_Opcode_Short_MOVL, // 66
        &MCU::MCU_Opcode_Short_MOVL, // 67
        &MCU::MCU_Opcode_Short_MOVL, // 68
        &MCU::MCU_Opcode_Short_MOVL, // 69
        &MCU::MCU_Opcode_Short_MOVL, // 6A
        &MCU::MCU_Opcode_Short_MOVL, // 6B
        &MCU::MCU_Opcode_Short_MOVL, // 6C
        &MCU::MCU_Opcode_Short_MOVL, // 6D
        &MCU::MCU_Opcode_Short_MOVL, // 6E
        &MCU::MCU_Opcode_Short_MOVL, // 6F
        &MCU::MCU_Opcode_Short_MOVS, // 70
        &MCU::MCU_Opcode_Short_MOVS, // 71
        &MCU::MCU_Opcode_Short_MOVS, // 72
        &MCU::MCU_Opcode_Short_MOVS, // 73
        &MCU::MCU_Opcode_Short_MOVS, // 74
        &MCU::MCU_Opcode_Short_MOVS, // 75
        &MCU::MCU_Opcode_Short_MOVS, // 76
        &MCU::MCU_Opcode_Short_MOVS, // 77
        &MCU::MCU_Opcode_Short_MOVS, // 78
        &MCU::MCU_Opcode_Short_MOVS, // 79
        &MCU::MCU_Opcode_Short_MOVS, // 7A
        &MCU::MCU_Opcode_Short_MOVS, // 7B
        &MCU::MCU_Opcode_Short_MOVS, // 7C
        &MCU::MCU_Opcode_Short_MOVS, // 7D
        &MCU::MCU_Opcode_Short_MOVS, // 7E
        &MCU::MCU_Opcode_Short_MOVS, // 7F
        &MCU::MCU_Opcode_Short_MOVF, // 80
        &MCU::MCU_Opcode_Short_MOVF, // 81
        &MCU::MCU_Opcode_Short_MOVF, // 82
        &MCU::MCU_Opcode_Short_MOVF, // 83
        &MCU::MCU_Opcode_Short_MOVF, // 84
        &MCU::MCU_Opcode_Short_MOVF, // 85
        &MCU::MCU_Opcode_Short_MOVF, // 86
        &MCU::MCU_Opcode_Short_MOVF, // 87
        &MCU::MCU_Opcode_Short_MOVF, // 88
        &MCU::MCU_Opcode_Short_MOVF, // 89
        &MCU::MCU_Opcode_Short_MOVF, // 8A
        &MCU::MCU_Opcode_Short_MOVF, // 8B
        &MCU::MCU_Opcode_Short_MOVF, // 8C
        &MCU::MCU_Opcode_Short_MOVF, // 8D
        &MCU::MCU_Opcode_Short_MOVF, // 8E
        &MCU::MCU_Opcode_Short_MOVF, // 8F
        &MCU::MCU_Opcode_Short_MOVF, // 90
        &MCU::MCU_Opcode_Short_MOVF, // 91
        &MCU::MCU_Opcode_Short_MOVF, // 92
        &MCU::MCU_Opcode_Short_MOVF, // 93
        &MCU::MCU_Opcode_Short_MOVF, // 94
        &MCU::MCU_Opcode_Short_MOVF, // 95
        &MCU::MCU_Opcode_Short_MOVF, // 96
        &MCU::MCU_Opcode_Short_MOVF, // 97
        &MCU::MCU_Opcode_Short_MOVF, // 98
        &MCU::MCU_Opcode_Short_MOVF, // 99
        &MCU::MCU_Opcode_Short_MOVF, // 9A
        &MCU::MCU_Opcode_Short_MOVF, // 9B
        &MCU::MCU_Opcode_Short_MOVF, // 9C
        &MCU::MCU_Opcode_Short_MOVF, // 9D
        &MCU::MCU_Opcode_Short_MOVF, // 9E
        &MCU::MCU_Opcode_Short_MOVF, // 9F
        &MCU::MCU_Operand_General, // A0
        &MCU::MCU_Operand_General, // A1
        &MCU::MCU_Operand_General, // A2
        &MCU::MCU_Operand_General, // A3
        &MCU::MCU_Operand_General, // A4
        &MCU::MCU_Operand_General, // A5
        &MCU::MCU_Operand_General, // A6
        &MCU::MCU_Operand_General, // A7
        &MCU::MCU_Operand_General, // A8
        &MCU::MCU_Operand_General, // A9
        &MCU::MCU_Operand_General, // AA
        &MCU::MCU_Operand_General, // AB
        &MCU::MCU_Operand_General, // AC
        &MCU::MCU_Operand_General, // AD
        &MCU::MCU_Operand_General, // AE
        &MCU::MCU_Operand_General, // AF
        &MCU::MCU_Operand_General, // B0
        &MCU::MCU_Operand_General, // B1
        &MCU::MCU_Operand_General, // B2
        &MCU::MCU_Operand_General, // B3
        &MCU::MCU_Operand_General, // B4
        &MCU::MCU_Operand_General, // B5
        &MCU::MCU_Operand_General, // B6
        &MCU::MCU_Operand_General, // B7
        &MCU::MCU_Operand_General, // B8
        &MCU::MCU_Operand_General, // B9
        &MCU::MCU_Operand_General, // BA
        &MCU::MCU_Operand_General, // BB
        &MCU::MCU_Operand_General, // BC
        &MCU::MCU_Operand_General, // BD
        &MCU::MCU_Operand_General, // BE
        &MCU::MCU_Operand_General, // BF
        &MCU::MCU_Operand_General, // C0
        &MCU::MCU_Operand_General, // C1
        &MCU::MCU_Operand_General, // C2
        &MCU::MCU_Operand_General, // C3
        &MCU::MCU_Operand_General, // C4
        &MCU::MCU_Operand_General, // C5
        &MCU::MCU_Operand_General, // C6
        &MCU::MCU_Operand_General, // C7
        &MCU::MCU_Operand_General, // C8
        &MCU::MCU_Operand_General, // C9
        &MCU::MCU_Operand_General, // CA
        &MCU::MCU_Operand_General, // CB
        &MCU::MCU_Operand_General, // CC
        &MCU::MCU_Operand_General, // CD
        &MCU::MCU_Operand_General, // CE
        &MCU::MCU_Operand_General, // CF
        &MCU::MCU_Operand_General, // D0
        &MCU::MCU_Operand_General, // D1
        &MCU::MCU_Operand_General, // D2
        &MCU::MCU_Operand_General, // D3
        &MCU::MCU_Operand_General, // D4
        &MCU::MCU_Operand_General, // D5
        &MCU::MCU_Operand_General, // D6
        &MCU::MCU_Operand_General, // D7
        &MCU::MCU_Operand_General, // D8
        &MCU::MCU_Operand_General, // D9
        &MCU::MCU_Operand_General, // DA
        &MCU::MCU_Operand_General, // DB
        &MCU::MCU_Operand_General, // DC
        &MCU::MCU_Operand_General, // DD
        &MCU::MCU_Operand_General, // DE
        &MCU::MCU_Operand_General, // DF
        &MCU::MCU_Operand_General, // E0
        &MCU::MCU_Operand_General, // E1
        &MCU::MCU_Operand_General, // E2
        &MCU::MCU_Operand_General, // E3
        &MCU::MCU_Operand_General, // E4
        &MCU::MCU_Operand_General, // E5
        &MCU::MCU_Operand_General, // E6
        &MCU::MCU_Operand_General, // E7
        &MCU::MCU_Operand_General, // E8
        &MCU::MCU_Operand_General, // E9
        &MCU::MCU_Operand_General, // EA
        &MCU::MCU_Operand_General, // EB
        &MCU::MCU_Operand_General, // EC
        &MCU::MCU_Operand_General, // ED
        &MCU::MCU_Operand_General, // EE
        &MCU::MCU_Operand_General, // EF
        &MCU::MCU_Operand_General, // F0
        &MCU::MCU_Operand_General, // F1
        &MCU::MCU_Operand_General, // F2
        &MCU::MCU_Operand_General, // F3
        &MCU::MCU_Operand_General, // F4
        &MCU::MCU_Operand_General, // F5
        &MCU::MCU_Operand_General, // F6
        &MCU::MCU_Operand_General, // F7
        &MCU::MCU_Operand_General, // F8
        &MCU::MCU_Operand_General, // F9
        &MCU::MCU_Operand_General, // FA
        &MCU::MCU_Operand_General, // FB
        &MCU::MCU_Operand_General, // FC
        &MCU::MCU_Operand_General, // FD
        &MCU::MCU_Operand_General, // FE
        &MCU::MCU_Operand_General, // FF
    };
    
    using OpcodeFunc = void (MCU::*)(uint8_t opcode, uint8_t opcode_reg);
    static constexpr OpcodeFunc MCU_Opcode_Table[32] = {
        &MCU::MCU_Opcode_MOVG_Immediate, // 00
        &MCU::MCU_Opcode_ADDQ, // 01
        &MCU::MCU_Opcode_CLR, // 02
        &MCU::MCU_Opcode_SHLR, // 03
        &MCU::MCU_Opcode_ADD, // 04
        &MCU::MCU_Opcode_ADDS, // 05
        &MCU::MCU_Opcode_SUB, // 06
        &MCU::MCU_Opcode_SUBS, // 07
        &MCU::MCU_Opcode_OR, // 08
        &MCU::MCU_Opcode_BSET_ORC, // 09
        &MCU::MCU_Opcode_AND, // 0A
        &MCU::MCU_Opcode_BCLR_ANDC, // 0B
        &MCU::MCU_Opcode_XOR, // 0C
        &MCU::MCU_Opcode_NotImplemented, // 0D
        &MCU::MCU_Opcode_CMP, // 0E
        &MCU::MCU_Opcode_BTST, // 0F
        &MCU::MCU_Opcode_MOVG, // 10
        &MCU::MCU_Opcode_LDC, // 11
        &MCU::MCU_Opcode_MOVG, // 12
        &MCU::MCU_Opcode_STC, // 13
        &MCU::MCU_Opcode_ADDX, // 14
        &MCU::MCU_Opcode_MULXU, // 15
        &MCU::MCU_Opcode_SUBX, // 16
        &MCU::MCU_Opcode_DIVXU, // 17
        &MCU::MCU_Opcode_BSET, // 18
        &MCU::MCU_Opcode_BSET, // 19
        &MCU::MCU_Opcode_BCLR, // 1A
        &MCU::MCU_Opcode_BCLR, // 1B
        &MCU::MCU_Opcode_BNOTI, // 1C
        &MCU::MCU_Opcode_BNOTI, // 1D
        &MCU::MCU_Opcode_BTSTI, // 1E
        &MCU::MCU_Opcode_BTSTI, // 1F
    };
};
