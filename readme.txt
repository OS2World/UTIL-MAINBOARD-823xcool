Power management tools for VIA KT266 and compatibles
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

1. Legal notice

   No warranties of any kind are made. In no event shall the author be liable
   for any damage resulting from use or misuse of this product, either direct
   or consequential. 

   The  trademarks   mentioned in this document   belong  to their respective
   owners.


2. Introduction 

   Chipsets  manufactured   by VIA Technologies,   Inc.    provide a  set  of
   facilities to prevent CPU  overheating and decrease the power consumption.
   In   systems like Windows  and Linux,  these facilities  are  managed by a
   system layer known as  ACPI (Advanced Configuration and  Power Interface).
   In OS/2, there  is no ACPI subsystem, meaning   that the user  will either
   have  to involve  APM   BIOS  facilities  or   install  additional  system
   monitoring and power management software oneself. 

   This package   provides a set  of  tools  to operate the   chipset's power
   management features under OS/2. Only the first component is mandatory: 

   - VIAAPM.SYS:  cools down the CPU and carries out power management tasks
   - APM_NH.SYS:  a version of APM.SYS compatible with VIAAPM.SYS
   - APMCTRL.EXE: an optional command-line control program for VIAAPM.SYS.

   OS/2 v 4.x or OS/2 v 3.0 with XR_W032 is required. VIAAPM.SYS will  run in
   all pre-XR_W032 versions of Warp 3 but then it could not be  controlled by
   means of APMCTRL.EXE.

   Scope of supported hardware:

   - VIA KT133 (ex-KZ133), KT133A, KX133, KM133, Apollo Pro 133. Support  for
     this  range  of chipsets  is experimental  and  is sometimes  bounded by
     hardware restrictions.

   - VIA KT266 and KT266A (8366[A] + 8233[A]). These were the original target
     platforms.

   - VIA KN266 (8372 + 8233A/8235).

   - VIA KT333 (8367 + 8233A/8235). Advanced  throttling  setup (/THROT:)  is
     not available for the VT8235 south bridge due to lack of documentation.

   ! VIA KT400 and VIA KT600 are NOT  supported. Certain models  may work due
     to similarity with supported chipsets  but no additional development  is
     planned in this area unless the datasheets are disclosed.

   - VIA KT880 (+ 8237). Advanced throttling setup (/THROT:) is not available
     for this chipset due to lack of documentation.

   For  a list  of  changes in  this  version, see the ChangeLog  file in the
   enclosed source code package.

   It is recommended that  you obtain StHwMon v  0.18  or higher,  a freeware
   utility by Stefan Milcke, in addition to  this toolkit, for monitoring the
   temperatures,  fan  speeds and  other   motherboard parameters.   Besides,
   StHwMon may be useful if you wish to keep  the CPU from reaching a certain
   temperature - there is a "smart" throttling algorithm for that.


3. Quick start

   There is no installation program. To use the CPU  cooling features, follow
   these instructions:

   1. Install VIAAPM.SYS in CONFIG.SYS:

      DEVICE=x:\OS2\APMTOOLS\VIAAPM.SYS

   2. If you were using  APM.SYS (e.g. for  the purpose of powering  down the
      computer), replace  its CONFIG.SYS entry  with APM_NH.SYS. For example,
      change:

      DEVICE=x:\OS2\BOOT\APM.SYS
      to
      DEVICE=x:\OS2\APMTOOLS\APM_NH.SYS

   3. Reboot.


4. VIA power management driver (VIAAPM.SYS)

   VIAAPM.SYS provides the following services:

   - CPU idle hook for APM KPI (APM_IdleHookRtn).
   - CPU throttling API.
   - Vendor-specific monitoring and tuning functions.

   The APM idle hook routine replaces the OS/2's standard idle operation (HLT
   x86 instruction). The reason for  it is that HLT  corresponds to ACPI "C1"
   level of  CPU operation (CPU still connected  to bus and consumes its full
   rated power).  The "C2" level, provided  by the idle hook, disconnects the
   CPU from its bus,  dropping the power  consumption by  about 10 times  and
   putting the CPU into so-called "Stop  Grant state".  CPU returns from this
   state in the same ways as from  "C1", therefore the performance losses are
   insignificant.

   If  it  happens that the system  comes  to  a  trap screen with REIPL=OFF,
   VIAAPM.SYS will intercept  the  trap handler  and  force ACPI C2  state to
   prevent the CPU from  overheating  (at the trap  screen  the CPU  runs  an
   eternal loop).  Similarly,  when a SYS1503  (swap file full) error occurs,
   the CPU is forced into  ACPI C3 state (stop CPU  clock) until RESET button
   on the system case is pressed. As OS/2 is not an ACPI-compliant system, it
   gives an advantage: there  are only hardware  ways left to exit  from ACPI
   C3.  An alternate function  of this driver is  to reboot on SYS1503, since
   OS/2 normally does  not reboot  regardless of REIPL  mode.   See below for
   /SYS1503.

   The "throttling API" enables the user to slow down  the CPU by selectively
   filtering out  CPU clock cycles.  In  modern VIA chipsets,  this method is
   particularly useful  for   cooling  down the  memory    controller ("north
   bridge") which imposes a noticeable source of heat besides the CPU. 

   Options:

       /VERBOSE = Verbose  initialization. Tells to display the full progress
                  of startup.

       /SYS1503 = Reboot the system when SYS1503 comes up. This error message
                  is normally a "dead-end" regardless of whether REIPL=ON has
                  been specified in CONFIG.SYS. However, it is sometimes more
                  practical to  allow  the system restart  when the swap file
                  is full.

 /THROT:{F|M|S} = Fine-tune  throttling   timings  (where   available).   The
                  throttling timer by  default runs in the "medium" (M) mode.
                  A  faster  mode (F) is available  which  modulates  the CPU
                  clock at  a  higher  rate for a smoother frequency control.
                  Conversely, the slow mode (S) will reduce this cycling time
                  proportionally.  /THROT:F  may   be  used  for  performance
                  tuning.

          /!HLT = Disable  automatic  HLT detection. This  would  revert  the
                  behavior  of VIAAPM.SYS  to  pre-2.00 level. On  some "133"
                  series chipsets, as well as on KT880, the HLT detection may
                  not  be  supported at  all, in  this case  the /!HLT switch
                  won't have any effect.

   Notes:

   1. Certain restrictions may apply to older components:

        * Intermittent system  hangs when  a poor power supply unit  is used.
          Athlon XP sets 300W as the minimum for high-performance PCs.
        * AMD  Athlon  Model 4  processors  with  half-frequency  multipliers
          (7.5, 8.5 =  750/850 MHz) have their own problems entering the "C2"
          state.  Please  refer  to  the  technical  documentation  available
          from  your  CPU   and  motherboard   manufacturer if you experience
          stability problems.
        * Some  KT133A boards  are "optimized for  PCI bus performance" - see
          the corresponding topics in troubleshooting section (IDE problems).

   2. In response to  suggestions from AMD, several chipsets and motherboards
      implement a BIOS  setting that  reads "HLT enters Stop Grant state". In
      this case, VIAAPM.SYS is not required unless you want to take advantage
      of throttling.

   3. The driver is hibernation-aware  and will restore  the power management
      configuration  when  exiting   from hibernation  or  trapdoor (with the
      exception that throttling  mode active  during shutdown is not restored
      and the CPU runs at  full speed). You may  also request resets manually 
      (see below for APMCTRL.EXE /RESET switch).


5. APM control program (APMCTRL.EXE)

   APMCTRL.EXE is a front-end  for VIAAPM.SYS. It can be  started with one of
   the following switches: 

   /RESET = reinitialize the chipset for ACPI  "C2" support and reinstall the
   kernel APM hook. Reports error if any problems were found. 

   /T = examine or start/stop CPU throttling.  There are 15 throttling levels
   (1 is the lowest, devoting only 0...6.25% of clock  ticks to CPU and 15 is
   the   highest,  meaning  93.75...100%).  Level  16  stands  for   "disable
   throttling and run CPU at 100% of its clock speed". Examples: 

	/T	Show if throttling is active
	/T:7	Set level 7 (about 43% duty cycle)
        /T:50%  Explicitly set the duty cycle
        /T:16   Disable throttling
        /T:100% Same as above

   /Q = query the chipset type.

   Tip: Both VIAAPM.SYS and APMCTRL.EXE provide some undocumented options for
   other  aspects of  fine-tuning.  See  the source code   for details. These
   options are intended for experienced users only.


6. Replacement APM driver (APM_NH.SYS)

   APM_NH.SYS is based on a pre-XR_D004 level of IBM's APM.SYS.  The specific
   job of APM_NH.SYS is to honor the APM hooks  installed by VIAAPM.SYS while
   allowing  usual APM   tasks such  as   power-down or suspend  mode.  It is
   therefore essential that  one installs APM_NH.SYS  rather than APM.SYS, so
   the KT266A idle hook would not be taken out by IBM's driver.


7. Troubleshooting suggestions

   Problem:   the driver traps  or   complains about  unknown  chipset during
   startup.

   Solution:

        1. Ensure that you  have the appropriate chipset. Contact the  author
           if  your chipset is   claimed  to be  supported by  this  program.
           Please  include a list of PCI device IDs in  your system. The tool
           to obtain this list is available at:
           http://hobbes.nmsu.edu/pub/os2/util/misc/pci047vk.zip
        2. Some systems, while generally compatible, have permanent  hardware
           limitations, which result in a reboot shortly after the ACPI C2 is
           enabled. Consult your vendor to ensure your system meets the power
           and configuration guidelines for ACPI compliance.


   Problem: the device driver does not cool down the CPU.

   Solution:

        1. Make sure you don't have IBM's APM.SYS  installed - it provides an
           an idle  hook of its own, conflicting  with VIAAPM.SYS. See  above
           for instructions on replacing APM.SYS.
        2. Check if the CPU is  really  idle for appropriate time - use Pulse
           or WarpCenter to determine the CPU load.
        3. Try APM /T:1 to see if  throttling works (if  it works, the system
           will be considerably slowed down). If the throttling does not work
           too, you may be using an incompatible revision of chipset.


   Problem: lockups on KT133 when VIAAPM.SYS is loaded

   Solution:

        1. VIAAPM.SYS  might  not  reliably  operate  on AMD  Athlon  CPUs at
           550/650/750/850/950 MHz.
        2. Ensure that your  power  supply unit  is capable  of dealing  with
           flux of CPU power consumption (9 to 70W).
        3. AMD  Athlon  "Model 4" (650...1000  MHz) has  a  reconnect  timing
           problem. Refer to AMD technical note #24478.
        4. KT133A only: if the IDE  subsystem is failing after  the driver is
           installed (e.g. failure to load a particular driver), follow these
           guidelines:
              1. Run BIOS setup.
              2. Disable "PCI Delay Transaction".
              3. Disable "PCI master READ caching".
              4. Set "PCI Latency" to 0.


   Problem: poor performance - /T:10% seems to be  much less than  actual 10%
   of the full performance!

   Solution: This is  explained by the fact how  throttling works. It is  not
   like running at  the exactly reduced frequency -  instead it runs your CPU
   at full frequency  for several thousand cycles,  then stops its  clock for
   another several thousand  cycles. This  prevents  the CPU  from accurately
   handling microsecond delays associated  with I/O, which in turn  decreases
   the overall responsiveness of the system.

   The /THROT  command-line   parameter  may alleviate  these   drawbacks  of
   throttling if set for a  "fast" mode, but  there  is no general method  of
   selecting the actual throttling rate to meet the needs of a specific task.
   For example, programmable I/O operations,  such as parallel port transfer,
   demand full  CPU power regardless of CPU   frequency.  Therefore, the only
   appropriate action in   this case is  to  decrease the  CPU frequency  via
   front-side  bus, which   is an  undocumented vendor-specific feature   not
   discussed in the present document.


8. Contacting the author 

   The author can  be contacted at <andrew_belov@newmail.ru>. Improvements to
   this software package are welcome; if you add support for a specific chip,
   please send in the corresponding source code patch.

   $Id: readme.txt,v 1.18 2004/12/02 04:25:37  Exp $ 
