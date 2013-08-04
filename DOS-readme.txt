DOS-based control utility for VIA chipset
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

   This    program  offers  a    limited   subset of   VIAAPM.SYS/APMCTRL.EXE
   functionality for  MS-DOS/PC-DOS  operating    systems  (version   2.0  or
   higher). Its purpose is to keep the system from  overheating in cases when
   DOS has to be  used to restore after a  major malfunction of the operating
   system. It is not normally required under OS/2 MDOS.

   VIAAPM.COM can be installed resident in RAM with /INSTALL option (there is
   no uninstall, nor a way to check if it has  already been installed).  This
   takes about 160   bytes    of RAM  plus    about 400  bytes  due   to  MCB
   fragmentation.  In the resident mode, VIAAPM  intercepts the DOS idle hook
   (which is  not as smart as  OS/2 idle hook) and  issues ACPI C2 state each
   time when DOS is waiting for standard input (keyboard).   The idle hook is
   executed only when the standard DOS programming interface  is used to poll
   keyboard, e.g.  COMMAND.COM will call it  but other shells wouldn't.  Some
   shell  programs,  such as   DOS  Navigator II  (http://www.dnosp.ru),  are
   capable of calling INT 28h during user  inactivity within the shell - this
   way it works with VIAAPM.

   The "/!HLT" modifier for "/INSTALL" will ignore setting  the automatic HLT
   detection and acts in the same way as under OS/2.

   Throttling is  recommended as a  more efficient  way of  power  management
   under DOS.  To   use  throttling, there is  no   need to make   VIAAPM.COM
   resident, just run it with the "/T" option as with APMCTRL.EXE under OS/2.

   As a side-note, the PC2001  specification that most KT266A-based solutions
   are aimed to comply with, makes no allowances for running real-mode DOS on
   the compliant  hardware. That is, the  hardware is not  designed for using
   DOS for purposes other than Windows bootstrap.

   $Id: readme.txt,v 1.4 2004/12/02 04:25:37  Exp $
