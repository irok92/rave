#pragma once

#define RV_T_UINTN(n) (RV_T_UINT << 8 | (0xFF & n))

typedef enum rv_type {
    RV_T_UINT  = 0x01,
    RV_T_SINT  = 0x02,
    RV_T_FLOAT = 0x03,
    RV_T_BOOL  = 0x04,
    RV_T_CHAR  = 0x05,
    RV_T_VOID  = 0x06,
} rv_type;

typedef enum rv_mod {
    RV_M_REF = 0x01,
    RV_M_MUT = 0x02,
    RV_M_PTR = 0x03,
} rv_modifier;

typedef enum rv_group {
    RV_G_FUNC = 0x01,
    RV_G_ARRAY = 0x02,
    RV_G_SPAN = 0x03,
    RV_G_ENUM = 0x04,
    RV_G_FLAG = 0x05,
    RV_G_UNION = 0x06,
    RV_G_TUPLE = 0x07
} rv_group;
