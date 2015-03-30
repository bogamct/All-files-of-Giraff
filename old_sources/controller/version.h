#ifdef UNIT_TEST
    #define MYPROG
#else
    #define MYPROG PROGMEM
#endif

char MYPROG CONTROLLER_PREFIX     []="# Giraf ";
char MYPROG CONTROLLER_VERSION    []="1.2.196";
char MYPROG CONTROLLER_DATE       []="2013-01-14";
char MYPROG CONTROLLER_USER_NAME  []="Patrik";
char MYPROG CONTROLLER_USER_ABBREV[]="p";
char MYPROG CONTROLLER_COMMA      []=", ";
char MYPROG CONTROLLER_CRLF       []="\r\n";
