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
#include <stdio.h>
#include <string.h>
#include "mcu.h"

void MCU::MCU_Interrupt_Start(int32_t mask)
{
    MCU_PushStack(mcu.pc);
    MCU_PushStack(mcu.cp);
    MCU_PushStack(mcu.sr);
    mcu.sr &= ~STATUS_T;
    if (mask >= 0)
    {
        mcu.sr &= ~STATUS_INT_MASK;
        mcu.sr |= mask << 8;
    }
    mcu.sleep = 0;
}

void MCU::MCU_Interrupt_SetRequest(uint32_t interrupt, uint32_t value)
{
    mcu.interrupt_pending[interrupt] = value;
}

void MCU::MCU_Interrupt_Exception(uint32_t exception)
{
    mcu.exception_pending = exception;
}

void MCU::MCU_Interrupt_TRAPA(uint32_t vector)
{
    mcu.trapa_pending[vector] = 1;
}

void MCU::MCU_Interrupt_StartVector(uint32_t vector, int32_t mask)
{
    uint32_t address = MCU_GetVectorAddress(vector);
    MCU_Interrupt_Start(mask);
    mcu.cp = address >> 16;
    mcu.pc = address;
}

void MCU::MCU_Interrupt_Handle(void)
{
    uint32_t i;
    for (i = 0; i < 16; i++)
    {
        if (mcu.trapa_pending[i])
        {
            mcu.trapa_pending[i] = 0;
            MCU_Interrupt_StartVector(VECTOR_TRAPA_0 + i, -1);
            return;
        }
    }
    if (mcu.exception_pending >= 0)
    {
        switch (mcu.exception_pending)
        {
            case EXCEPTION_SOURCE_ADDRESS_ERROR:
                MCU_Interrupt_StartVector(VECTOR_ADDRESS_ERROR, -1);
                break;
            case EXCEPTION_SOURCE_INVALID_INSTRUCTION:
                MCU_Interrupt_StartVector(VECTOR_INVALID_INSTRUCTION, -1);
                break;
            case EXCEPTION_SOURCE_TRACE:
                MCU_Interrupt_StartVector(VECTOR_TRACE, -1);
                break;

        }
        mcu.exception_pending = -1;
        return;
    }
    if (mcu.interrupt_pending[INTERRUPT_SOURCE_NMI])
    {
        MCU_Interrupt_StartVector(VECTOR_NMI, 7);
        return;
    }
    uint32_t mask = (mcu.sr >> 8) & 7;
    for (i = INTERRUPT_SOURCE_NMI + 1; i < INTERRUPT_SOURCE_MAX; i++)
    {
        int32_t vector = -1;
        int32_t level = 0;
        if (!mcu.interrupt_pending[i])
            continue;
        switch (i)
        {
            case INTERRUPT_SOURCE_IRQ0:
                if ((dev_IRQCR & 0b0001) == 0)
                    continue;
                vector = VECTOR_INTERRUPT_80;
                level = (dev_register[DEV_IPRA] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_WDT:
                vector = VECTOR_INTERRUPT_84;
                level = (dev_register[DEV_IPRA] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_IRQ1:
                if ((dev_IRQCR & 0b0010) == 0)
                    continue;
                vector = VECTOR_INTERNAL_INTERRUPT_90;
                level = (dev_register[DEV_IPRA] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_IRQ2:
                if ((dev_IRQCR & 0b0100) == 0)
                    continue;
                vector = VECTOR_INTERNAL_INTERRUPT_94;
                level = (dev_register[DEV_IPRA] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_IRQ3:
                if ((dev_IRQCR & 0b1000) == 0)
                    continue;
                vector = VECTOR_INTERNAL_INTERRUPT_98;
                level = (dev_register[DEV_IPRA] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_FRT0_OCIA:
                vector = VECTOR_INTERNAL_INTERRUPT_A4;
                level = (dev_register[DEV_IPRB] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_FRT0_OCIB:
                vector = VECTOR_INTERNAL_INTERRUPT_A8;
                level = (dev_register[DEV_IPRB] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_FRT0_FOVI:
                vector = VECTOR_INTERNAL_INTERRUPT_AC;
                level = (dev_register[DEV_IPRB] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_FRT1_OCIA:
                vector = VECTOR_INTERNAL_INTERRUPT_B4;
                level = (dev_register[DEV_IPRB] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_FRT1_OCIB:
                vector = VECTOR_INTERNAL_INTERRUPT_B8;
                level = (dev_register[DEV_IPRB] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_FRT1_FOVI:
                vector = VECTOR_INTERNAL_INTERRUPT_BC;
                level = (dev_register[DEV_IPRB] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_TIMER_CMIA:
                vector = VECTOR_INTERNAL_INTERRUPT_C0;
                level = (dev_register[DEV_IPRC] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_TIMER_CMIB:
                vector = VECTOR_INTERNAL_INTERRUPT_C4;
                level = (dev_register[DEV_IPRC] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_TIMER_OVI:
                vector = VECTOR_INTERNAL_INTERRUPT_C8;
                level = (dev_register[DEV_IPRC] >> 4) & 7;
                break;
            case INTERRUPT_SOURCE_UART_RX:
                vector = VECTOR_INTERNAL_INTERRUPT_D4;
                level = (dev_register[DEV_IPRC] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_UART_TX:
                vector = VECTOR_INTERNAL_INTERRUPT_D8;
                level = (dev_register[DEV_IPRC] >> 0) & 7;
                break;
            case INTERRUPT_SOURCE_ANALOG:
                vector = VECTOR_INTERNAL_INTERRUPT_F0;
                level = (dev_register[DEV_IPRD] >> 0) & 7;
                break;
            default:
                break;
        }

        if ((int32_t)mask < level)
        {
            // mcu.interrupt_pending[INTERRUPT_SOURCE_NMI] = 0;
            MCU_Interrupt_StartVector(vector, level);
            return;
        }
    }
}

enum {
    REG_TCR = 0x00,
    REG_TCSR = 0x01,
    REG_FRCH = 0x02,
    REG_FRCL = 0x03,
    REG_OCRAH = 0x04,
    REG_OCRAL = 0x05,
    REG_OCRBH = 0x06,
    REG_OCRBL = 0x07,
    REG_ICRH = 0x08,
    REG_ICRL = 0x09,
};

void MCU::TIMER_Reset(void)
{
    timer_cycles = 0;
    timer_tempreg = 0;
    memset(frt, 0, sizeof(frt));
    memset(&timer, 0, sizeof(timer));
    dev_WDT_TCSR = 0x00;
    dev_WDT_TCNT = 0x00;
}

void MCU::TIMER_Write(uint32_t address, uint8_t data)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return;
    address &= 0x0f;
    frt_t *timer = &frt[t];
    switch (address)
    {
    case REG_TCR:
        timer->tcr = data;
        break;
    case REG_TCSR:
        timer->tcsr &= ~0xf;
        timer->tcsr |= data & 0xf;
        if ((data & 0x10) == 0 && (timer->status_rd & 0x10) != 0)
        {
            timer->tcsr &= ~0x10;
            timer->status_rd &= ~0x10;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_FOVI + t * 4, 0);
        }
        if ((data & 0x20) == 0 && (timer->status_rd & 0x20) != 0)
        {
            timer->tcsr &= ~0x20;
            timer->status_rd &= ~0x20;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_OCIA + t * 4, 0);
        }
        if ((data & 0x40) == 0 && (timer->status_rd & 0x40) != 0)
        {
            timer->tcsr &= ~0x40;
            timer->status_rd &= ~0x40;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_OCIB + t * 4, 0);
        }
        break;
    case REG_FRCH:
    case REG_OCRAH:
    case REG_OCRBH:
    case REG_ICRH:
        timer_tempreg = data;
        break;
    case REG_FRCL:
        timer->frc = (timer_tempreg << 8) | data;
        break;
    case REG_OCRAL:
        timer->ocra = (timer_tempreg << 8) | data;
        break;
    case REG_OCRBL:
        timer->ocrb = (timer_tempreg << 8) | data;
        break;
    case REG_ICRL:
        timer->icr = (timer_tempreg << 8) | data;
        break;
    }
}

uint8_t MCU::TIMER_Read(uint32_t address)
{
    uint32_t t = (address >> 4) - 1;
    if (t > 2)
        return 0xff;
    address &= 0x0f;
    frt_t *timer = &frt[t];
    switch (address)
    {
    case REG_TCR:
        return timer->tcr;
    case REG_TCSR:
    {
        uint8_t ret = timer->tcsr;
        timer->status_rd |= timer->tcsr & 0xf0;
        //timer->status_rd |= 0xf0;
        return ret;
    }
    case REG_FRCH:
        timer_tempreg = timer->frc & 0xff;
        return timer->frc >> 8;
    case REG_OCRAH:
        timer_tempreg = timer->ocra & 0xff;
        return timer->ocra >> 8;
    case REG_OCRBH:
        timer_tempreg = timer->ocrb & 0xff;
        return timer->ocrb >> 8;
    case REG_ICRH:
        timer_tempreg = timer->icr & 0xff;
        return timer->icr >> 8;
    case REG_FRCL:
    case REG_OCRAL:
    case REG_OCRBL:
    case REG_ICRL:
        return timer_tempreg;
    }
    return 0xff;
}

void MCU::TIMER2_Write(uint32_t address, uint8_t data)
{
    switch (address)
    {
    case DEV_TMR_TCR:
        timer.tcr = data;
        break;
    case DEV_TMR_TCSR:
        timer.tcsr &= ~0xf;
        timer.tcsr |= data & 0xf;
        if ((data & 0x20) == 0 && (timer.status_rd & 0x20) != 0)
        {
            timer.tcsr &= ~0x20;
            timer.status_rd &= ~0x20;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_OVI, 0);
        }
        if ((data & 0x40) == 0 && (timer.status_rd & 0x40) != 0)
        {
            timer.tcsr &= ~0x40;
            timer.status_rd &= ~0x40;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_CMIA, 0);
        }
        if ((data & 0x80) == 0 && (timer.status_rd & 0x80) != 0)
        {
            timer.tcsr &= ~0x80;
            timer.status_rd &= ~0x80;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_CMIB, 0);
        }
        break;
    case DEV_TMR_TCORA:
        timer.tcora = data;
        break;
    case DEV_TMR_TCORB:
        timer.tcorb = data;
        break;
    case DEV_TMR_TCNT:
        timer.tcnt = data;
        break;
    }
}
uint8_t MCU::TIMER_Read2(uint32_t address)
{
    switch (address)
    {
    case DEV_TMR_TCR:
        return timer.tcr;
    case DEV_TMR_TCSR:
    {
        uint8_t ret = timer.tcsr;
        timer.status_rd |= timer.tcsr & 0xe0;
        return ret;
    }
    case DEV_TMR_TCORA:
        return timer.tcora;
    case DEV_TMR_TCORB:
        return timer.tcorb;
    case DEV_TMR_TCNT:
        return timer.tcnt;
    }
    return 0xff;
}

void MCU::TIMER_Clock(uint64_t cycles)
{
    uint32_t i;
    while (timer_cycles*2 < cycles) // FIXME
    {
        for (i = 0; i < 2; i++)
        {
            frt_t *timer = &frt[i];

            switch (timer->tcr & 3)
            {
            case 0: // o / 4
                if (timer_cycles & 3)
                    continue;
                break;
            case 1: // o / 8
                if (timer_cycles & 7)
                    continue;
                break;
            case 2: // o / 32
                if (timer_cycles & 31)
                    continue;
                break;
            case 3: // ext (o / 2)
                if (timer_cycles & 3)
                    continue;
                break;
            }

            uint32_t value = timer->frc;
            uint32_t matcha = value == timer->ocra;
            uint32_t matchb = value == timer->ocrb;
            if ((timer->tcsr & 1) != 0 && matcha) // CCLRA
                value = 0;
            else
                value++;
            uint32_t of = (value >> 16) & 1;
            value &= 0xffff;
            timer->frc = value;

            // flags
            if (of)
                timer->tcsr |= 0x10;
            if (matcha)
                timer->tcsr |= 0x20;
            if (matchb)
                timer->tcsr |= 0x40;
            if ((timer->tcr & 0x10) != 0 && (timer->tcsr & 0x10) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_FOVI + i * 4, 1);
            if ((timer->tcr & 0x20) != 0 && (timer->tcsr & 0x20) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_OCIA + i * 4, 1);
            if ((timer->tcr & 0x40) != 0 && (timer->tcsr & 0x40) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_FRT0_OCIB + i * 4, 1);
        }

        int32_t timer_step = 0;

        switch (timer.tcr & 7)
        {
        case 0:
        case 4:
            break;
        case 1: // o / 8
            if ((timer_cycles & 7) == 0)
                timer_step = 1;
            break;
        case 2: // o / 64
            if ((timer_cycles & 63) == 0)
                timer_step = 1;
            break;
        case 3: // o / 1024
            if ((timer_cycles & 1023) == 0)
                timer_step = 1;
            break;
        case 5:
        case 6:
        case 7: // ext (o / 2)
            if ((timer_cycles & 1) == 0)
                timer_step = 1;
            break;
        }
        if (timer_step)
        {
            uint32_t value = timer.tcnt;
            uint32_t matcha = value == timer.tcora;
            uint32_t matchb = value == timer.tcorb;
            if ((timer.tcr & 24) == 8 && matcha)
                value = 0;
            else if ((timer.tcr & 24) == 16 && matchb)
                value = 0;
            else
                value++;
            uint32_t of = (value >> 8) & 1;
            value &= 0xff;
            timer.tcnt = value;

            // flags
            if (of)
                timer.tcsr |= 0x20;
            if (matcha)
                timer.tcsr |= 0x40;
            if (matchb)
                timer.tcsr |= 0x80;
            if ((timer.tcr & 0x20) != 0 && (timer.tcsr & 0x20) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_OVI, 1);
            if ((timer.tcr & 0x40) != 0 && (timer.tcsr & 0x40) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_CMIA, 1);
            if ((timer.tcr & 0x80) != 0 && (timer.tcsr & 0x80) != 0)
                MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_TIMER_CMIB, 1);
        }

        if ((dev_WDT_TCSR & 0x20) == 0)
        {
            dev_WDT_TCNT = 0;
        }
        else
        {
            int32_t wdt_step = 0;

            switch (dev_WDT_TCSR & 7)
            {
            case 0:  // o / 2
                if ((timer_cycles & 1) == 0)
                    wdt_step = 1;
                break;
            case 1: // o / 32
                if ((timer_cycles & 31) == 0)
                    wdt_step = 1;
                break;
            case 2: // o / 64
                if ((timer_cycles & 63) == 0)
                    wdt_step = 1;
                break;
            case 3: // o / 128
                if ((timer_cycles & 127) == 0)
                    wdt_step = 1;
                break;
            case 4: // o / 256
                if ((timer_cycles & 255) == 0)
                    wdt_step = 1;
                break;
            case 5: // o / 512
                if ((timer_cycles & 511) == 0)
                    wdt_step = 1;
                break;
            case 6: // o / 2048
                if ((timer_cycles & 2047) == 0)
                    wdt_step = 1;
                break;
            case 7: // o / 4096
                if ((timer_cycles & 4095) == 0)
                    wdt_step = 1;
                break;
            }
            if (wdt_step)
            {
                bool overflow = ((int)dev_WDT_TCNT + 1) > 0xff;
                dev_WDT_TCNT++;

                if (overflow)
                {
                    dev_WDT_TCSR |= 0x80;
                    if (overflow && (dev_WDT_TCSR & 0x40) == 0)
                    {
                        MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_WDT, 1);
                    }
                    else if (overflow)
                    {
                        MCU_Reset();
                    }
                }
            }
        }

        timer_cycles++;
    }
}


void MCU::MCU_ErrorTrap(void)
{
    printf("ERROR %.2x %.4x\n", mcu.cp, mcu.pc);
}

void MCU::MCU_DeviceWrite_532(uint32_t address, uint8_t data)
{
    address &= 0x7f;
    if (address >= 0x10 && address < 0x40)
    {
        TIMER_Write(address, data);
        return;
    }
    if (address >= 0x50 && address < 0x55)
    {
        TIMER2_Write(address, data);
        return;
    }
    switch (address)
    {
    case DEV_P1DDR: // P1DDR
        break;
    case DEV_P5DDR:
        break;
    case DEV_P6DDR:
        break;
    case DEV_P7DDR:
        break;
    case DEV_SCR:
        break;
    case DEV_WCR:
        break;
    case DEV_P9DDR:
        break;
    case DEV_RAME: // RAME
        break;
    case DEV_P1CR: // P1CR
        break;
    case DEV_DTEA:
        break;
    case DEV_DTEB:
        break;
    case DEV_DTEC:
        break;
    case DEV_DTED:
        break;
    case DEV_SMR:
        break;
    case DEV_BRR:
        break;
    case DEV_IPRA:
        break;
    case DEV_IPRB:
        break;
    case DEV_IPRC:
        break;
    case DEV_IPRD:
        break;
    case DEV_PWM1_DTR:
        break;
    case DEV_PWM1_TCR:
        break;
    case DEV_PWM2_DTR:
        break;
    case DEV_PWM2_TCR:
        break;
    case DEV_PWM3_DTR:
        break;
    case DEV_PWM3_TCR:
        break;
    case DEV_P7DR:
        break;
    case DEV_TMR_TCNT:
        break;
    case DEV_TMR_TCR:
        break;
    case DEV_TMR_TCSR:
        break;
    case DEV_TMR_TCORA:
        break;
    case DEV_TDR:
        break;
    case DEV_ADCSR:
    {
        dev_register[address] &= ~0x7f;
        dev_register[address] |= data & 0x7f;
        if ((data & 0x80) == 0 && adf_rd)
        {
            dev_register[address] &= ~0x80;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_ANALOG, 0);
        }
        if ((data & 0x40) == 0)
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_ANALOG, 0);
        return;
    }
    case DEV_SSR:
    {
        if ((data & 0x80) == 0 && (ssr_rd & 0x80) != 0)
        {
            dev_register[address] &= ~0x80;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_UART_TX, 0);
        }
        if ((data & 0x40) == 0 && (ssr_rd & 0x40) != 0)
        {
            dev_register[address] &= ~0x40;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_UART_RX, 0);
        }
        if ((data & 0x20) == 0 && (ssr_rd & 0x20) != 0)
        {
            dev_register[address] &= ~0x20;
        }
        if ((data & 0x10) == 0 && (ssr_rd & 0x10) != 0)
        {
            dev_register[address] &= ~0x10;
        }
        break;
    }
    default:
        address += 0;
        printf("%02x%04x: write dev %04x %02x\n", mcu.cp, mcu.pc, address, data);
        break;
    }
    dev_register[address] = data;
}

uint8_t MCU::MCU_DeviceRead_532(uint32_t address)
{
    address &= 0x7f;
    if (address >= 0x10 && address < 0x40)
    {
        return TIMER_Read(address);
    }
    if (address >= 0x50 && address < 0x55)
    {
        return TIMER_Read2(address);
    }
    switch (address)
    {
    case DEV_ADDRAH:
    case DEV_ADDRAL:
    case DEV_ADDRBH:
    case DEV_ADDRBL:
    case DEV_ADDRCH:
    case DEV_ADDRCL:
    case DEV_ADDRDH:
    case DEV_ADDRDL:
        return dev_register[address];
    case DEV_ADCSR:
        adf_rd = (dev_register[address] & 0x80) != 0;
        return dev_register[address];
    case DEV_SSR:
        ssr_rd = dev_register[address];
        return dev_register[address];
    case DEV_RDR:
        return 0xff;
    case 0x00:
        return 0xff;
    case DEV_P7DR:
    {
        uint8_t data = 0xff;
        uint32_t button_pressed = mcu_button_pressed;

        if (io_sd == 0b11111011)
            data &= ((button_pressed >> 0) & 0b11111) ^ 0xFF;
        if (io_sd == 0b11110111)
            data &= ((button_pressed >> 5) & 0b11111) ^ 0xFF;
        if (io_sd == 0b11101111)
            data &= ((button_pressed >> 10) & 0b1111) ^ 0xFF;

        data |= 0b10000000;
        return data;
    }
    case DEV_P9DR:
    {
        int cfg = 0;
        int dir = dev_register[DEV_P9DDR];

        int val = cfg & (dir ^ 0xff);
        val |= dev_register[DEV_P9DR] & dir;
        return val;
    }
    case DEV_SCR:
    case DEV_TDR:
    case DEV_SMR:
        return dev_register[address];
    case DEV_IPRC:
    case DEV_IPRD:
    case DEV_DTEC:
    case DEV_DTED:
    case DEV_FRT2_TCSR:
    case DEV_FRT1_TCSR:
    case DEV_FRT1_TCR:
    case DEV_FRT1_FRCH:
    case DEV_FRT1_FRCL:
    case DEV_FRT3_TCSR:
    case DEV_FRT3_OCRAH:
    case DEV_FRT3_OCRAL:
        return dev_register[address];
    }
    return dev_register[address];
}

void MCU::MCU_DeviceWrite_510(uint32_t address, uint8_t value)
{
    // Ports
         if (address == 0xfe80) printf("w P1DDR %x\n", value);
    else if (address == 0xfe81) printf("w P2DDR %x\n", value);
    else if (address == 0xfe82) printf("w P1DR %x\n", value);
    else if (address == 0xfe83) printf("w P2DR %x\n", value);
    else if (address == 0xfe84) printf("w P3DDR %x\n", value);
    else if (address == 0xfe85) printf("w P4DDR %x\n", value);
    else if (address == 0xfe86) ;// printf("w P3DR %x\n", value);
    else if (address == 0xfe87) printf("w P4DR %x\n", value);
    else if (address == 0xfe88) printf("w P5DDR %x\n", value);
    else if (address == 0xfe89) printf("w P6DDR %x\n", value);
    else if (address == 0xfe8a) printf("w P5DR %x\n", value);
    else if (address == 0xfe8b) printf("w P6DR %x\n", value);
    else if (address == 0xfe8d) printf("w P8DDR %x\n", value);
    else if (address == 0xfe8e) printf("w P7DR %x\n", value);
    else if (address == 0xfe8f) printf("w P8DR %x\n", value);

    // A/D
    else if (address == 0xfe90) MCU_DeviceWrite_532(DEV_ADDRAH, value);
    else if (address == 0xfe91) MCU_DeviceWrite_532(DEV_ADDRAL, value);
    else if (address == 0xfe92) MCU_DeviceWrite_532(DEV_ADDRBH, value);
    else if (address == 0xfe93) MCU_DeviceWrite_532(DEV_ADDRBL, value);
    else if (address == 0xfe94) MCU_DeviceWrite_532(DEV_ADDRCH, value);
    else if (address == 0xfe95) MCU_DeviceWrite_532(DEV_ADDRCL, value);
    else if (address == 0xfe96) MCU_DeviceWrite_532(DEV_ADDRDH, value);
    else if (address == 0xfe97) MCU_DeviceWrite_532(DEV_ADDRDL, value);
    else if (address == 0xfe98) MCU_DeviceWrite_532(DEV_ADCSR, value);
    else if (address == 0xfe98) MCU_DeviceWrite_532(DEV_ADCSR, value);
    else if (address == 0xfe99) printf("w ADCR %x\n", value);

    // FRT1
    else if (address == 0xfea0) {/*printf("w DEV_FRT1_TCR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_TCR, value);}
    else if (address == 0xfea1) {/*printf("w DEV_FRT1_TCSR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_TCSR, value);}
    else if (address == 0xfea2) {/*printf("w DEV_FRT1_FRCH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_FRCH, value);}
    else if (address == 0xfea3) {/*printf("w DEV_FRT1_FRCL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_FRCL, value);}
    else if (address == 0xfea4) {/*printf("w DEV_FRT1_OCRAH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_OCRAH, value);}
    else if (address == 0xfea5) {/*printf("w DEV_FRT1_OCRAL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_OCRAL, value);}
    else if (address == 0xfea6) {/*printf("w DEV_FRT1_OCRBH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_OCRBH, value);}
    else if (address == 0xfea7) {/*printf("w DEV_FRT1_OCRBL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT1_OCRBL, value);}
    else if (address == 0xfea8) printf("w FRT1_ICR_H %x\n", value);
    else if (address == 0xfea9) printf("w FRT1_ICR_L %x\n", value);

    // FRT2
    else if (address == 0xfeb0) {/*printf("w DEV_FRT2_TCR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_TCR, value);}
    else if (address == 0xfeb1) {/*printf("w DEV_FRT2_TCSR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_TCSR, value);}
    else if (address == 0xfeb2) {/*printf("w DEV_FRT2_FRCH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_FRCH, value);}
    else if (address == 0xfeb3) {/*printf("w DEV_FRT2_FRCL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_FRCL, value);}
    else if (address == 0xfeb4) {/*printf("w DEV_FRT2_OCRAH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_OCRAH, value);}
    else if (address == 0xfeb5) {/*printf("w DEV_FRT2_OCRAL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_OCRAL, value);}
    else if (address == 0xfeb6) {/*printf("w DEV_FRT2_OCRBH %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_OCRBH, value);}
    else if (address == 0xfeb7) {/*printf("w DEV_FRT2_OCRBL %02x\n", value);*/ MCU_DeviceWrite_532(DEV_FRT2_OCRBL, value);}
    else if (address == 0xfeb8) printf("w FRT2_ICR_H %x\n", value);
    else if (address == 0xfeb9) printf("w FRT2_ICR_L %x\n", value);

    // TMR
    else if (address == 0xfec0) {/*printf("w DEV_TMR_TCR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TMR_TCR, value);}
    else if (address == 0xfec1) {/*printf("w DEV_TMR_TCSR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TMR_TCSR, value);}
    else if (address == 0xfec2) {/*printf("w DEV_TMR_TCORA %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TMR_TCORA, value);}
    else if (address == 0xfec3) {/*printf("w DEV_TMR_TCORB %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TMR_TCORB, value);}
    else if (address == 0xfec4) {/*printf("w DEV_TMR_TCNT %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TMR_TCNT, value);}

    // SCI1
    else if (address == 0xfec8) {/*printf("w DEV_SMR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_SMR, value);}
    else if (address == 0xfec9) {/*printf("w DEV_BRR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_BRR, value);}
    else if (address == 0xfeca) {/*printf("w DEV_SCR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_SCR, value);}
    else if (address == 0xfecb) {/*printf("w DEV_TDR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_TDR, value);}
    else if (address == 0xfecc) {/*printf("w DEV_SSR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_SSR, value);}
    else if (address == 0xfecd) {/*printf("w DEV_RDR %02x\n", value);*/ MCU_DeviceWrite_532(DEV_RDR, value);}

    // SCI2
    else if (address == 0xfed0) printf("w SCI2 DEV_SMR %x\n", value);
    else if (address == 0xfed1) printf("w SCI2 DEV_BRR %x\n", value);
    else if (address == 0xfed2) printf("w SCI2 DEV_SCR %x\n", value);
    else if (address == 0xfed3) printf("w SCI2 DEV_TDR %x\n", value);
    else if (address == 0xfed4) printf("w SCI2 DEV_SSR %x\n", value);
    else if (address == 0xfed5) printf("w SCI2 DEV_RDR %x\n", value);

    // RFSHCR
    else if (address == 0xfed8) printf("w RFSHCR %x\n", value);

    // INTC
    else if (address == 0xff00) {/*printf("w DEV_IPRA %02x\n", value);*/ MCU_DeviceWrite_532(DEV_IPRA, value);}
    else if (address == 0xff01) {/*printf("w DEV_IPRB %02x\n", value);*/ MCU_DeviceWrite_532(DEV_IPRB, value);}
    else if (address == 0xff02) {/*printf("w DEV_IPRC %02x\n", value);*/ MCU_DeviceWrite_532(DEV_IPRC, value);}
    else if (address == 0xff03) {/*printf("w DEV_IPRD %02x\n", value);*/ MCU_DeviceWrite_532(DEV_IPRD, value);}
    else if (address == 0xff08) {/*printf("w DEV_DTEA %02x\n", value);*/ MCU_DeviceWrite_532(DEV_DTEA, value);}
    else if (address == 0xff09) {/*printf("w DEV_DTEB %02x\n", value);*/ MCU_DeviceWrite_532(DEV_DTEB, value);}
    else if (address == 0xff0a) {/*printf("w DEV_DTEC %02x\n", value);*/ MCU_DeviceWrite_532(DEV_DTEC, value);}
    else if (address == 0xff0b) {/*printf("w DEV_DTED %02x\n", value);*/ MCU_DeviceWrite_532(DEV_DTED, value);}

    // WTD is handled by MCU_Write16
    else if (address == 0xff10) printf("Unexpected write8 to WDT TCSR %x\n", value);
    else if (address == 0xff11) printf("Unexpected write8 to WDT TCNT %x\n", value);

    // WSC
    else if (address == 0xff14) printf("w WCR %x\n", value);

    // BSC
    else if (address == 0xff16) printf("w ARBT %x\n", value);
    else if (address == 0xff17) printf("w AR3T %x\n", value);

    else if (address == 0xff19) printf("w MDCR %x\n", value);
    else if (address == 0xff1a) printf("w SBYCR %x\n", value);
    else if (address == 0xff1b) printf("w BRCR %x\n", value);
    else if (address == 0xff1c) printf("w NMICR %x\n", value);
    else if (address == 0xff1d)
    {
        dev_IRQCR = value;
        printf("w IRQCR %x\n", value);
    }
    else if (address == 0xff1e) printf("w RSTCSR %x\n", value);
    else if (address == 0xff1f) printf("w RSTCSR WDT %x\n", value);

    else
        printf("%02x%04x: write dev %02x%04x %02x\n", mcu.cp, mcu.pc, 0, address, value);
}

uint8_t MCU::MCU_DeviceRead_510(uint32_t address)
{
    uint8_t ret = 0xff;

    // Ports
         if (address == 0xfe80) printf("r P1DDR\n");
    else if (address == 0xfe81) printf("r P2DDR\n");
    else if (address == 0xfe82) printf("r P1DR\n");
    else if (address == 0xfe83) printf("r P2DR\n");
    else if (address == 0xfe84) printf("r P3DDR\n");
    else if (address == 0xfe85) printf("r P4DDR\n");
    else if (address == 0xfe86) ;// printf("r P3DR\n");
    else if (address == 0xfe87) printf("r P4DR\n");
    else if (address == 0xfe88) printf("r P5DDR\n");
    else if (address == 0xfe89) printf("r P6DDR\n");
    else if (address == 0xfe8a) printf("r P5DR\n");
    else if (address == 0xfe8b) printf("r P6DR\n");
    else if (address == 0xfe8d) printf("r P8DDR\n");
    else if (address == 0xfe8e) printf("r P7DR\n");
    else if (address == 0xfe8f) printf("r P8DR\n");

    // A/D
    else if (address == 0xfe90) ret = MCU_DeviceRead_532(DEV_ADDRAH);
    else if (address == 0xfe91) ret = MCU_DeviceRead_532(DEV_ADDRAL);
    else if (address == 0xfe92) ret = MCU_DeviceRead_532(DEV_ADDRBH);
    else if (address == 0xfe93) ret = MCU_DeviceRead_532(DEV_ADDRBL);
    else if (address == 0xfe94) ret = MCU_DeviceRead_532(DEV_ADDRCH);
    else if (address == 0xfe95) ret = MCU_DeviceRead_532(DEV_ADDRCL);
    else if (address == 0xfe96) ret = MCU_DeviceRead_532(DEV_ADDRDH);
    else if (address == 0xfe97) ret = MCU_DeviceRead_532(DEV_ADDRDL);
    else if (address == 0xfe98) ret = MCU_DeviceRead_532(DEV_ADCSR);
    else if (address == 0xfe98) ret = MCU_DeviceRead_532(DEV_ADCSR);
    else if (address == 0xfe99) printf("r ADCR\n");

    // FRT1
    else if (address == 0xfea0) ret = MCU_DeviceRead_532(DEV_FRT1_TCR);
    else if (address == 0xfea1) ret = MCU_DeviceRead_532(DEV_FRT1_TCSR);
    else if (address == 0xfea2) ret = MCU_DeviceRead_532(DEV_FRT1_FRCH);
    else if (address == 0xfea3) ret = MCU_DeviceRead_532(DEV_FRT1_FRCL);
    else if (address == 0xfea4) ret = MCU_DeviceRead_532(DEV_FRT1_OCRAH);
    else if (address == 0xfea5) ret = MCU_DeviceRead_532(DEV_FRT1_OCRAL);
    else if (address == 0xfea6) ret = MCU_DeviceRead_532(DEV_FRT1_OCRBH);
    else if (address == 0xfea7) ret = MCU_DeviceRead_532(DEV_FRT1_OCRBL);
    else if (address == 0xfea8) printf("r FRT1_ICR_H\n");
    else if (address == 0xfea9) printf("r FRT1_ICR_L\n");

    // FRT2
    else if (address == 0xfeb0) ret = MCU_DeviceRead_532(DEV_FRT2_TCR);
    else if (address == 0xfeb1) ret = MCU_DeviceRead_532(DEV_FRT2_TCSR);
    else if (address == 0xfeb2) ret = MCU_DeviceRead_532(DEV_FRT2_FRCH);
    else if (address == 0xfeb3) ret = MCU_DeviceRead_532(DEV_FRT2_FRCL);
    else if (address == 0xfeb4) ret = MCU_DeviceRead_532(DEV_FRT2_OCRAH);
    else if (address == 0xfeb5) ret = MCU_DeviceRead_532(DEV_FRT2_OCRAL);
    else if (address == 0xfeb6) ret = MCU_DeviceRead_532(DEV_FRT2_OCRBH);
    else if (address == 0xfeb7) ret = MCU_DeviceRead_532(DEV_FRT2_OCRBL);
    else if (address == 0xfeb8) printf("r FRT2_ICR_H\n");
    else if (address == 0xfeb9) printf("r FRT2_ICR_L\n");

    // TMR
    else if (address == 0xfec0) ret = MCU_DeviceRead_532(DEV_TMR_TCR);
    else if (address == 0xfec1) ret = MCU_DeviceRead_532(DEV_TMR_TCSR);
    else if (address == 0xfec2) ret = MCU_DeviceRead_532(DEV_TMR_TCORA);
    else if (address == 0xfec3) ret = MCU_DeviceRead_532(DEV_TMR_TCORB);
    else if (address == 0xfec4) ret = MCU_DeviceRead_532(DEV_TMR_TCNT);

    // SCI1
    else if (address == 0xfec8) ret = MCU_DeviceRead_532(DEV_SMR);
    else if (address == 0xfec9) ret = MCU_DeviceRead_532(DEV_BRR);
    else if (address == 0xfeca) ret = MCU_DeviceRead_532(DEV_SCR);
    else if (address == 0xfecb) ret = MCU_DeviceRead_532(DEV_TDR);
    else if (address == 0xfecc) ret = MCU_DeviceRead_532(DEV_SSR);
    else if (address == 0xfecd) ret = MCU_DeviceRead_532(DEV_RDR);

    // SCI2
    else if (address == 0xfed0) printf("r SCI2 DEV_SMR\n");
    else if (address == 0xfed1) printf("r SCI2 DEV_BRR\n");
    else if (address == 0xfed2) printf("r SCI2 DEV_SCR\n");
    else if (address == 0xfed3) printf("r SCI2 DEV_TDR\n");
    else if (address == 0xfed4) printf("r SCI2 DEV_SSR\n");
    else if (address == 0xfed5) printf("r SCI2 DEV_RDR\n");

    // WDT
    else if (address == 0xff10)
    {
        ret = dev_WDT_TCSR;
        MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_WDT, 0);
        dev_WDT_TCSR &= ~0x80;
    }
    else if (address == 0xff11)
    {
        ret = dev_WDT_TCNT;
    }

    else if (address == 0xff1d)
    {
        printf("r IRQCR\n");
        ret = dev_IRQCR;
    }

    else
        printf("%02x%04x: read dev %02x%04x\n", mcu.cp, mcu.pc, 0, address);

    return ret;
}

void MCU::MCU_DeviceReset(void)
{
    dev_register[DEV_RAME] = 0x80;
    dev_register[DEV_SSR] = 0x80;

    TIMER_Reset();
}

uint8_t MCU::MCU_Read(uint32_t address, bool code)
{
    uint32_t address_full = address;
    uint8_t page = address >> 16;
    address &= 0xffff;
    uint8_t ret = 0xff;

    if (address == 0xfe87) // P4DR
        return 0x00;
    else if (address_full >= 0xfe80 && address_full <= 0xff1f)
        return MCU_DeviceRead_510(address);
    else if (address_full < 0x800000)
    {
        return rom2[address_full & 0xfffff];
    }
    else if (address_full < 0xc80000)
    {
        return sram[address_full & 0xffff];
    }
    else if (address_full >= 0xe00000 && address_full < 0xe7ffff)
    {
        // SUB CPU
        if (address == 0x00c0) // Version H
            ret = 0x01;
        else if (address == 0x00c1)  // Version L
            ret = 0x23;
        else if (address == 0x00dc)
        {
            ret = 0x00;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_IRQ2, 0);
        }
        else if (address == 0x00dd)
            ret = 0x00;
        else if (address == 0x00de)
            ret = 0x00;
        else if (address == 0x00df)
            ret = 0x00;
        else if (address == 0x00fd) // IPC Semaphore
            ret = 0x80;
        else if (address == 0x00fe) // Buttons
        {
            uint8_t data = 0xff;
            uint32_t button_pressed = mcu_button_pressed;

            if (io_sd == 0x1)
                data &= ((button_pressed >> 0) & 0xff) ^ 0xff;
            if (io_sd == 0x2)
                data &= ((button_pressed >> 8) & 0xff) ^ 0xff;
            if (io_sd == 0x4)
                data &= ((button_pressed >> 16) & 0xff) ^ 0xff;
            if (io_sd == 0x8)
                data &= ((button_pressed >> 24) & 0xff) ^ 0xff;

            ret = data;
        }
        return ret;
    }
    else if (address_full >= 0xe80000 && address_full < 0xf00000)
    {
        // GA
        if (address == 0xc104) // IRQ
        {
            ret = ga_int_trigger;
            ga_int_trigger = 0;
            MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_IRQ0, 0);
        }
        return ret;
    }
    else if (address_full >= 0xf00000 && address_full < 0xf80000)
    {
        // LSP
        // printf("%02x%04x: read lsp %02x%04x\n", mcu.cp, mcu.pc, page, address);
        return 0x00;
    }
    else
        printf("%02x%04x: read %02x%04x\n", mcu.cp, mcu.pc, page, address);
    return 0xff;
}

uint16_t MCU::MCU_Read16(uint32_t address, bool code)
{
    address &= ~1;
    uint8_t b0, b1;
    b0 = MCU_Read(address, code);
    b1 = MCU_Read(address+1, code);
    return (b0 << 8) + b1;
}

uint32_t MCU::MCU_Read32(uint32_t address, bool code)
{
    address &= ~3;
    uint8_t b0, b1, b2, b3;
    b0 = MCU_Read(address, code);
    b1 = MCU_Read(address+1, code);
    b2 = MCU_Read(address+2, code);
    b3 = MCU_Read(address+3, code);
    return (b0 << 24) + (b1 << 16) + (b2 << 8) + b3;
}

void MCU::loadState(const uint8_t* data)
{
    uint64_t analog_end_time;
    uint32_t uart_write_ptr;
    uint32_t uart_read_ptr;
    uint8_t uart_buffer[8192];
    uint8_t midi_command = 0;
    uint8_t midi_par1 = 0;
    uint8_t midi_par2 = 0;
    uint8_t midi_channel = 0;
    uint8_t midi_stage = 0;
    uint8_t uart_rx_byte;
    uint64_t uart_rx_delay;
    uint64_t uart_tx_delay;

    memcpy(&mcu, data, sizeof(mcu)); data += sizeof(mcu);
    memcpy(sram, data, sizeof(sram)); data += sizeof(sram);
    memcpy(dev_register, data, sizeof(dev_register)); data += sizeof(dev_register);
    memcpy(&dev_IRQCR, data, sizeof(dev_IRQCR)); data += sizeof(dev_IRQCR);
    memcpy(lsp_temp, data, sizeof(lsp_temp)); data += sizeof(lsp_temp);
    memcpy(ga_int, data, sizeof(ga_int)); data += sizeof(ga_int);
    memcpy(&ga_int_enable, data, sizeof(ga_int_enable)); data += sizeof(ga_int_enable);
    memcpy(&ga_int_trigger, data, sizeof(ga_int_trigger)); data += sizeof(ga_int_trigger);
    memcpy(&ga_lcd_counter, data, sizeof(ga_lcd_counter)); data += sizeof(ga_lcd_counter);
    memcpy(&adf_rd, data, sizeof(adf_rd)); data += sizeof(adf_rd);
    memcpy(&analog_end_time, data, sizeof(analog_end_time)); data += sizeof(analog_end_time);
    memcpy(&ssr_rd, data, sizeof(ssr_rd)); data += sizeof(ssr_rd);
    memcpy(&uart_write_ptr, data, sizeof(uart_write_ptr)); data += sizeof(uart_write_ptr);
    memcpy(&uart_read_ptr, data, sizeof(uart_read_ptr)); data += sizeof(uart_read_ptr);
    memcpy(uart_buffer, data, sizeof(uart_buffer)); data += sizeof(uart_buffer);
    memcpy(&midi_command, data, sizeof(midi_command)); data += sizeof(midi_command);
    memcpy(&midi_par1, data, sizeof(midi_par1)); data += sizeof(midi_par1);
    memcpy(&midi_par2, data, sizeof(midi_par2)); data += sizeof(midi_par2);
    memcpy(&midi_channel, data, sizeof(midi_channel)); data += sizeof(midi_channel);
    memcpy(&midi_stage, data, sizeof(midi_stage)); data += sizeof(midi_stage);
    memcpy(&uart_rx_byte, data, sizeof(uart_rx_byte)); data += sizeof(uart_rx_byte);
    memcpy(&uart_rx_delay, data, sizeof(uart_rx_delay)); data += sizeof(uart_rx_delay);
    memcpy(&uart_tx_delay, data, sizeof(uart_tx_delay)); data += sizeof(uart_tx_delay);
    memcpy(&encoder_int_delay, data, sizeof(encoder_int_delay)); data += sizeof(encoder_int_delay);
    memcpy(&lcd_delay, data, sizeof(lcd_delay)); data += sizeof(lcd_delay);
    memcpy(&lcd_ready, data, sizeof(lcd_ready)); data += sizeof(lcd_ready);
    memcpy(&timer_cycles, data, sizeof(timer_cycles)); data += sizeof(timer_cycles);
    memcpy(&timer_tempreg, data, sizeof(timer_tempreg)); data += sizeof(timer_tempreg);
    memcpy(frt, data, sizeof(frt)); data += sizeof(frt);
    memcpy(&timer, data, sizeof(timer)); data += sizeof(timer);
    memcpy(&dev_WDT_TCSR, data, sizeof(dev_WDT_TCSR)); data += sizeof(dev_WDT_TCSR);
    memcpy(&dev_WDT_TCNT, data, sizeof(dev_WDT_TCNT)); data += sizeof(dev_WDT_TCNT);
}

void MCU::MCU_Write(uint32_t address, uint8_t value)
{
    uint32_t address_full = address;
    uint8_t page = address >> 16;
    address &= 0xffff;

    if (address == 0xfe87) // P4DR, contrast
    { }
    else if (address_full >= 0xfe80 && address_full <= 0xff1f)
        MCU_DeviceWrite_510(address, value);
    else if (address_full >= 0xc00000 && address_full < 0xc80000)
    {
        sram[address_full & 0xffff] = value;
    }
    else if (address_full >= 0xe00000 && address_full < 0xe7ffff)
    {
        // SUB CPU
        if (address <= 0xfd)
            printf("%02x%04x: write-f %02x%04x %02x %c\n", mcu.cp, mcu.pc, page, address, value, value);
        if (address == 0x00fe) // SM Port Write
            io_sd = value;
        else if (address == 0x00fd) // SM IPC Semaphore
        { }
        else if (address == 0x00ff) // SM Port control
        { }
    }
    else if (address_full >= 0xe80000 && address_full < 0xefffff)
    {
        // GA
        if (address == 0xc11f)
        {
            // LCD_Write(0, value);
            // printf("%02x%04x: write ga lcd %02x = %02x %c\n", mcu.cp, mcu.pc, address, value, value);
            ga_lcd_counter = 1;
        }
        else if (address > 0xc11f)
        {
            // LCD_Write(1, value);
            // printf("%02x%04x: write ga lcd %02x = %02x %c\n", mcu.cp, mcu.pc, address, value, value);
            ga_lcd_counter = 1;

            if (initial)
            {
                initial = false;
                mcu_button_pressed = 0;
            }
        }
    }
    else if (address_full >= 0xf00000 && address_full < 0xf80000)
    {
        // LSP
        // printf("%02x%04x: write lsp %02x%04x = %02x\n", mcu.cp, mcu.pc, page, address, value);
        lsp_regs[address & 0xf] = value;

        if (address == 0x00) {
            lsp_temp[(lsp_regs[0x01] << 8) | lsp_regs[0x00]] = (lsp_regs[0x02] << 16) | (lsp_regs[0x03] << 8) | lsp_regs[0x04];

            if (mcu.cp == 0x01 && mcu.pc == 0x9501 && ((lsp_regs[0x01] << 8) | lsp_regs[0x00]) == 0x1f8
                && lsp_regs[0x02] == 0x7f && lsp_regs[0x03] == 0x75 && lsp_regs[0x04] == 0x08)
            {
                done = true;
            }
        }
    }
    else if (address_full >= 0xc00000 && address_full < 0xcfffff)
    {
        // XP
        // printf("%02x%04x: write xp %02x%04x = %02x\n", mcu.cp, mcu.pc, page, address, value);
    }
    else
        printf("%02x%04x: write %02x%04x\n", mcu.cp, mcu.pc, page, address);
    return;
}

void MCU::MCU_Write16(uint32_t address, uint16_t value)
{
    if (address == 0xFF10 && value >> 8 == 0xa5)
    {
        dev_WDT_TCSR = value & 0xff;
    }
    else if (address == 0xFF10 && value >> 8 == 0x5a)
    {
        dev_WDT_TCNT = value & 0xff;
    }
    else
    {
        address &= ~1;
        MCU_Write(address, value >> 8);
        MCU_Write(address + 1, value & 0xff);
    }
}

void MCU::MCU_ReadInstruction(void)
{
    uint8_t operand = MCU_ReadCodeAdvance();

    (this->*MCU_Operand_Table[operand])(operand);

    if (mcu.sr & STATUS_T)
    {
        MCU_Interrupt_Exception(EXCEPTION_SOURCE_TRACE);
    }
}

void MCU::MCU_Reset(void)
{
    mcu.r[0] = 0;
    mcu.r[1] = 0;
    mcu.r[2] = 0;
    mcu.r[3] = 0;
    mcu.r[4] = 0;
    mcu.r[5] = 0;
    mcu.r[6] = 0;
    mcu.r[7] = 0;

    mcu.pc = 0;

    mcu.sr = 0x700;

    mcu.cp = 0;
    mcu.dp = 0;
    mcu.ep = 0;
    mcu.tp = 0;
    mcu.br = 0;

    uint32_t reset_address = MCU_GetVectorAddress(VECTOR_RESET);
    mcu.cp = (reset_address >> 16) & 0xff;
    mcu.pc = reset_address & 0xffff;

    mcu.exception_pending = -1;

    MCU_DeviceReset();

    ga_int_enable = 255;

    encoder_int_delay = 0;
    lcd_delay = 0;
    lcd_ready = true;
}

void MCU::MCU_Run()
{
    uint32_t button_pressed_tmp = 0x00;

    done = false;

    while (!done)
    {
        if (!mcu.ex_ignore)
            MCU_Interrupt_Handle();
        else
            mcu.ex_ignore = 0;

        if (button_pressed_tmp > 0)
            button_pressed_tmp = 0;
        else
            button_pressed_tmp = 1 << 6;
        mcu_button_pressed = button_pressed_tmp;

        if (!mcu.sleep)
            MCU_ReadInstruction();

        mcu.cycles += 12; // FIXME: assume 12 cycles per instruction

        TIMER_Clock(mcu.cycles);

        if (ga_lcd_counter)
        {
            ga_lcd_counter--;
            if (ga_lcd_counter == 0)
            {
                MCU_GA_SetGAInt(1, 0);
                MCU_GA_SetGAInt(1, 1);
            }
        }
    }
}

void MCU::MCU_GA_SetGAInt(int line, int value)
{
    // guesswork
    if (value && !ga_int[line] && (ga_int_enable & (1 << line)) != 0)
        ga_int_trigger = line;
    ga_int[line] = value;

    MCU_Interrupt_SetRequest(INTERRUPT_SOURCE_IRQ0, ga_int_trigger != 0);
}

// int main(int argc, char *argv[])
// {
//     MCU mcu;
//     memset(&mcu.mcu, 0, sizeof(MCU::mcu_core));
    
//     mcu.MCU_Reset();
//     FILE *state_file = fopen("state_loadlsp.bin", "rb");
//     uint8_t state_data[76145];
//     fread(state_data, 1, sizeof(state_data), state_file);
//     mcu.loadState(state_data);

//     FILE *rom2_file = fopen("sc88pro_valid.bin", "rb");
//     fread(mcu.rom2, 1, ROM2_SIZE, rom2_file);
//     fclose(rom2_file);

//     mcu.rom2[0xd0100+0x3a] = 0x01; // mfx group
//     mcu.rom2[0xd0100+0x3b] = 0x00; // mfx type
//     for (size_t i = 0; i < 0x15; i++)
//     {
//         mcu.rom2[0xd0100+0x3c + i] = 0x40; // mfx params
//     }

//     mcu.MCU_Run();
//     FILE *f = fopen("lsp_dump.bin", "wb");
//     fwrite(mcu.lsp_temp+0x80, 1, sizeof(mcu.lsp_temp)-(0x80*4), f);
//     fclose(f);

//     return 0;
// }

void MCU::ProgGen_Prepare(const uint8_t *rom2_data, size_t rom2_size)
{
    memset(&mcu, 0, sizeof(mcu_core));

    memcpy(rom2, rom2_data, rom2_size);
}

uint32_t* MCU::ProgGen_Generate(uint8_t group, uint8_t type, uint8_t params[21], const uint8_t * saveStateData)
{
    MCU_Reset();
    loadState(saveStateData);

    rom2[0xd0100+0x3a] = group; // mfx group
    rom2[0xd0100+0x3b] = type; // mfx type
    memcpy(&rom2[0xd0100+0x3c], params, 21); // mfx params

    MCU_Run();
    
    return lsp_temp + 0x80;
}
