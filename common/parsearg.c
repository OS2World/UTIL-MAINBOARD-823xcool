/* $Id: parsearg.c,v 1.3 2003/11/19 21:13:24 root Exp $ */

/* Command-line argument parser */

#include <stddef.h>

#include "vsprintf.h"
#include "kprintf.h"
#include "parsearg.h"

/*
 * Parameter handling routines
 */

/* Yet another toupper() hack */

static char ltoupper(char c)
{
 return((c>='a'&&c<='z')?(c-32):c);
}

/* strlen() for far strings */

static unsigned int lstrlen(char far *s)
{
 unsigned int rc;

 for(rc=0; s[rc]!='\0'; rc++)
  ;
 return(rc);
}

/* A strcmp() hack for processing the keywords */

static int parmcmp(char far *s1, char far *s2)
{
 int i;

 for(i=0; s1[i]!='\0'; i++)
 {
  if(ltoupper(s1[i])!=ltoupper(s2[i]))
   return(1);                           /* Hack */
 }
 return(0);
}

/* Counts the parameter length. The delimiters are ':', '/', and 0x00..0x20. */

int ctparms(char far *s)
{
 int i;

 for(i=0; s[i]>' '&s[i]!=':'&&s[i]!='/'; i++)
  ;
 return(i);
}

/* Skips to what seems to be a parameter. */

static char far *skip_to_parm(char far *s)
{
 while(*s!='/')
 {
  if(*s=='\0')
   return(NULL);
  s++;
 }
 while(*++s=='/')
  ;
 if(*s=='\0')
  return(NULL);
 return(s);
}

/* Performs parameter processing */

static void proc_parm(char far *s, struct paramset *pars)
{
 char far *data_ptr;                    /* Pointer to data (if any) */
 char far *p_s;
 int i;

 for(i=0; pars[i].t!=NULL; i++)
 {
  if(!parmcmp(pars[i].t, s))
  {
   p_s=s-1;
   s+=lstrlen(pars[i].t);
   data_ptr=(ctparms(s)==0)?NULL:s;
   if(pars[i].process!=NULL)
   {
    if(pars[i].process(data_ptr))
     kprintf(ANSI15 "Command-line error near `" ANSI14 "%Fs" ANSI15 "`" ANSI7 "\n", p_s);
   }
   if(pars[i].trigger)
    *(pars[i].trigger)=1;
  }
 }
}

/* Command-line parser */

int parse_args(char far *p, struct paramset *pars)
{
 /* Skip over the first argument which tends to be argv[0] */
 while(*p!=' '&&*p!='\0')
  p++;
 if(*p<' ')
  return(0);
 while((p=skip_to_parm(p+1))!=NULL)
  proc_parm(p, pars);
 return(0);
}
