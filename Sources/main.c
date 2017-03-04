/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MKE02Z2.h"

static int i = 0;

#define GPIOA_PTB4_MASK (1U<<12U) // bit respectivo ao PTB4 nos registradores de GPIO, pagina 583

// interrupção do Periodic Interrupt Timer, canal 0
void PIT_CH0_IRQHandler(void){
	GPIOA->PTOR |= GPIOA_PTB4_MASK; // toggle em ptb 4

	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK; // escreve 1 na flag de interrupção para limpa-la (senão a interrupção fica disparando sempre)
}

int main(void)
{
	// força divisão do clock de bus por dois (perifericos operando em 20MHz)
	SIM->BUSDIV |= SIM_BUSDIV_BUSDIV_MASK;

	// Enable PTB4 (Led Pin) as OUTPUT. Refer to http://www.nxp.com/assets/documents/data/en/reference-manuals/MKE02P64M40SF0RM.pdf
	SIM->SOPT &= ~(SIM_SOPT_NMIE_MASK); // desabilita a função NMI do pino PTB4, pagina 163

	PORT->PUEL &= ~(PORT_PUEL_PTBPE4_MASK); // escreve 0 on bit 12 from PUEL, pagina 146/148 do reference, desabilita pullup na porta
	PORT->HDRVE |= PORT_HDRVE_PTB4_MASK; // escreve 1 no bit 0 do HDRVE, pagina 1154/155, habilita saida de alta corrente em PTB4
	GPIOA->PDDR |= GPIOA_PTB4_MASK; // define pino como output, pagina 588

	// habilitando a interrupção temporal para aproximadamente 1s
	SIM->SCGC |= SIM_SCGC_PIT_MASK; // habilita clock para o periférico de PIT, pagina 167
	PIT->MCR |= (PIT_MCR_MDIS_MASK | PIT_MCR_FRZ_MASK); // habilita o driver e habilita freeze em debug, pagina 476

	// Configuração de clock
	SystemCoreClockUpdate(); // atualiza o valor interno de clock
	// 20MHz sao  0,00000005s por pulso, para obter 1s = timeout = 20.000.000 = 0x1312D00
	uint32_t pitTimeout = 0x1312D00;
	PIT->CHANNEL[0].LDVAL = pitTimeout;
	PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK); // habilita interrupção e inicia o timer

    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        __asm("NOP"); // nada a se fazer aqui!
    }
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
