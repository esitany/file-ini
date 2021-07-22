/**
* @file file_ini_.h
* @brief This file declares control ini file
* @author yikim
* @version 1.0
* @date 2014-12-10
*/

#ifndef _FILE_INI_HEADER
#define _FILE_INI_HEADER

#include <stdint.h>

/** UTF-8 Description
 * +---------+----------------------+--------+---------+---------+---------+---------+---------+---------+
 * | Bits of |       Code point     |  Bytes |         |         |         |         |         |         |
 * |   code  +-----------+----------+   in   |  Byte 1 |  Byte 2 |  Byte 3 |  Byte 4 |  Byte 5 |  Byte 6 |
 * |  point  |    First  |   Last   |sequence|         |         |         |         |         |         |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |    7	 |    U+0000 |    U+007F|    1   | 0xxxxxxx|                                                 |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |   11    |    U+0080 |    U+07FF|    2   | 110xxxxx| 10xxxxxx|                                       |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |   16    |    U+0800 |    U+FFFF|    3   | 1110xxxx| 10xxxxxx| 10xxxxxx|                             |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |   21    |   U+10000 |  U+1FFFFF|    4   | 11110xxx| 10xxxxxx| 10xxxxxx| 10xxxxxx|                   |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |The patterns below are not part of UTF-8, but were art of thefirst specfication.                     |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |   26    |  U+200000 | U+3FFFFFF|    5   | 111110xx| 10xxxxxx| 10xxxxxx| 10xxxxxx| 10xxxxxx|         |
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 * |   31    | U+4000000 |U+7FFFFFFF|    6   | 1111110x| 10xxxxxx| 10xxxxxx| 10xxxxxx| 10xxxxxx| 10xxxxxx|
 * +---------+-----------+----------+--------+---------+---------+---------+---------+---------+---------+
 */

typedef enum ENUM_INI_TYPE {
    E_INI_T_BLANK    = 0,  // balnk link
    E_INI_T_COMMENT  ,     // comment(# or ;)
    E_INI_T_SECTION  ,     // section
    E_INI_T_PROPERTY ,     // property
    E_INI_T_UNKNOWN        // Not defined type...
} enFIType;

typedef struct STRUCT_INI_ELEMENT_CONFIG {
    uint32_t type    : 4; //  0: 3, Element node type
    uint32_t size    :12; //  4:15, Element node data size(length)
    uint32_t reserve :16;
} stFIECfg;

typedef struct STRUCT_INI_ELEMENT_NODE {
    stFIECfg  cfg;
    void     *value;
    void     *front;
    void     *next;
} stFINode;

typedef struct STRUCT_INI_HANDLE {
    stFINode  *head;
    stFINode  *tail;
} stFIHandle;

typedef struct STRUCT_INI_PROPERTY {
	char *key;
	char *val;
} stFIProperty;

typedef struct STRUCT_INI_SECTION {
	char       *name;
    stFIHandle *hIni;
} stFISection;


#define FI_LINE           "\r\n"
#define FI_BUFFER_SIZE    4096

stFIHandle *fiInit(void);
void        fiDestroy(stFIHandle *hIni);
void        fiShow(stFIHandle *hIni);

stFIHandle *fiFileRead(const char *file);
int         fiFileSave(const char *file, stFIHandle *hIni);

char *fiGet(stFIHandle *hIni, const char *sect, const char *key);
int   fiPut(stFIHandle *hIni, const char *sect, const char *key, const char *value);

#endif /* _FILE_INI_HEADER */
