#include "HTML Entities.hpp"

std::vector<HTML_Entity> html_entities
{
   {"nbsp", " "}, //160: Non-break space
   {"iexcl", "!"}, //161: Inverted Exclamation Mark
   {"cent", "cents"}, //162: Cent sign
   {"pound", "pounds"}, //163: Pound sign
   {"curren", "currency"}, //164: Currency sign
   {"yen", "yen"}, //165: Yen Sign
   {"brvbar", "|"}, //166: Broken Bar
   {"sect", ""}, //167: Section
   {"uml", ""}, //168: Diaeresis
   {"copy", ""}, //169: Copyright Sign
   {"ordf", "a"}, //170: Feminine Ordinal Indicator
   {"laquo", "<<"}, //171: Left-Pointing Double Angle Quotation Mark
   {"not", "!"}, //172: Not Sign
   {"shy", ""}, //173: Soft Hyphen
   {"reg", "(R)"}, //174: Registered Sign
   {"macr", ""}, //175: Macron
   {"deg", "degrees"}, //176: Degree Sign
   {"plusmn", "+-"}, //177: Plus-Minus Sign
   {"sup2", "2"}, //178: Superscript Two
   {"sup3", "3"}, //179: Superscript Three
   {"acute", "`"}, //180: Acute Accent
   {"micro", "u"}, //181: Micro Sign
   {"para", ""}, //182: Pilcrow Sign
   {"middot", "-"}, //183: Middle Dot
   {"cedil", ""}, //184: Cedilla
   {"sup1", "1"}, //185: Superscript One
   {"ordm", "o"}, //186: Masculine Ordinal Indicator
   {"raquo", ">>"}, //187: Right-Pointing Double Angle Quotation Mark
   {"frac14", "1/4"}, //188: Vulgar Fraction One Quarter
   {"frac12", "1/2"}, //189: Vulgar Fraction One Half
   {"frac34", "3/4"}, //190: Vulgar Fraction Three Quarters
   {"iquest", "?"}, //191: Inverted Question Mark
   {"Agrave", "A"}, //192: Latin Capital Letter A With Grave
   {"Aacute", "A"}, //193: Latin Capital Letter A With Acute
   {"Acirc", "A"}, //194: Latin Capital Letter A With Circumflex
   {"Atilde", "A"}, //195: Latin Capital Letter A With Tilde
   {"Auml", "A"}, //196: Latin Capital Letter A With Diaeresis
   {"Aring", "A"}, //197: Latin Capital Letter A With Ring Above
   {"AElig", "AE"}, //198: Latin Capital Letter AE
   {"Ccedil", "C"}, //199: Latin Capital Letter C With Cedilla
   {"Egrave", "E"}, //200: Latin Capital Letter E With Grave
   {"Eacute", "E"}, //201: Latin Capital Letter E With Acute
   {"Ecirc", "E"}, //202: Latin Capital Letter E With Circumflex
   {"Euml", "E"}, //203: Latin Capital Letter E With Diaeresis
   {"Igrave", "I"}, //204: Latin Capital Letter I With Grave
   {"Iacute", "I"}, //205: Latin Capital Letter I With Acute
   {"Icirc", "I"}, //206: Latin Capital Letter I With Circumflex
   {"Iuml", "I"}, //207: Latin Capital Letter I With Diaeresis
   {"ETH", "D"}, //208: Latin Capital Letter Eth
   {"Ntilde", "N"}, //209: Latin Capital Letter N With Tilde
   {"Ograve", "O"}, //210: Latin Capital Letter O With Grave
   {"Oacute", "O"}, //211: Latin Capital Letter O With Acute
   {"Ocirc", "O"}, //212: Latin Capital Letter O With Circumflex
   {"Otilde", "O"}, //213: Latin Capital Letter O With Tilde
   {"Ouml", "O"}, //214: Latin Capital Letter O With Diaeresis
   {"times", "x"}, //215: Multiplication Sign
   {"Oslash", "O"}, //216: Latin Capital Letter O With Stroke
   {"Ugrave", "U"}, //217: Latin Capital Letter U With Grave
   {"Uacute", "U"}, //218: Latin Capital Letter U With Acute
   {"Ucirc", "U"}, //219: Latin Capital Letter U With Circumflex
   {"Uuml", "U"}, //220: Latin Capital Letter U With Diaeresis
   {"Yacute", "Y"}, //221: Latin Capital Letter Y With Acute
   {"THORN", ""}, //222: Latin Capital Letter Thorn
   {"szlig", ""}, //223: Latin Small Letter Sharp S
   {"agrave", "a"}, //224: Latin Small Letter A With Grave
   {"aacute", "a"}, //225: Latin Small Letter A With Acute
   {"acirc", "a"}, //226: Latin Small Letter A With Circumflex
   {"atilde", "a"}, //227: Latin Small Letter A With Tilde
   {"auml", "a"}, //228: Latin Small Letter A With Diaeresis
   {"aring", "a"}, //229: Latin Small Letter A With Ring Above
   {"aelig", "ae"}, //230: Latin Small Letter AE
   {"ccedil", "c"}, //231: Latin Small Letter C With Cedilla
   {"egrave", "e"}, //232: Latin Small Letter E With Grave
   {"eacute", "e"}, //233: Latin Small Letter E With Acute
   {"ecirc", "e"}, //234: Latin Small Letter E With Circumflex
   {"euml", "e"}, //235: Latin Small Letter E With Diaeresis
   {"igrave", "i"}, //236: Latin Small Letter I With Grave
   {"iacute", "i"}, //237: Latin Small Letter I With Acute
   {"icirc", "i"}, //238: Latin Small Letter I With Circumflex
   {"iuml", "i"}, //239: Latin Small Letter I With Diaeresis
   {"eth", "o"}, //240: Latin Small Letter Eth
   {"ntilde", "n"}, //241: Latin Small Letter N With Tilde
   {"ograve", "o"}, //242: Latin Small Letter O With Grave
   {"oacute", "o"}, //243: Latin Small Letter O With Acute
   {"ocirc", "o"}, //244: Latin Small Letter O With Circumflex
   {"otilde", "o"}, //245: Latin Small Letter O With Tilde
   {"ouml", "o"}, //246: Latin Small Letter O With Diaeresis
   {"divide", "/"}, //247: Division Sign
   {"oslash", "o"}, //248: Latin Small Letter O With Stroke
   {"ugrave", "u"}, //249: Latin Small Letter U With Grave
   {"uacute", "u"}, //250: Latin Small Letter U With Acute
   {"ucirc", "u"}, //251: Latin Small Letter U With Circumflex
   {"uuml", "u"}, //252: Latin Small Letter U With Diaeresis
   {"yacute", "y"}, //253: Latin Small Letter Y With Acute
   {"thorn", ""}, //254: Latin Small Letter Thorn
   {"yuml", "y"}, //255: Latin Small Letter Y With Diaeresis

   {"quot", "\""}, //34: Quotation Mark
   {"amp", "&"}, //38: Ampersand
   {"apos", "'"}, //39: Apostrophe
   {"lt", "<"}, //60: Less-Than Sign
   {"gt", ">"}, //62: Greater-Than Sign
};