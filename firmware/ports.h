//********************************************************************
//          Macros for easy i/o pin access
//********************************************************************


#define BIT(p,b)                (b)

#define PORT(p,b)               (PORT ## p)
#define PIN(p,b)                (PIN ## p)
#define DDR(p,b)                (DDR ## p)


#define Set_Port_Bit(p,b)       ((p) |= _BV(b))
#define Clr_Port_Bit(p,b)       ((p) &= ~_BV(b))
#define Tgl_Port_Bit(p,b)       ((p) ^= _BV(b))

#define Get_Port_Bit(p,b)       (((p) & _BV(b)) != 0)


//user functions:
#define bit(io)	                BIT(io)
#define port(io)                PORT(io)

#define pin_high(io)            Set_Port_Bit(PORT(io),BIT(io))
#define pin_low(io)             Clr_Port_Bit(PORT(io),BIT(io))
#define pin_toggle(io)          Tgl_Port_Bit(PORT(io),BIT(io))

#define get_output(io)          Get_Port_Bit(PORT(io),BIT(io))
#define get_input(io)           Get_Port_Bit(PIN(io),BIT(io))

#define set_dir_in(io)          Clr_Port_Bit(DDR(io),BIT(io))
#define set_dir_out(io)         Set_Port_Bit(DDR(io),BIT(io))
#define dir_toggle(io)          Tgl_Port_Bit(DDR(io),BIT(io))

//********************************************************************
// define pins as:
// #define LED D,PD2
//
// write code as:
// pin_high(LED)
//
//********************************************************************

#define hi_z(io)				set_dir_in(io)
#define pull_up(io)				set_dir_in(io)
