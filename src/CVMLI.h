#ifndef CVML_I_H
#define CVML_I_H

#include <CVML.h>
#include <CVMLOpCodes.h>
#include <CUtil.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <CStrParse.h>
#include <CMathGen.h>
#include <CReadLine.h>

#define SET_FLAG(v,f)  ((v) |=  (f))
#define RST_FLAG(v,f)  ((v) &= ~(f))
#define TST_FLAG(v,f) (((v) &   (f)) ? true : false)

#define SET_BIT(v,n) SET_FLAG(v,1<<(n))
#define RST_BIT(v,n) RST_FLAG(v,1<<(n))
#define TST_BIT(v,n) TST_FLAG(v,1<<(n))

#define RESET_BIT(v,n) RST_BIT(v,n)
#define IS_BIT(v,n)    TST_BIT(v,n)

#endif
