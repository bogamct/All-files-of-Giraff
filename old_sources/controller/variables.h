#ifndef VARIABLES_H
#define VARIABLES_H

#include <stdbool.h>
#include <common/vartypes.h>

union Num_ {
  float f;
  s32 i;
};

typedef union Num_ Num;

struct Setting_ {
  char *m_name;
  const char *m_description;
  bool m_isFloat;
  Num m_min;
  Num m_max;
  Num m_lastDisplayedValue;
};

typedef struct Setting_ Setting;

void setI( Setting* p_setting, char* p_name, const char* p_description, s32 p_min, s32 p_max );
void setF( Setting* p_setting, char* p_name, const char* p_description, float p_min, float p_max );

#endif
