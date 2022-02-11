#ifndef _WARP_TRANSFORM_NETWORK_CONFIG_HPP
#define _WARP_TRANSFORM_NETWORK_CONFIG_HPP

#define CPUPacketFsmType uint8_t

/********************************************
 * Internal WarpTransform accelerator command
 ********************************************/
enum MemTestCmd {
    WRPTX_IMG_CMD = 2,
    WRPTX_TXMAT_CMD  = 1,
    WRPTX_INVLD_CMD = 0
};

//CMD 8 bitwdith up to 255 commands (0 is invalid)
#define WARPTRANSFORM_COMMANDS_HIGH_BIT WARPTRANSFORM_COMMANDS_BITWIDTH-1
#define WARPTRANSFORM_COMMANDS_LOW_BIT 0
#define WARPTRANSFORM_COMMANDS_BITWIDTH 8

typedef unsigned int  img_meta_t; 
#define TRANSFORM_MATRIX_DIM 9
const unsigned int const_tx_matrix_dim=TRANSFORM_MATRIX_DIM;


//64 bits 8 for cmd, 40 rows/cols 3 channels = 51 missing 13
//If  other info, we need to change how it is working many stuffs I think
#define WARPTRANSFORM_CHNNEL_BITWIDTH 8
#define WARPTRANSFORM_COLS_BITWIDTH 16
#define WARPTRANSFORM_ROWS_BITWIDTH 16

#define WARPTRANSFORM_ROWS_HIGH_BIT NETWORK_WORD_BIT_WIDTH-1 // 63
#define WARPTRANSFORM_ROWS_LOW_BIT NETWORK_WORD_BIT_WIDTH-WARPTRANSFORM_ROWS_BITWIDTH //64-16 = 48

#define WARPTRANSFORM_COLS_HIGH_BIT WARPTRANSFORM_ROWS_LOW_BIT-1 // 47
#define WARPTRANSFORM_COLS_LOW_BIT WARPTRANSFORM_ROWS_LOW_BIT-WARPTRANSFORM_COLS_BITWIDTH //48-16 = 32

// #define WARPTRANSFORM_CHNNEL_HIGH_BIT WARPTRANSFORM_COLS_LOW_BIT-1 // 31
#define WARPTRANSFORM_CHNNEL_HIGH_BIT 16-1 // 15
#define WARPTRANSFORM_CHNNEL_LOW_BIT 16-WARPTRANSFORM_CHNNEL_BITWIDTH //16-8 = 8

union float_bits_u {
    unsigned int i;
    float f;
};

#endif //_WARP_TRANSFORM_NETWORK_CONFIG_HPP