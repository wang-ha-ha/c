/*
    Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

        * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
          and/or other materials provided with the distribution.
        * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "cli/pch.h"

#include "encoding.h"
#include "constraints.h"

CLI_NS_USE(cli)


StringEncoder::StringEncoder(void)
  : m_tkString(10)
{
}

StringEncoder::~StringEncoder(void)
{
}

const char* const StringEncoder::Encode(const KEY E_Key) const
{
    switch (E_Key)
    {
    case KEY_aacute:    return "á";
    case KEY_agrave:    return "à";
    case KEY_auml:      return "ä";
    case KEY_acirc:     return "â";
    case KEY_ccedil:    return "ç";
    case KEY_eacute:    return "é";
    case KEY_egrave:    return "è";
    case KEY_euml:      return "ë";
    case KEY_ecirc:     return "ê";
    case KEY_iacute:    return "í";
    case KEY_igrave:    return "ì";
    case KEY_iuml:      return "ï";
    case KEY_icirc:     return "î";
    case KEY_oacute:    return "ó";
    case KEY_ograve:    return "ò";
    case KEY_ouml:      return "ö";
    case KEY_ocirc:     return "ô";
    case KEY_uacute:    return "ú";
    case KEY_ugrave:    return "ù";
    case KEY_uuml:      return "ü";
    case KEY_ucirc:     return "û";
    case SQUARE:        return "²";
    case EURO:          return "€";
    case POUND:         return "£";
    case MICRO:         return "µ";
    case PARAGRAPH:     return "§";
    case DEGREE:        return "°";
    case COPYRIGHT:     return "©";
    default:
        m_tkString.Reset();
        m_tkString.Append((char) E_Key);
        return (const char*) m_tkString;
    }
}


StringDecoder::StringDecoder(void)
  : m_tkString(10), m_iCurrentSequence(0)
{
}

StringDecoder::StringDecoder(const char* const STR_String)
  : m_tkString(MAX_CMD_LINE_LENGTH, STR_String), m_iCurrentSequence(0)
{
}

StringDecoder::~StringDecoder(void)
{
}

const KEY StringDecoder::Decode(const int I_Char)
{
    // Concatenate sequence.
    int i_Char = (I_Char & 0xff); // Ensure only the last byte is processed.
    if (m_iCurrentSequence != 0)
    {
        i_Char = (m_iCurrentSequence << 8) + i_Char;
        m_iCurrentSequence = 0;
    }

    switch (i_Char)
    {
    case 0xc2: case 0xc3:
    case 0xe2: case 0xe282: // utf-8 encoding of '€': e2 82 ac
        m_iCurrentSequence = i_Char;
        return FEED_MORE;

    case 10: case 13:   return ENTER;

    case ' ':           return SPACE;
    case '\t':          return TAB;
    case '\b':          return BACKSPACE;

    case '0':           return KEY_0;
    case '1':           return KEY_1;
    case '2':           return KEY_2;
    case '3':           return KEY_3;
    case '4':           return KEY_4;
    case '5':           return KEY_5;
    case '6':           return KEY_6;
    case '7':           return KEY_7;
    case '8':           return KEY_8;
    case '9':           return KEY_9;

    case 'a':           return KEY_a;
    case 0xc3a1:        // utf-8 encoding for 'á'
    case 0xe1:          // iso-8859-1 encoding for 'á'
                        return KEY_aacute;
    case 0xc3a0:        // utf-8 encoding for 'à'
    case 0xe0:          // iso-8859-1 encoding for 'à'
                        return KEY_agrave;
    case 0xc3a4:        // utf-8 encoding for 'ä'
    case 0xe4:          // iso-8859-1 encoding for 'ä'
                        return KEY_auml;
    case 0xc3a2:        // utf-8 encoding for 'â'
    //case 0xe2:        // iso-8859-1 encoding for 'â' / conflicting with utf-8 encoding
                        return KEY_acirc;
    case 'b':           return KEY_b;
    case 'c':           return KEY_c;
    case 0xc3a7:        // utf-8 encoding for 'ç'
    case 0xe7:          // iso-8859-1 encoding for 'ç'
                        return KEY_ccedil;
    case 'd':           return KEY_d;
    case 'e':           return KEY_e;
    case 0xc3a9:        // utf-8 encoding for 'é'
    case 0xe9:          // iso-8859-1 encoding for 'é'
                        return KEY_eacute;
    case 0xc3a8:        // utf-8 encoding for 'è'
    case 0xe8:          // iso-8859-1 encoding for 'è'
                        return KEY_egrave;
    case 0xc3ab:        // utf-8 encoding for 'ë'
    case 0xeb:          // iso-8859-1 encoding for 'ë'
                        return KEY_euml;
    case 0xc3aa:        // utf-8 encoding for 'ê'
    case 0xea:          // iso-8859-1 encoding for 'ê'
                        return KEY_ecirc;
    case 'f':           return KEY_f;
    case 'g':           return KEY_g;
    case 'h':           return KEY_h;
    case 'i':           return KEY_i;
    case 0xc3ad:        // utf-8 encoding for 'í'
    case 0xed:          // iso-8859-1 encoding for 'í'
                        return KEY_iacute;
    case 0xc3ac:        // utf-8 encoding for 'ì'
    case 0xec:          // iso-8859-1 encoding for 'ì'
                        return KEY_igrave;
    case 0xc3af:        // utf-8 encoding for 'ï'
    case 0xef:          // iso-8859-1 encoding for 'ï'
                        return KEY_iuml;
    case 0xc3ae:        // utf-8 encoding for 'î'
    case 0xee:          // iso-8859-1 encoding for 'î'
                        return KEY_icirc;
    case 'j':           return KEY_j;
    case 'k':           return KEY_k;
    case 'l':           return KEY_l;
    case 'm':           return KEY_m;
    case 'n':           return KEY_n;
    case 'o':           return KEY_o;
    case 0xc3b3:        // utf-8 encoding for 'ó'
    case 0xf3:          // iso-8859-1 encoding for 'ó'
                        return KEY_oacute;
    case 0xc3b2:        // utf-8 encoding for 'ò'
    case 0xf2:          // iso-8859-1 encoding for 'ò'
                        return KEY_ograve;
    case 0xc3b6:        // utf-8 encoding for 'ö'
    case 0xf6:          // iso-8859-1 encoding for 'ö'
                        return KEY_ouml;
    case 0xc3b4:        // utf-8 encoding for 'ô'
    case 0xf4:          // iso-8859-1 encoding for 'ô'
                        return KEY_ocirc;
    case 'p':           return KEY_p;
    case 'q':           return KEY_q;
    case 'r':           return KEY_r;
    case 's':           return KEY_s;
    case 't':           return KEY_t;
    case 'u':           return KEY_u;
    case 0xc3ba:        // utf-8 encoding for 'ú'
    case 0xfa:          // iso-8859-1 encoding for 'ú'
                        return KEY_uacute;
    case 0xc3b9:        // utf-8 encoding for 'ù'
    case 0xf9:          // iso-8859-1 encoding for 'ù'
                        return KEY_ugrave;
    case 0xc3bc:        // utf-8 encoding for 'ü'
    case 0xfc:          // iso-8859-1 encoding for 'ü'
                        return KEY_uuml;
    case 0xc3bb:        // utf-8 encoding for 'û'
    case 0xfb:          // iso-8859-1 encoding for 'û'
                        return KEY_ucirc;
    case 'v':           return KEY_v;
    case 'w':           return KEY_w;
    case 'x':           return KEY_x;
    case 'y':           return KEY_y;
    case 'z':           return KEY_z;

    case 'A':           return KEY_A;
    case 'B':           return KEY_B;
    case 'C':           return KEY_C;
    case 'D':           return KEY_D;
    case 'E':           return KEY_E;
    case 'F':           return KEY_F;
    case 'G':           return KEY_G;
    case 'H':           return KEY_H;
    case 'I':           return KEY_I;
    case 'J':           return KEY_J;
    case 'K':           return KEY_K;
    case 'L':           return KEY_L;
    case 'M':           return KEY_M;
    case 'N':           return KEY_N;
    case 'O':           return KEY_O;
    case 'P':           return KEY_P;
    case 'Q':           return KEY_Q;
    case 'R':           return KEY_R;
    case 'S':           return KEY_S;
    case 'T':           return KEY_T;
    case 'U':           return KEY_U;
    case 'V':           return KEY_V;
    case 'W':           return KEY_W;
    case 'X':           return KEY_X;
    case 'Y':           return KEY_Y;
    case 'Z':           return KEY_Z;

    case '+':           return PLUS;
    case '-':           return MINUS;
    case '*':           return STAR;
    case '/':           return SLASH;
    case '<':           return LOWER_THAN;
    case '>':           return GREATER_THAN;
    case '=':           return EQUAL;
    case '%':           return PERCENT;

    case '_':           return UNDERSCORE;
    case '@':           return AROBASE;
    case '#':           return SHARP;
    case '&':           return AMPERCENT;
    case '$':           return DOLLAR;
    case '\\':          return BACKSLASH;
    case '|':           return PIPE;
    case '~':           return TILDE;
    case 0xc2b2:        // utf-8 encoding for '²'
    case 0xb2:          // iso-8859-1 encoding for '²'
                        return SQUARE;
    case 0xe282ac:      // utf-8 encoding (without BOM) for '€'
    case 0xc280:        // utf-8 (BOM) encoding for '€'
    case 0x80:          // iso-8859-1 encoding for '€'
                        return EURO;
    case 0xc2a3:        // utf-8 encoding for '£'
    case 0xa3:          // iso-8859-1 encoding for '£'
                        return POUND;
    case 0xc2b5:        // utf-8 encoding for 'µ'
    case 0xb5:          // iso-8859-1 encoding for 'µ'
                        return MICRO;
    case 0xc2a7:        // utf-8 encoding for '§'
    case 0xa7:          // iso-8859-1 encoding for '§'
                        return PARAGRAPH;
    case 0xc2b0:        // utf-8 encoding for '°'
    case 0xb0:          // iso-8859-1 encoding for '°'
                        return DEGREE;
    case 0xc2a9:        // utf-8 encoding for '©'
    case 0xa9:          // iso-8859-1 encoding for '©'
                        return COPYRIGHT;

    case '?':   return QUESTION;
    case '!':   return EXCLAMATION;
    case ':':   return COLUMN;
    case '.':   return DOT;
    case ',':   return COMA;
    case ';':   return SEMI_COLUMN;
    case '\'':  return QUOTE;
    case '"':   return DOUBLE_QUOTE;

    case '(':   return OPENING_BRACE;
    case ')':   return CLOSING_BRACE;
    case '{':   return OPENING_CURLY_BRACE;
    case '}':   return CLOSING_CURLY_BRACE;
    case '[':   return OPENING_BRACKET;
    case ']':   return CLOSING_BRACKET;

    default:
        // Unrecognized character.
        return NULL_KEY;
    }
}

const KEY StringDecoder::GetKey(void)
{
    if (! m_tkString.IsEmpty())
    {
        const char c_Char = m_tkString.GetChar(0);
        m_tkString = m_tkString.SubString(1, m_tkString.GetLength() - 1);
        const KEY e_Key = Decode(c_Char);
        if (e_Key == FEED_MORE)
        {
            // Recursive call
            return GetKey();
        }
        return e_Key;
    }
    return NULL_KEY;
}
