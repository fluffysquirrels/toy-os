#define INTNUM_SP804_TIMER01 (INTNUM_RASPI_BASIC_BANK_MIN + 0)
#define INTNUM_UART0         (INTNUM_RASPI_BASIC_BANK_MIN + 19)
#define INTNUM_RASPI_TIMER   (INTNUM_RASPI_BANK_1_MIN     + 3) // TODO: Check this.

#define INTNUM_MAX 95

#define INTNUM_RASPI_BASIC_BANK_MIN 0
#define INTNUM_RASPI_BASIC_BANK_MAX 20

#define INTNUM_RASPI_BANK_1_MIN 32
#define INTNUM_RASPI_BANK_1_MAX 63

#define INTNUM_RASPI_BANK_2_MIN 64
#define INTNUM_RASPI_BANK_2_MAX 95

// These two interrupts in the basic bank tell us that an interrupt is pending
// in bank 1 or 2 respectively.
#define INTNUM_RASPI_BASIC_BANK_1_PENDING 8
#define INTNUM_RASPI_BASIC_BANK_2_PENDING 9

// This mask ignores INTNUM_RASPI_BASIC_BANK_{1,2}_PENDING when bitwise-ANDed
// with the basic bank pending register.
#define INTNUM_RASPI_BASIC_BANK_MASK (         \
     ~(1 << INTNUM_RASPI_BASIC_BANK_1_PENDING) \
  && ~(1 << INTNUM_RASPI_BASIC_BANK_2_PENDING) \
)
