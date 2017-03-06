#include <ctype.h>

const uchar _ctype[256] = {
    /***** ASCII 0-127 *****/
    _C_CTRL,             //   0 (NUL)
    _C_CTRL,             //   1 (SOH)
    _C_CTRL,             //   2 (STX)
    _C_CTRL,             //   3 (ETX)
    _C_CTRL,             //   4 (EOT)
    _C_CTRL,             //   5 (ENQ)
    _C_CTRL,             //   6 (ACK)
    _C_CTRL,             //   7 (BEL)
    _C_CTRL,             //   8 (BS)
    _C_CTRL | _C_SPACE,  //   9 (HT)
    _C_CTRL | _C_SPACE,  //  10 (LF)
    _C_CTRL | _C_SPACE,  //  11 (VT)
    _C_CTRL | _C_SPACE,  //  12 (FF)
    _C_CTRL | _C_SPACE,  //  13 (CR)
    _C_CTRL,             //  14 (SI)
    _C_CTRL,             //  15 (SO)
    _C_CTRL,             //  16 (DLE)
    _C_CTRL,             //  17 (DC1)
    _C_CTRL,             //  18 (DC2)
    _C_CTRL,             //  19 (DC3)
    _C_CTRL,             //  20 (DC4)
    _C_CTRL,             //  21 (NAK)
    _C_CTRL,             //  22 (SYN)
    _C_CTRL,             //  23 (ETB)
    _C_CTRL,             //  24 (CAN)
    _C_CTRL,             //  25 (EM)
    _C_CTRL,             //  26 (SUB)
    _C_CTRL,             //  27 (ESC)
    _C_CTRL,             //  28 (FS)
    _C_CTRL,             //  29 (GS)
    _C_CTRL,             //  30 (RS)
    _C_CTRL,             //  31 (US)
    _C_BLANK | _C_SPACE, //  32 SPACE
    _C_PUNCT,            //  33 !
    _C_PUNCT,            //  34 "
    _C_PUNCT,            //  35 #
    _C_PUNCT,            //  36 $
    _C_PUNCT,            //  37 %
    _C_PUNCT,            //  38 &
    _C_PUNCT,            //  39 '
    _C_PUNCT,            //  40 (
    _C_PUNCT,            //  41 )
    _C_PUNCT,            //  42 *
    _C_PUNCT,            //  43  |
    _C_PUNCT,            //  44 ,
    _C_PUNCT,            //  45 -
    _C_PUNCT,            //  46 .
    _C_PUNCT,            //  47 /
    _C_DIGIT | _C_HEX,   //  48 0
    _C_DIGIT | _C_HEX,   //  49 1
    _C_DIGIT | _C_HEX,   //  50 2
    _C_DIGIT | _C_HEX,   //  51 3
    _C_DIGIT | _C_HEX,   //  52 4
    _C_DIGIT | _C_HEX,   //  53 5
    _C_DIGIT | _C_HEX,   //  54 6
    _C_DIGIT | _C_HEX,   //  55 7
    _C_DIGIT | _C_HEX,   //  56 8
    _C_DIGIT | _C_HEX,   //  57 9
    _C_PUNCT,            //  58 :
    _C_PUNCT,            //  59 ;
    _C_PUNCT,            //  60 <
    _C_PUNCT,            //  61 =
    _C_PUNCT,            //  62 >
    _C_PUNCT,            //  63 ?
    _C_PUNCT,            //  64 @
    _C_UPPER | _C_HEX,   //  65 A
    _C_UPPER | _C_HEX,   //  66 B
    _C_UPPER | _C_HEX,   //  67 C
    _C_UPPER | _C_HEX,   //  68 D
    _C_UPPER | _C_HEX,   //  69 E
    _C_UPPER | _C_HEX,   //  70 F
    _C_UPPER,            //  71 G
    _C_UPPER,            //  72 H
    _C_UPPER,            //  73 I
    _C_UPPER,            //  74 J
    _C_UPPER,            //  75 K
    _C_UPPER,            //  76 L
    _C_UPPER,            //  77 M
    _C_UPPER,            //  78 N
    _C_UPPER,            //  79 O
    _C_UPPER,            //  80 P
    _C_UPPER,            //  81 Q
    _C_UPPER,            //  82 R
    _C_UPPER,            //  83 S
    _C_UPPER,            //  84 T
    _C_UPPER,            //  85 U
    _C_UPPER,            //  86 V
    _C_UPPER,            //  87 W
    _C_UPPER,            //  88 X
    _C_UPPER,            //  89 Y
    _C_UPPER,            //  90 Z
    _C_PUNCT,            //  91 [
    _C_PUNCT,            //  92 backslash
    _C_PUNCT,            //  93 ]
    _C_PUNCT,            //  94 ^
    _C_PUNCT,            //  95 _
    _C_PUNCT,            //  96 `
    _C_LOWER | _C_HEX,   //  97 a
    _C_LOWER | _C_HEX,   //  98 b
    _C_LOWER | _C_HEX,   //  99 c
    _C_LOWER | _C_HEX,   // 100 d
    _C_LOWER | _C_HEX,   // 101 e
    _C_LOWER | _C_HEX,   // 102 f
    _C_LOWER,            // 103 g
    _C_LOWER,            // 104 h
    _C_LOWER,            // 105 i
    _C_LOWER,            // 106 j
    _C_LOWER,            // 107 k
    _C_LOWER,            // 108 l
    _C_LOWER,            // 109 m
    _C_LOWER,            // 110 n
    _C_LOWER,            // 111 o
    _C_LOWER,            // 112 p
    _C_LOWER,            // 113 q
    _C_LOWER,            // 114 r
    _C_LOWER,            // 115 s
    _C_LOWER,            // 116 t
    _C_LOWER,            // 117 u
    _C_LOWER,            // 118 v
    _C_LOWER,            // 119 w
    _C_LOWER,            // 120 x
    _C_LOWER,            // 121 y
    _C_LOWER,            // 122 z
    _C_PUNCT,            // 123 {
    _C_PUNCT,            // 124 |
    _C_PUNCT,            // 125 }
    _C_PUNCT,            // 126 ~
    _C_CTRL,             // 127 (DEL)
    /***** Reserved 128-159 *****/
    0,                   // 128
    0,                   // 129
    0,                   // 130
    0,                   // 131
    0,                   // 132
    0,                   // 133
    0,                   // 134
    0,                   // 135
    0,                   // 136
    0,                   // 137
    0,                   // 138
    0,                   // 139
    0,                   // 140
    0,                   // 141
    0,                   // 142
    0,                   // 143
    0,                   // 144
    0,                   // 145
    0,                   // 146
    0,                   // 147
    0,                   // 148
    0,                   // 149
    0,                   // 150
    0,                   // 151
    0,                   // 152
    0,                   // 153
    0,                   // 154
    0,                   // 155
    0,                   // 156
    0,                   // 157
    0,                   // 158
    0,                   // 159
    /***** Latin1 160-255 *****/
    _C_BLANK | _C_SPACE, // 160
    _C_PUNCT,            // 161
    _C_PUNCT,            // 162
    _C_PUNCT,            // 163
    _C_PUNCT,            // 164
    _C_PUNCT,            // 165
    _C_PUNCT,            // 166
    _C_PUNCT,            // 167
    _C_PUNCT,            // 168
    _C_PUNCT,            // 169
    _C_PUNCT,            // 170
    _C_PUNCT,            // 171
    _C_PUNCT,            // 172
    _C_PUNCT,            // 173
    _C_PUNCT,            // 174
    _C_PUNCT,            // 175
    _C_PUNCT,            // 176
    _C_PUNCT,            // 177
    _C_PUNCT,            // 178
    _C_PUNCT,            // 179
    _C_PUNCT,            // 180
    _C_PUNCT,            // 181
    _C_PUNCT,            // 182
    _C_PUNCT,            // 183
    _C_PUNCT,            // 184
    _C_PUNCT,            // 185
    _C_PUNCT,            // 186
    _C_PUNCT,            // 187
    _C_PUNCT,            // 188
    _C_PUNCT,            // 189
    _C_PUNCT,            // 190
    _C_PUNCT,            // 191
    _C_UPPER,            // 192
    _C_UPPER,            // 193
    _C_UPPER,            // 194
    _C_UPPER,            // 195
    _C_UPPER,            // 196
    _C_UPPER,            // 197
    _C_UPPER,            // 198
    _C_UPPER,            // 199
    _C_UPPER,            // 200
    _C_UPPER,            // 201
    _C_UPPER,            // 202
    _C_UPPER,            // 203
    _C_UPPER,            // 204
    _C_UPPER,            // 205
    _C_UPPER,            // 206
    _C_UPPER,            // 207
    _C_UPPER,            // 208
    _C_UPPER,            // 209
    _C_UPPER,            // 210
    _C_UPPER,            // 211
    _C_UPPER,            // 212
    _C_UPPER,            // 213
    _C_UPPER,            // 214
    _C_PUNCT,            // 215
    _C_UPPER,            // 216
    _C_UPPER,            // 217
    _C_UPPER,            // 218
    _C_UPPER,            // 219
    _C_UPPER,            // 220
    _C_UPPER,            // 221
    _C_UPPER,            // 222
    _C_LOWER,            // 223
    _C_LOWER,            // 224
    _C_LOWER,            // 225
    _C_LOWER,            // 226
    _C_LOWER,            // 227
    _C_LOWER,            // 228
    _C_LOWER,            // 229
    _C_LOWER,            // 230
    _C_LOWER,            // 231
    _C_LOWER,            // 232
    _C_LOWER,            // 233
    _C_LOWER,            // 234
    _C_LOWER,            // 235
    _C_LOWER,            // 236
    _C_LOWER,            // 237
    _C_LOWER,            // 238
    _C_LOWER,            // 239
    _C_LOWER,            // 240
    _C_LOWER,            // 241
    _C_LOWER,            // 242
    _C_LOWER,            // 243
    _C_LOWER,            // 244
    _C_LOWER,            // 245
    _C_LOWER,            // 246
    _C_PUNCT,            // 247
    _C_LOWER,            // 248
    _C_LOWER,            // 249
    _C_LOWER,            // 250
    _C_LOWER,            // 251
    _C_LOWER,            // 252
    _C_LOWER,            // 253
    _C_LOWER,            // 254
    _C_LOWER             // 255
};
