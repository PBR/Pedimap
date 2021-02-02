//PCH_H: Precompiled Common Header file
#ifndef PCH_H
#define PCH_H

//include every VCL header (included Vcl.h instead)
#include <Vcl.h>
#include <ComCtrls.hpp>  //TTreeNode; not included in vcl.h ??
#include <Printers.hpp>
#include <StrUtils.hpp>
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>

//include C/C++ RTL headers used
#include <math.h>
#include <values.h>
#include <stdlib.h>

//include headers for 3rd party controls (none so far)


//our custom controls (none so far)


//Object repository header files (none so far)


//project include files
//pre-compile only if PRECOMPILE_ALL is defined
#ifdef PRECOMPILE_ALL

#include "DataUnit.h"
#include "ParseUnit.h"
#include "FontUnit.h"
#include "OptionsUnit.h"
#include "Main.h"

#endif

#endif
