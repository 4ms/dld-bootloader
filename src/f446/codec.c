#include "codec_CS4271.h"
#include "codec_CS4271_regs.h"
#include "dig_pins.h"

//TODO: can we make this a common interface?

const uint8_t codec_init_data_slave[] =
{
		SINGLE_SPEED
		| RATIO0
		| SLAVE
		| DIF_I2S_24b,		//MODECTRL1

		SLOW_FILT_SEL
		| DEEMPH_OFF,		//DACCTRL

		ATAPI_aLbR,			//DACMIX

		0b00000000,			//DACAVOL
		0b00000000,			//DACBVOL

		ADC_DIF_I2S
		/*| HPFDisableA
		| HPFDisableB */	//ADCCTRL

};

static uint32_t Codec_Reset(I2C_TypeDef *CODEC);

uint32_t Codec_Register_Setup() {
	uint32_t err = 0;
	Codec_A_CtrlInterface_Init();

	CODECA_RESET_HIGH;
	HAL_Delay(2);

	err = Codec_Reset(CODECA_I2C);

	return err;
}

static uint32_t Codec_Reset(I2C_TypeDef *CODEC) {
	uint8_t i;
	uint32_t err = 0;

	err += Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN | PDN, CODEC); //Control Port Enable and Power Down Enable

	for (i = 0; i < CS4271_NUM_REGS; i++)
		err += Codec_WriteRegister(i + 1, codec_init_data_slave[i], CODEC);

	err += Codec_WriteRegister(CS4271_REG_MODELCTRL2, CPEN, CODEC); //Power Down disable

	return err;
}
