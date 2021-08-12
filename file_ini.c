/**
* @file file_ini.c
* @brief This file is implements ini file
* @author yikim
* @version 1.0
* @date 2014-12-10
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <errno.h>

#include "file_ini.h"

#if defined(ENABLE_LOG_TRACE)
  #include "log_trace.h"

  #define TAG_NAME     "UTIL_INI"

  #define lDbg(...)        ltMsg(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
  #define lWrn(...)        ltMsg(TAG_NAME, LT_WARN,  __FILE__, __LINE__, __VA_ARGS__)
  #define lErr(...)        ltMsg(TAG_NAME, LT_ERR,   __FILE__, __LINE__, __VA_ARGS__)

  #define hexdump(T, P, S) ltDump(TAG_NAME, LT_DEBUG, __FILE__, __LINE__, P, S, T);

#else
  #define lDbg(fmt, ...) { \
    fprintf(stdout, "INI(%5d) > " fmt "\n", __LINE__,  ##__VA_ARGS__); \
    fflush(stdout); \
  }

  #define lWrn(fmt, ...) { \
    fprintf(stdout, "\x1b[33mINI(%5d) > " fmt "\x1B[0m\n", __LINE__,  ##__VA_ARGS__); \
    fflush(stdout); \
  }

  #define lErr(fmt, ...) { \
    fprintf(stdout, "\x1b[31mINI(%5d) > " fmt "[E=%s(%d)]\x1B[0m\n", __LINE__,  ##__VA_ARGS__, strerror(errno), errno); \
    fflush(stdout); \
  }

  void fiHexdump(int line, const char *title, void *pack, int size);

  #define hexdump(T, P, S) fiHexdump(__LINE__, T, P, S)
#endif

void fiHexdump(int line, const char *title, void *pack, int size)
{
    int   idx = 0;

    char strTmp[4]    = {"\0"};
    char strAscii[32] = {"\0"};
    char strDump[64]  = {"\0"};
    char *dump        = NULL;

    dump = (char *)pack;
    if ((size > 0) && (pack != NULL)) {
        fprintf(stdout, "INI(%5d) >  ***** %s %d bytes *****\n", line, (title == NULL) ? "None" : title, size);
        fflush(stdout);

        memset(strDump, 0, 64);
        memset(strAscii, 0, 32);

        for(idx = 0; idx < size; idx++) {
            if    ((0x1F < dump[idx]) && (dump[idx] < 0x7F) ) { strAscii[idx & 0x0F] = dump[idx]; }
            else                                              { strAscii[idx & 0x0F] = 0x2E;
            }

            snprintf(strTmp, 4, "%02X ", (unsigned char)dump[idx]);
            strcat(strDump, strTmp);
            if( (idx != 0) && ((idx & 0x03) == 0x03)) { strcat(strDump, " "); }

            if((idx & 0x0F) == 0x0F) {
                fprintf(stdout, "%12s <0x%04X> %s%s\n", "", (idx & 0xFFF0), strDump, strAscii);
                fflush(stdout);
                memset(strDump, 0, 64);
                memset(strAscii, 0, 32);
            }
        }

        if (((size - 1) & 0x0F) != 0x0F) {
            for(idx = strlen(strDump) ; idx < 52; idx++) {
                strDump[idx] = 0x20;
            }
            fprintf(stdout, "%12s <0x%04X> %s%s\n", "", (size & 0xFFF0), strDump, strAscii);
            fflush(stdout);
        }

        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

stFIHandle *fiInit(void)
{
    stFIHandle *hIni = NULL;

    hIni = (stFIHandle *)malloc(sizeof(stFIHandle) + 1);
    if (hIni == NULL) {
        lErr("Allocate failed...");
    }
    else {
        hIni->head = NULL;
        hIni->tail = NULL;
    }

    return hIni;
}

void fiDestroy(stFIHandle *hIni)
{
    stFINode     *head = NULL;
    stFISection  *sect = NULL;
    stFIProperty *prop = NULL;

    if (hIni) {
        while ( (head = (stFINode *)hIni->head) != NULL) {
            switch(head->cfg.type) {
            case E_INI_T_SECTION  :
                sect = (stFISection *)head->value;
                if (sect->name) { free(sect->name); }

                fiDestroy(sect->hIni);
                free(sect);
                break;

            case E_INI_T_PROPERTY :
                prop = (stFIProperty *)head->value;
                if (prop->key) { free(prop->key); }
                if (prop->val) { free(prop->val); }

                free(prop);
                break;

            case E_INI_T_UNKNOWN  :
            case E_INI_T_BLANK    :
            case E_INI_T_COMMENT  :
            default               :
                if (head->value) { free(head->value); }
                break;
            }

            hIni->head = (stFINode *)head->next;
            if ( hIni->head ) {
                hIni->head->front = NULL;
            }

            free(head);
        }

        free(hIni);
    }
}

void fiShow(stFIHandle *hIni)
{
    stFINode     *head = NULL,
                 *next = NULL;
    stFISection  *sect = NULL;
    stFIProperty *prop = NULL;

    if (hIni) {
        head = (stFINode *)hIni->head;

        while ( head != NULL) {
            next = head->next;
            switch(head->cfg.type) {
            case E_INI_T_SECTION  :
                sect = (stFISection *)head->value;
                lDbg("<INI|%12s> %s", "Section", sect->name);
                fiShow(sect->hIni);
                break;

            case E_INI_T_PROPERTY :
                prop = (stFIProperty *)head->value;
                lDbg("<INI|%12s> %s = %s", "Property", prop->key, prop->val);
                break;

            case E_INI_T_BLANK    : lDbg("<INI|%12s>", "Black"); break;
            case E_INI_T_COMMENT  : lDbg("<INI|%12s> %s", "Comment", (char *)head->value); break;
            case E_INI_T_UNKNOWN  :
            default               : lDbg("<INI|%12s>", "Unknown"); break;
                break;
            }

            head = next;
        }
    }
}

void *fiMakeSection(const char *str, size_t size)
{
    stFISection *sect = NULL;

    sect = (stFISection *)malloc( sizeof(stFISection) );
    if ( sect == NULL ) {
        lErr("Allocate failed...");
    }
    else {
        if (size > 0) {
            sect->name = (char *)malloc( size + 1 );
            if ( sect->name == NULL ) {
                lErr("Allocate failed...");
                free(sect);
                sect = NULL;
            }
        }

        if (sect != NULL) {
            sect->hIni = (stFIHandle *)fiInit();
            if(sect->hIni == NULL) {
                lErr("fiInit() failed...");
                free(sect);
                sect = NULL;
            }
        }

        if (sect != NULL) {
            if (size > 0) {
                snprintf(sect->name, size + 1, "%s", str);
            }
            sect->hIni->head = NULL;
            sect->hIni->tail = NULL;
        }
    }

    return sect;
}

void *fiMakeProperty(const char *key, char *value)
{
    size_t length = 0;

    stFIProperty *prop = NULL;

    prop = (stFIProperty *)malloc( sizeof(stFIProperty) );
    if ( prop == NULL ) {
        lErr("Allocate failed...");
    }
    else {
        prop->key = NULL;
        prop->val = NULL;

        if (key == NULL) {
            lWrn("Porpery key is Not exist!!!");
            free(prop);
            prop = NULL;
        }
        else {
            length = strlen(key) + 1;
            prop->key = (char *)malloc(length);
            if (prop->key == NULL) {
                lErr("Allocate failed...");

                free(prop);
                prop = NULL;
            }
            else {
                snprintf(prop->key, length, "%s", key);

                if (value) {
                    length = strlen(value) + 1;
                    prop->val = (char *)malloc(length);
                    if (prop->val == NULL) {
                        lErr("Allocate failed...");

                        free(prop->key);
                        free(prop);
                        prop = NULL;
                    }
                    else {
                        snprintf(prop->val, length, "%s", value);
                    }
                }
            }
        }
    }

    return prop;
}


void *fiMakePropertyFromString(char *str, size_t size)
{

    size_t offHead = 0,
           offTail = 0;

    char *key    = NULL,
         *val    = NULL,
         *equals = NULL;

    stFIProperty *prop = NULL;

    if (str == NULL) {
        lWrn("Ini Property string is not exist!!!");
    }
    else {
        equals = strchr(str, '=');

        // Parsing Ini Property Key
        offHead = 0;
        offTail = (size_t)(equals - str);

        while( (offHead < offTail) && (str[offHead] == ' ')) {
            offHead = offHead + 1;
        }

        do {
            str[offTail--] = 0x00;
        } while( (offHead < offTail) && (str[offTail] == ' ') );

        if (offHead < offTail) key = (char *)&str[offHead];

        // Parsing Ini Property value
        offHead = (size_t)(equals - str) + 1;
        offTail = size;

        while( (offHead < offTail) && (str[offHead] == ' ')) {
            offHead = offHead + 1;
        }
        while( (offHead < offTail) && (str[offTail] == ' ') ) {
            str[offTail--] = 0x00;
        }

        if (offHead < offTail) val = (char *)&str[offHead];

        prop = (stFIProperty *)fiMakeProperty((const char *)key, val);
    }

    return prop;
}

void *fiMakeCommand(const char *str, size_t size)
{
    char *ptr = NULL;

    if (size < 1) {
        lWrn("Comment size is invalid...%d", (int)size);
    }
    else {
        ptr = (char *)malloc( size + 1 );
        if ( ptr == NULL ) {
            lErr("Allocate failed...");
        }
        else {
            snprintf(ptr, size + 1, "%s", str);
        }
    }

    return ptr;
}

int fiInsertNode(stFIHandle *hIni, stFINode *node)
{
    int ret = 0;

    stFINode *tail = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else if (node == NULL) {
        lWrn("Is Not exist Insert Ini Node!!!");
        ret = -EINVAL;
    }
    else {
        tail = (stFINode *)hIni->tail;

        node->front = tail;

        if (hIni->head == NULL) {
            hIni->head = node;
        }

        if (tail != NULL) {
            tail->next = node;
        }

        hIni->tail = node;
    }

    return ret;
}

int fiInsert(stFIHandle *hIni, int type, const char *str, size_t size)
{
    int ret = 0;

    stFINode *node  = NULL;
    void     *value = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else {
        switch(type) {
        case E_INI_T_SECTION  :
            value = fiMakeSection(str, size);
            if (value == NULL) {
                lWrn("fiMakeSection() failed!!!");
                ret = -EFAULT;
            }
            break;
        case E_INI_T_PROPERTY :
            value = fiMakePropertyFromString((char *)str, size);
            if (value == NULL) {
                lWrn("fiMakePropertyFromString() failed!!!");
                ret = -EFAULT;
            }
            break;
        case E_INI_T_COMMENT  :
            value = fiMakeCommand(str, size);
            if (value == NULL) {
                lWrn("fiMakeSection() failed!!!");
                ret = -EFAULT;
            }
            break;
        case E_INI_T_BLANK    : break;
        case E_INI_T_UNKNOWN  :
        default               :
            lWrn("Not support type...%d", type);
            ret = -EINVAL;
            break;
        }
    }

    if (ret >= 0) {
        node = (stFINode *)malloc(sizeof(stFINode));
        if (node == NULL) {
            lErr("Allocate failed...");
            if (value != NULL) { free(value); }
            ret = -EFAULT;
        }
        else {
            node->cfg.type = type;

            node->value = value;
            node->front = NULL;
            node->next  = NULL;

            ret = fiInsertNode(hIni, node);
        }
    }

    return ret;
}

int fiInsertComment(stFIHandle *hIni, char *str, size_t size)
{
    int ret = 0;

    size_t offset = 0;

    char *tok = NULL,
         *old = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else if (str == NULL) {
        lWrn("Text line is empty!!!");
        ret = -EINVAL;
    }
    else {
        tok = strtok_r(str, "#;", &old);
        if ( tok ) {
            offset = tok - str;
//              lDbg("%s() %s[Offset=(%d, L=%d, O=%d)",
//                  __FUNCTION__, tok, (int)(tok - str), (int)strlen(tok), (int)(old - str));

            while( (offset < size) && (str[offset] == ' ')) {
                offset = offset + 1;
            }

            if (offset < size) {
                ret = fiInsert(hIni, E_INI_T_COMMENT, &str[offset], strlen(&str[offset]));
            }
            else {
                lWrn("Comment is empty!!!");
                ret = 0;
            }
        }
    }

    return ret;
}

stFISection *fiFindSection(stFIHandle *hIni, const char *key)
{
    size_t lenKey = 0;
    stFISection *sect   = NULL,
                *fiSect = NULL;

    stFINode    *head = NULL,
                *next = NULL;

    if (hIni) {
        lenKey = strlen(key);

        head = (stFINode *)hIni->head;
        while ( (head != NULL) && (sect == NULL) ) {
            next = head->next;
            if( head->cfg.type == E_INI_T_SECTION ) {
                fiSect = (stFISection *)head->value;

                if (fiSect->name == NULL) {
                    if (lenKey == 0) {
                        sect = fiSect;
                    }
                }
                else {
                    if ( (lenKey > 0)
                      && (strcmp((const char *)fiSect->name, (const char *)key) == 0) ) {
                        sect = fiSect;
                    }
                }
            }
            head = next;
        }

    }

    return sect;
}

stFISection *fiSearchSection(stFIHandle *hIni, const char *key)
{
    stFINode    *node = NULL;
    stFISection *sect = NULL;

    if (hIni) {
        sect = fiFindSection(hIni, key);
        if (sect == NULL) {
            sect = fiMakeSection(key, strlen(key));
            if (sect == NULL) {
                lWrn("fiMakeSection() failed!!!");
            }
            else {
                node = (stFINode *)malloc(sizeof(stFINode));
                if (node == NULL) {
                    lErr("Allocate failed...");
                    if (sect != NULL) {
                        free(sect);
                        sect = NULL;
                    }
                }
                else {
                    node->cfg.type = E_INI_T_SECTION;

                    node->value = sect;
                    node->front = NULL;
                    node->next  = NULL;

                    fiInsertNode(hIni, node);

                }
            }
        }
    }

    return sect;
}

int fiInsertSection(stFIHandle *hIni, char *str, size_t size)
{
    int ret = 0;

    char *tok = NULL,
         *old = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else if (str == NULL) {
        lWrn("Text line is empty!!!");
        ret = -EINVAL;
    }
    else {
        tok = strtok_r(str, "[]", &old);
        if ( tok ) {
            if (fiSearchSection(hIni, tok) == NULL) {
                ret = -EFAULT;
            }
        }
    }

    return ret;
}

int fiInsertProperty(stFIHandle *hIni, char *str, size_t size)
{
    int ret = 0;

    stFISection *sect = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else if (str == NULL) {
        lWrn("Text line is empty!!!");
        ret = -EINVAL;
    }
    else {
        if (hIni->tail) {
            if (hIni->tail->cfg.type == E_INI_T_SECTION) {
                sect = (stFISection *)hIni->tail->value;
            }
        }

        if (sect == NULL) {
            sect = fiSearchSection(hIni, "");
            if (sect == NULL) {
                lWrn("fiSearchSection() failed!!!");
                ret = -EFAULT;
            }
        }

        if (sect) {
            fiInsert(sect->hIni, E_INI_T_PROPERTY, str, size);
        }
    }

    return ret;
}

int fiProcType(const char *str, size_t size)
{
    int ret = E_INI_T_UNKNOWN;

    char *semicolon = strchr(str, ';');
    char *hash      = strchr(str, '#');

    char *square  = strchr(str, '[');
    char *equals  = strchr(str, '=');
    char *comment = NULL;

    if (semicolon || hash) {
        if (semicolon && hash) { comment = (semicolon < hash) ? semicolon : hash; }
        else if (semicolon)    { comment = semicolon; }
        else                   { comment = hash; }
    }

    if ( square && (strchr(str, ']') == NULL) ) {
        square = NULL;
    }

    if ( square ) {
        if ( comment ) {
            ret = ( square < comment ) ? E_INI_T_SECTION : E_INI_T_COMMENT;
        }
        else {
            ret = E_INI_T_SECTION;
        }
    }
    else if ( equals ) {
        if ( comment ) {
            ret = ( equals < comment ) ? E_INI_T_PROPERTY : E_INI_T_COMMENT;
        }
        else {
            ret = E_INI_T_PROPERTY;
        }
    }
    else {
        if ( comment ) {
            ret = E_INI_T_COMMENT;
        }
    }

    return ret;
}

stFIHandle *fiProcRead(int fd)
{
    size_t offset = 0,
           offOld = 0,
           offTok = 0;

    size_t szRead = 0,
           szLine = 0;

    char delmit[4] = {"\r\n"};
	char ptr[FI_BUFFER_SIZE] = {0,};

    char *old = NULL,
         *tok = NULL;

    stFIHandle *hIni = NULL;

	if (fd == -1) {
        lWrn("Ini file descriptor invaild!!!");
    }
    else {
		lseek(fd, 0, SEEK_SET);

        hIni = fiInit();
        while (hIni
            && ((szRead = read(fd, &ptr[offset], FI_BUFFER_SIZE - offset)) > 0) ) {
//              lDbg("read size = %d, offset=%d", (int)szRead, (int)offset);
            szRead = szRead + offset;
//              hexdump("Before test", ptr, szRead);

            offset = 0;
            tok = strtok_r(&ptr[0], delmit, &old);
            while (tok != NULL) {
                offTok = tok - ptr;
                offOld = old - ptr;

//                  lDbg("Offset=(%d, R=%d, T=%d, O=%d)", (int)offset, (int)szRead, (int)offTok, (int)offOld);
                while (offset < offTok) {
                    if ( (ptr[offset] == 0x0D) // CR \r
                      || (ptr[offset] == 0x0A) ) { // LF \n
//                          lDbg("E_INI_T_BLANK");
                        fiInsert(hIni, E_INI_T_BLANK, NULL, 0);

                        if ( (ptr[offset + 0] == 0x0D)
                          && (ptr[offset + 1] == 0x0A) ) { offset = offset + 2; }
                        else                             { offset = offset + 1; }
                    }
                    else {
                        lWrn("Unknown Ignore %c", (char)ptr[offset]);
                        offset = offset + 1;
                    }
                }

                if (ptr[offOld - 1] == 0) {
                    szLine = strlen(tok);
//                      lDbg("Line(%d) = %s", (int)szLine, tok);

                    switch( fiProcType(tok, szLine) ) {
                    case E_INI_T_SECTION  : fiInsertSection(hIni, tok, szLine);  break;
                    case E_INI_T_PROPERTY : fiInsertProperty(hIni, tok, szLine); break;
                    case E_INI_T_COMMENT  : fiInsertComment(hIni, tok, szLine);  break;
                    case E_INI_T_BLANK    :
                    case E_INI_T_UNKNOWN  :
                    default               : break;
                    }

                    offset = offOld + ((ptr[offOld] == 0x0A) ? 1 : 0);
                    tok = (offOld < szRead) ? strtok_r(NULL, delmit, &old) : NULL;
                }
                else {
                    tok = NULL;
                }
            }

            if (szRead > offset) {
                offTok = szRead - offset;
//              hexdump("After test", ptr, szRead);
//              lDbg("read=%d, elapse=%d, reamin=%d", (int)szRead, (int)offset, (int)offTok );

                memmove(&ptr[0], &ptr[offset], offTok);
                offset = offTok;
            }
            else {
                offset = 0;
            }
        }
    }

    return hIni;
}

int fiProcSave(int fd, stFIHandle *hIni)
{
    int ret = 0;

	char ptr[FI_BUFFER_SIZE] = {0,};

    stFINode     *head = NULL,
                 *next = NULL;
    stFISection  *sect = NULL;
    stFIProperty *prop = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else if (fd == -1) {
        lWrn("Save file descriptor invaoild!!!");
        ret = -EINVAL;
    }
    else {
        head = (stFINode *)hIni->head;
        while ( head != NULL ) {
            next = head->next;
            switch(head->cfg.type) {
            case E_INI_T_SECTION  :
                sect = (stFISection *)head->value;
                if (sect->name) {
                    snprintf(ptr, FI_BUFFER_SIZE, "[%s]" FI_LINE, sect->name);
                    write(fd, ptr, strlen(ptr));
                }
                fiProcSave(fd, sect->hIni);
                break;

            case E_INI_T_PROPERTY :
                prop = (stFIProperty *)head->value;
                snprintf(ptr, FI_BUFFER_SIZE, "%s = %s" FI_LINE, prop->key, prop->val);
                write(fd, ptr, strlen(ptr));
                break;

            case E_INI_T_COMMENT  :
                snprintf(ptr, FI_BUFFER_SIZE, "; %s" FI_LINE, (char *)head->value);
                write(fd, ptr, strlen(ptr));
                break;

            case E_INI_T_BLANK    : write(fd, FI_LINE, strlen(FI_LINE)); break;
            case E_INI_T_UNKNOWN  :
            default               : break;
            }

            head = next;
        }
    }

    return ret;
}

int fiFileSave(const char *file, stFIHandle *hIni)
{
    int fd  = -1,
        ret = 0;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else {
        fd = open(file, O_RDWR | O_CREAT | O_TRUNC, (mode_t)00666);
        if (fd == -1) {
            lErr("%s open() failed...", file);
            ret = -EFAULT;
        }
        else {
		    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
                lErr("%s lseek( 0, SEEK_SET) failed...", file);
            }

            fiProcSave(fd, hIni);

            if (fsync(fd) == -1) {
                lErr("%s fsync() failed...", file);
            }

            if (close(fd) == -1) {
                lErr("%s close() failed...", file);
            }
        }
    }

    return ret;
}

stFIHandle *fiFileRead(const char *file)
{
    int fd  = -1;

    struct stat sb;

    stFIHandle *hIni = NULL;

    if (stat(file, &sb) == -1) {
        lErr("stat(%s, ) failed...", file);
    }
    else {
        if ( S_ISREG(sb.st_mode) ) {
            fd = open(file, O_RDONLY, (mode_t)00666);
            if (fd == -1) {
                lErr("%s open failed...", file);
            }
            else {
                hIni = fiProcRead(fd);
                close(fd);
            }
        }
        else {
            lWrn("%s is not file Regular file", file);
        }
    }

    return hIni;
}

stFIProperty *fiFindProperty(stFIHandle *hIni, const char *key)
{
    stFINode *head = NULL,
             *next = NULL;

    stFIProperty *prop   = NULL,
                 *fiProp = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
    }
    else {
        head = (stFINode *)hIni->head;
        while ( (head != NULL) && (prop == NULL) ) {
            next = head->next;
            if( head->cfg.type == E_INI_T_PROPERTY ) {
                fiProp = (stFIProperty *)head->value;

                if (strcmp((const char *)fiProp->key, (const char *)key) == 0) {
                    prop = fiProp;
                }
            }
            head = next;
        }
    }

    return prop;
}

char *fiGetPropertyData(stFIHandle *hIni, const char *key)
{
    char     *value = NULL;

    stFIProperty *prop = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
    }
    else {
        prop = fiFindProperty(hIni, key);
        if (prop) {
            value = (char *)prop->val;
        }
    }

    return value;
}

char *fiGet(stFIHandle *hIni, const char *sect, const char *key)
{
    char *value = NULL;

    stFISection *fiSect = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
    }
    else {
        fiSect = (stFISection *)fiFindSection(hIni, sect);
        if (fiSect) {
            value = (char *)fiGetPropertyData(fiSect->hIni, key);
        }
    }

    return value;
}

int fiPut(stFIHandle *hIni, const char *sect, const char *key, const char *value)
{
    int ret = 0;

    size_t length = 0;

    char *ptr = NULL;

    stFINode *node = NULL;

    stFISection  *fiSect = NULL;
    stFIProperty *fiProp = NULL;

    if (hIni == NULL) {
        lWrn("Is Not exist handle!!!");
        ret = -EINVAL;
    }
    else {
        length = strlen(value) + 1;
        fiSect = (stFISection *)fiSearchSection(hIni, sect);
        if (fiSect) {
            fiProp = fiFindProperty(fiSect->hIni, key);
            if (fiProp == NULL) {
                fiProp = (stFIProperty *)fiMakeProperty(key, (char *)value);
                if (fiProp == NULL) {
                    lWrn("fiMakeProperty() failed!!!");
                    ret = -EFAULT;
                }
                else {
                    node = (stFINode *)malloc(sizeof(stFINode));
                    if (node == NULL) {
                        lErr("Allocate failed...");
                        ret = -EFAULT;
                    }
                    else {
                        node->cfg.type = E_INI_T_PROPERTY;

                        node->value = fiProp;
                        node->front = NULL;
                        node->next  = NULL;

                        ret = fiInsertNode(fiSect->hIni, node);
                    }
                }

            }
            else {
                if (length > 1) {
                    ptr = (char *)malloc(length);
                    if (ptr == NULL) {
                        lErr("Allocate failed...");
                        ret = -EFAULT;
                    }
                    else {
                        snprintf(ptr, length, "%s", value);
                    }
                }

                if (fiProp->val) { free(fiProp->val); }

                fiProp->val = ptr;
            }
        }
    }

    return ret;
}

