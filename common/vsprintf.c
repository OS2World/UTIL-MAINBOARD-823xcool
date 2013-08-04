/* $Id: vsprintf.c,v 1.2 2002/06/15 20:19:13 root Exp $ */

/* Derived from Ext2FS driver for OS/2; this is an adaptation for
   the 16-bit model. */

#include <stdarg.h>

/* Hacking for macro implementations */
#define is_digit(c) ((c)>='0'&&(c)<='9')
#define islower(c)  ((c)>='a'&&(c)<='z')
#define isxdigit(c) (is_digit(c)||(c)>='A'&&(c)<='F'||(c)>='a'&&(c)<='f')
#define toupper(c)  (c-32) /* Hack (see below) */

/* Length-limited strlen() */

static int strnlen(const char far * s, int count)
{
 const char far *sc;

 for (sc=s; *sc!='\0'&&count--; ++sc)
  ;
 return(sc-s);
}

/* Hex representation of digits */

static char adigit(unsigned long n, int is_uc)
{
 if(n<10)
  return('0'+n);
 n-=10;
 return((is_uc?'A':'a')+n);
}

/* Q'n'D strtoul() implementation */

unsigned long simple_strtoul(const char far *cp, char far **endp, unsigned int base)
{
 unsigned long result=0, value;

 if(!base)
 {
  base=10;
  if(*cp=='0')
  {
   base=8;
   cp++;
   if((*cp=='x')&&isxdigit(cp[1]))
   {
    cp++;
    base=16;
   }
  }
 }
 while(isxdigit(*cp)&&(value=is_digit(*cp)?
                       *cp-'0':
                       (islower(*cp)?toupper(*cp):*cp)-'A'+10)<base)
 {
  result=result*base+value;
  cp++;
 }
 if(endp)
  *endp=(char far *)cp;
 return(result);
}

/* Convert digits and skip over them */

static int skip_atoi(const char **s)
{
 int i=0;

 while(is_digit(**s))
  i=i*10+*((*s)++)-'0';
 return(i);
}

#define ZEROPAD                    1    /* pad with zero */
#define SIGN                       2    /* unsigned/signed long */
#define PLUS                       4    /* show plus */
#define SPACE                      8    /* space if plus */
#define LEFT                      16    /* left justified */
#define SPECIAL                   32    /* 0x */
#define LARGE                     64    /* use 'ABCDEF' instead of 'abcdef' */
#define FAR_STR                  128    /* Far strings (Fs) */

/* Number representation routine */

static char *number(char *str, long num, int base, int size, int precision, int type)
{
 char c,sign,tmp[66];
 int i;
 int ucase_dig=0;

 if(type&LARGE)
  ucase_dig=1;
 if(type&LEFT)
  type&=~ZEROPAD;
 if(base<2||base>36)
  return(0);
 c=(type&ZEROPAD)?'0':' ';
 sign=0;
 if(type&SIGN)
 {
  if(num<0)
  {
   sign='-';
   num=-num;
   size--;
  }
  else if(type&PLUS)
  {
   sign='+';
   size--;
  }
  else if(type&SPACE)
  {
   sign=' ';
   size--;
  }
 }
 if(type&SPECIAL)
 {
  if(base==16)
   size-=2;
  else if(base==8)
   size--;
 }
 i=0;
 if(num==0)
  tmp[i++]='0';
 else while (num!=0)
 {
  unsigned long __res;

  __res=((unsigned long)num)%(unsigned long)base;
  num=((unsigned long)num)/(unsigned long)base;
  tmp[i++]=adigit(__res, ucase_dig);
 }
 if(i>precision)
  precision=i;
 size-=precision;
 if(!(type&(ZEROPAD+LEFT)))
 {
  while(size-->0)
   *str++=' ';
 }
 if(sign)
  *str++=sign;
 if(type&SPECIAL)
 {
  if(base==8)
   *str++='0';
  else if(base==16)
  {
   *str++='0';
   *str++=ucase_dig?'X':'x';
  }
 }
 if(!(type&LEFT))
 {
  while(size-->0)
   *str++=c;
 }
 while(i<precision--)
  *str++='0';
 while(i-->0)
  *str++=tmp[i];
 while(size-->0)
  *str++=' ';
 return(str);
}

/* vsprintf() implementation */

int vsprintf(char *buf, const char *fmt, va_list args)
{
 int len;
 unsigned long num;
 int i, base;
 char *str;
 char far *s;
 int flags;                             /* flags to number() */
 int field_width;                       /* width of output field */
 int precision;                         /* min. # of digits for integers; max
                                           number of chars for from string */
 int qualifier;                         /* 'h', 'l', or 'L' for integer
                                           fields */
 int far_str;                           /* Far string qualifier */

 for(str=buf; *fmt; ++fmt)
 {
  if(*fmt!='%')
  {
   if(*fmt=='\n')
    *str++='\r';
   *str++=*fmt;
   continue;
  }
  /* Process flags */
  flags=0;
  repeat:
  ++fmt;                                /* This also skips first '%' */
  switch(*fmt)
  {
   case '-': flags|=LEFT; goto repeat;
   case '+': flags|=PLUS; goto repeat;
   case ' ': flags|=SPACE; goto repeat;
   case '#': flags|=SPECIAL; goto repeat;
   case '0': flags|=ZEROPAD; goto repeat;
   case 'F': flags|=FAR_STR; goto repeat;
  }
  /* Get field width */
  field_width=-1;
  if(is_digit(*fmt))
   field_width=skip_atoi((char **)&fmt);
  else if(*fmt=='*')
  {
   ++fmt;
   /* It's the next argument */
   field_width=va_arg(args, int);
   if(field_width<0)
   {
    field_width=-field_width;
    flags|=LEFT;
   }
  }
  /* Get the precision */
  precision=-1;
  if(*fmt=='.')
  {
   ++fmt; 
   if(is_digit(*fmt))
    precision=skip_atoi((char **)&fmt);
   else if(*fmt=='*')
   {
    ++fmt;
    /* It's the next argument */
    precision=va_arg(args, int);
   }
   if(precision<0)
    precision=0;
  }
  /* Get the conversion qualifier */
  qualifier=-1;
  if(*fmt=='h'||*fmt=='l'||*fmt=='L')
  {
   qualifier=*fmt;
   ++fmt;
  }
  /* Default base */
  base=10;
  switch(*fmt)
  {
   case 'c':
    if(!(flags&LEFT))
     while(--field_width>0)
      *str++=' ';
    *str++=(unsigned char)va_arg(args, int);
    while(--field_width>0)
     *str++=' ';
    continue;
   case 's':
    s=(flags&FAR_STR)?va_arg(args, char *):va_arg(args, char far *);
    if(!s)
    {
     _asm{
      int 3                             /* Not expected */
     }
    }
    len=strnlen(s, precision);
    if(!(flags&LEFT))
     while(len<field_width--)
      *str++=' ';
    for(i=0; i<len; ++i)
    {
     if(*s=='\n')
      *str++='\r';
     *str++=*s++;
    }
    while(len<field_width--)
     *str++=' ';
    continue;
  case 'p':
   if(field_width==-1)
   {
    field_width=2*sizeof(void *);
    flags|=ZEROPAD;
   }
   str=number(str, (unsigned long)va_arg(args, void *), 16,
              field_width, precision, flags);
   continue;
  case 'n':
   if(qualifier=='l')
   {
    long *ip=va_arg(args, long *);
    *ip=(str-buf);
   }
   else
   {
    int *ip=va_arg(args, int *);
    *ip=(str-buf);
   }
   continue;
  /* Integer number formats - set up the flags and "break" */
  case 'o':
   base=8;
   break;
  case 'X':
   flags|=LARGE;
  case 'x':
   base=16;
   break;
  case 'd':
  case 'i':
   flags|=SIGN;
  case 'u':
   break;
  default:
   if(*fmt!='%')
    *str++='%';
   if(*fmt)
    *str++=*fmt;
   else
    --fmt;
   continue;
  }
  if(qualifier=='l')
   num=va_arg(args, unsigned long);
  else if(qualifier=='h')
  {
   if(flags&SIGN)
    num=va_arg(args, short);
   else
    num=va_arg(args, unsigned short);
  }
  else if(flags&SIGN)
   num=va_arg(args, int);
  else
   num=va_arg(args, unsigned int);
  str=number(str, num, base, field_width, precision, flags);
 }
 *str='\0';
 return(str-buf);
}

/* sprintf() for outside use */

int sprintf(char *buf, const char *fmt, ...)
{
 va_list args;
 int i;

 va_start(args, (char *)fmt);
 i=vsprintf(buf,fmt,args);
 va_end(args);
 return i;
}
