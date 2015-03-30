#include "variables.h"

void setI( Setting* p_setting, char* p_name, const char* p_description, s32 p_min, s32 p_max ) {
  p_setting->m_name = p_name;
  p_setting->m_description = p_description;
  p_setting->m_isFloat = false;
  p_setting->m_min.i = p_min;
  p_setting->m_max.i = p_max;
}

void setF( Setting* p_setting, char* p_name, const char* p_description, float p_min, float p_max ) {
  p_setting->m_name = p_name;
  p_setting->m_description = p_description;
  p_setting->m_isFloat = true;
  p_setting->m_min.f = p_min;
  p_setting->m_max.f= p_max;
}

