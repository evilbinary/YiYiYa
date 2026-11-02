/* g2d_mini_write: 29 条指令 */
static const uint64_t g2d_mini_write[] = {
    0x39800000bb03f000ULL, /*  0: nop                           ; nop                         ; ldunifrf.rf0 */
    0x39804000bb03f000ULL, /*  1: nop                           ; nop                         ; ldunifrf.rf1 */
    0x39808000bb03f000ULL, /*  2: nop                           ; nop                         ; ldunifrf.rf2 */
    0x3980c000bb03f000ULL, /*  3: nop                           ; nop                         ; ldunifrf.rf3 */
    0x39810000bb03f000ULL, /*  4: nop                           ; nop                         ; ldunifrf.rf4 */
    0x39814000bb03f000ULL, /*  5: nop                           ; nop                         ; ldunifrf.rf5 */
    0x38000000bb03f000ULL, /*  6: nop                           ; nop */
    0x38000000bb03f000ULL, /*  7: nop                           ; nop */
    0x38000000bb03f000ULL, /*  8: nop                           ; nop */
    0x38000000bb03f000ULL, /*  9: nop                           ; nop */
    0x38000000bb03f000ULL, /* 10: nop                           ; nop */
    0x38000000bb03f000ULL, /* 11: nop                           ; nop */
    0x38001020f903f003ULL, /* 12: mov tmuc, rf0                 ; nop */
    0x38001020f903f043ULL, /* 13: mov tmuc, rf1                 ; nop */
    0x38001020f903f083ULL, /* 14: mov tmuc, rf2                 ; nop */
    0x3800100cf903f103ULL, /* 15: mov tmua, rf4                 ; nop */
    0x38200000bb03f000ULL, /* 16: nop                           ; nop                         ; thrsw */
    0x38870000bb03f000ULL, /* 17: nop                           ; nop                         ; ldtmu.rf28 */
    0x38000000bb03f000ULL, /* 18: nop                           ; nop */
    0x38000000bb03f000ULL, /* 19: nop                           ; nop */
    0x380022c0bb143000ULL, /* 20: nop                           ; mov tmud, rf5 */
    0x3800100cf903f0c3ULL, /* 21: mov tmua, rf3                 ; nop */
    0x38001006bb03f00fULL, /* 22: tmuwt -                       ; nop */
    0x38000000bb03f000ULL, /* 23: nop                           ; nop */
    0x38000000bb03f000ULL, /* 24: nop                           ; nop */
    0x38200000bb03f000ULL, /* 25: nop                           ; nop                         ; thrsw */
    0x38000000bb03f000ULL, /* 26: nop                           ; nop */
    0x38000000bb03f000ULL, /* 27: nop                           ; nop */
    0x38000000bb03f000ULL, /* 28: nop                           ; nop */
};
static const unsigned g2d_mini_write_n = 29;

/* g2d_mini_read: 42 条指令 */
static const uint64_t g2d_mini_read[] = {
    0x39800000bb03f000ULL, /*  0: nop                           ; nop                         ; ldunifrf.rf0 */
    0x39804000bb03f000ULL, /*  1: nop                           ; nop                         ; ldunifrf.rf1 */
    0x39808000bb03f000ULL, /*  2: nop                           ; nop                         ; ldunifrf.rf2 */
    0x3980c000bb03f000ULL, /*  3: nop                           ; nop                         ; ldunifrf.rf3 */
    0x39810000bb03f000ULL, /*  4: nop                           ; nop                         ; ldunifrf.rf4 */
    0x39814000bb03f000ULL, /*  5: nop                           ; nop                         ; ldunifrf.rf5 */
    0x39818000bb03f000ULL, /*  6: nop                           ; nop                         ; ldunifrf.rf6 */
    0x3981c000bb03f000ULL, /*  7: nop                           ; nop                         ; ldunifrf.rf7 */
    0x38000000bb03f000ULL, /*  8: nop                           ; nop */
    0x38000000bb03f000ULL, /*  9: nop                           ; nop */
    0x38000000bb03f000ULL, /* 10: nop                           ; nop */
    0x38000000bb03f000ULL, /* 11: nop                           ; nop */
    0x38000000bb03f000ULL, /* 12: nop                           ; nop */
    0x38000000bb03f000ULL, /* 13: nop                           ; nop */
    0x38001020f903f143ULL, /* 14: mov tmuc, rf5                 ; nop */
    0x38001020f903f183ULL, /* 15: mov tmuc, rf6                 ; nop */
    0x38001020f903f1c3ULL, /* 16: mov tmuc, rf7                 ; nop */
    0x38000000bb03f000ULL, /* 17: nop                           ; nop */
    0x38000000bb03f000ULL, /* 18: nop                           ; nop */
    0x3800100cf903f103ULL, /* 19: mov tmua, rf4                 ; nop */
    0x38200000bb03f000ULL, /* 20: nop                           ; nop                         ; thrsw */
    0x38870000bb03f000ULL, /* 21: nop                           ; nop                         ; ldtmu.rf28 */
    0x38874000bb03f000ULL, /* 22: nop                           ; nop                         ; ldtmu.rf29 */
    0x38878000bb03f000ULL, /* 23: nop                           ; nop                         ; ldtmu.rf30 */
    0x3887c000bb03f000ULL, /* 24: nop                           ; nop                         ; ldtmu.rf31 */
    0x38000000bb03f000ULL, /* 25: nop                           ; nop */
    0x38000000bb03f000ULL, /* 26: nop                           ; nop */
    0x38000000bb03f000ULL, /* 27: nop                           ; nop */
    0x380022c0bb703000ULL, /* 28: nop                           ; mov tmud, rf28 */
    0x3800100cf903f0c3ULL, /* 29: mov tmua, rf3                 ; nop */
    0x380022c0bb743000ULL, /* 30: nop                           ; mov tmud, rf29 */
    0x3800100cf903f143ULL, /* 31: mov tmua, rf5                 ; nop */
    0x380022c0bb783000ULL, /* 32: nop                           ; mov tmud, rf30 */
    0x3800100cf903f183ULL, /* 33: mov tmua, rf6                 ; nop */
    0x380022c0bb7c3000ULL, /* 34: nop                           ; mov tmud, rf31 */
    0x3800100cf903f1c3ULL, /* 35: mov tmua, rf7                 ; nop */
    0x38001006bb03f00fULL, /* 36: tmuwt -                       ; nop */
    0x38000000bb03f000ULL, /* 37: nop                           ; nop */
    0x38000000bb03f000ULL, /* 38: nop                           ; nop */
    0x38200000bb03f000ULL, /* 39: nop                           ; nop                         ; thrsw */
    0x38000000bb03f000ULL, /* 40: nop                           ; nop */
    0x38000000bb03f000ULL, /* 41: nop                           ; nop */
};
static const unsigned g2d_mini_read_n = 42;

