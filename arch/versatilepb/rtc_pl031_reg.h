#define RTC_BASE ((volatile uint32_t*) 0x101E8000)

#define RTC_DR   0x0     // 0x00 bytes
#define RTC_MR   0x1     // 0x04 bytes
#define RTC_LR   0x2     // 0x08 bytes
#define RTC_CR   0x3     // 0x0C bytes
#define RTC_IMSC 0x4     // 0x10 bytes
#define RTC_RIS  0x5     // 0x14 bytes
#define RTC_MIS  0x6     // 0x18 bytes
#define RTC_ICR  0x7     // 0x1C bytes

#define RTC_CR_START (1 << 0)

#define RTC_MIS_IMSC (1 << 0)

#define RTC_ICR_CLEAR (1 << 0)
