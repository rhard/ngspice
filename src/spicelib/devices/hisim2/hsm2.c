/***********************************************************************

 HiSIM (Hiroshima University STARC IGFET Model)
 Copyright (C) 2014 Hiroshima University & STARC

 MODEL NAME : HiSIM
 ( VERSION : 2  SUBVERSION : 8  REVISION : 0 )
 
 FILE : hsm2.c

 Date : 2014.6.5

 released by 
                Hiroshima University &
                Semiconductor Technology Academic Research Center (STARC)
***********************************************************************/

/**********************************************************************

The following source code, and all copyrights, trade secrets or other
intellectual property rights in and to the source code in its entirety,
is owned by the Hiroshima University and the STARC organization.

All users need to follow the "HiSIM2 Distribution Statement and
Copyright Notice" attached to HiSIM2 model.

-----HiSIM2 Distribution Statement and Copyright Notice--------------

Software is distributed as is, completely without warranty or service
support. Hiroshima University or STARC and its employees are not liable
for the condition or performance of the software.

Hiroshima University and STARC own the copyright and grant users a perpetual,
irrevocable, worldwide, non-exclusive, royalty-free license with respect 
to the software as set forth below.   

Hiroshima University and STARC hereby disclaim all implied warranties.

Hiroshima University and STARC grant the users the right to modify, copy,
and redistribute the software and documentation, both within the user's
organization and externally, subject to the following restrictions

1. The users agree not to charge for Hiroshima University and STARC code
itself but may charge for additions, extensions, or support.

2. In any product based on the software, the users agree to acknowledge
Hiroshima University and STARC that developed the software. This
acknowledgment shall appear in the product documentation.

3. The users agree to reproduce any copyright notice which appears on
the software on any copy or modification of such made available
to others."


*************************************************************************/

#include "ngspice/ngspice.h"
#include "ngspice/devdefs.h"
#include "hsm2def.h"
#include "ngspice/suffix.h"

IFparm HSM2pTable[] = { /* parameters */
 IOP( "l",   HSM2_L,      IF_REAL   , "Length"),
 IOP( "w",   HSM2_W,      IF_REAL   , "Width"),
 IOP( "ad",  HSM2_AD,     IF_REAL   , "Drain area"),
 IOP( "as",  HSM2_AS,     IF_REAL   , "Source area"),
 IOP( "pd",  HSM2_PD,     IF_REAL   , "Drain perimeter"),
 IOP( "ps",  HSM2_PS,     IF_REAL   , "Source perimeter"),
 IOP( "nrd", HSM2_NRD,    IF_REAL   , "Number of squares in drain"),
 IOP( "nrs", HSM2_NRS,    IF_REAL   , "Number of squares in source"),
 IOP( "temp", HSM2_TEMP,  IF_REAL   , "Lattice temperature [K]"),
 IOP( "dtemp", HSM2_DTEMP,IF_REAL   , ""),
 IOP( "off", HSM2_OFF,    IF_FLAG   , "Device is initially off"), 
 IP ( "ic",  HSM2_IC,     IF_REALVEC , "Vector of DS,GS,BS initial voltages"),
 IOP("corbnet", HSM2_CORBNET, IF_INTEGER, "activate body resistance (1) or not (0)"),
 IOP("rbpb", HSM2_RBPB, IF_REAL, ""),
 IOP("rbpd", HSM2_RBPD, IF_REAL, ""),
 IOP("rbps", HSM2_RBPS, IF_REAL, ""),
 IOP("rbdb", HSM2_RBDB, IF_REAL, ""),
 IOP("rbsb", HSM2_RBSB, IF_REAL, ""),
 IOP("corg", HSM2_CORG, IF_INTEGER, "activate gate resistance (1) or not (0)"),
/*  IOP("rshg", HSM2_RSHG, IF_REAL, "gate-elecrode sheet resistance"), */
 IOP("ngcon", HSM2_NGCON, IF_REAL, "number of gate contacts"),
 IOP("xgw", HSM2_XGW, IF_REAL, "distance from gate contact to channel edge"),
 IOP("xgl", HSM2_XGL, IF_REAL, "offset of gate length due to variation in patterning"),
 IOP("nf", HSM2_NF, IF_REAL, "number of fingers"),
 IOP("sa", HSM2_SA, IF_REAL, "distance from STI edge to Gate edge [m]"),
 IOP("sb", HSM2_SB, IF_REAL, "distance from STI edge to Gate edge [m]"),
 IOP("sd", HSM2_SD, IF_REAL, "distance from Gate edge to Gate edge [m]"),
 IOP("nsubcdfm", HSM2_NSUBCDFM, IF_REAL, "constant part of Nsub for DFM [1/cm^3]"),
 IOP("mphdfm", HSM2_MPHDFM, IF_REAL, "NSUBCDFM dependence of phonon scattering for DFM"),
 IOP("m", HSM2_M, IF_REAL, "Multiplication factor [-]"),
/* WPE */
 IOP("sca", HSM2_SCA, IF_REAL, "WPE sca"),
 IOP("scb", HSM2_SCB, IF_REAL, "WPE scb"),
 IOP("scc", HSM2_SCC, IF_REAL, "WPE scc"),

	/* Output Physical Values: */
 OP ( "ids",   HSM2_CD,    IF_REAL   , "Ids"),  /* Drain-Source current */
 OP ( "isub",  HSM2_ISUB,  IF_REAL   , "Isub"),  /* Substrate current */
 OP ( "igidl", HSM2_IGIDL, IF_REAL   , "Igidl"), /* Gate-Induced Drain Leakage current */
 OP ( "igisl", HSM2_IGISL, IF_REAL   , "Igisl"), /* Gate-Induced Source Leakage current */
 OP ( "igd",   HSM2_IGD,   IF_REAL   , "Igd"),  /* Gate-Drain current */
 OP ( "igs",   HSM2_IGS,   IF_REAL   , "Igs"),  /* Gate-Source current */
 OP ( "igb",   HSM2_IGB,   IF_REAL   , "Igb"),  /* Gate-Substrate current */
 OP ( "gm",    HSM2_GM,    IF_REAL   , "Gm"),   /* Transconductance */
 OP ( "gds",   HSM2_GDS,   IF_REAL   , "Gds"),  /* Channel conductance */
 OP ( "gmbs",  HSM2_GMBS,  IF_REAL   , "Gmbs"), /* Body effect (Back gate) transconductance */
 OP ( "von",   HSM2_VON,   IF_REAL   , "Von"),  /* Threshold voltage */
 OP ( "vdsat", HSM2_VDSAT, IF_REAL   , "Vdsat"), /* Saturation voltage */
 OP ( "qb",    HSM2_QB,    IF_REAL   , "Qb"),   /* Bulk charge */
 OP ( "qg",    HSM2_QG,    IF_REAL   , "Qg"),   /* Gate charge */
 OP ( "qd",    HSM2_QD,    IF_REAL   , "Qd"),   /* Drain charge */
 OP ( "cgg",   HSM2_CGG,   IF_REAL   , "Cgg"),  /* MOSFET capacitance */
 OP ( "cgd",   HSM2_CGD,   IF_REAL   , "Cgd"),  /* MOSFET capacitance */
 OP ( "cgs",   HSM2_CGS,   IF_REAL   , "Cgs"),  /* MOSFET capacitance */
 OP ( "cbg",   HSM2_CBG,   IF_REAL   , "Cbg"),  /* MOSFET capacitance */
 OP ( "cbs",   HSM2_CBSB,  IF_REAL   , "Cbs"),  /* MOSFET capacitance */
 OP ( "cbd",   HSM2_CBDB,  IF_REAL   , "Cbd"),  /* MOSFET capacitance */
 OP ( "cdg",   HSM2_CDG,   IF_REAL   , "Cdg"),  /* MOSFET capacitance */
 OP ( "cdd",   HSM2_CDD,   IF_REAL   , "Cdd"),  /* MOSFET capacitance */
 OP ( "cds",   HSM2_CDS,   IF_REAL   , "Cds"),  /* MOSFET capacitance */
 OP ( "cgdo",  HSM2_CGDO,  IF_REAL   , "Cgdo"), /* MOSFET overlap capacitance */
 OP ( "cgso",  HSM2_CGSO,  IF_REAL   , "Cgso"), /* MOSFET overlap capacitance */
 OP ( "cgbo",  HSM2_CGBO,  IF_REAL   , "Cgbo"), /* MOSFET overlap capacitance */
 OP ( "ibd",   HSM2_CBD,   IF_REAL   , "Ibd"),  /* Diode current */
 OP ( "ibs",   HSM2_CBS,   IF_REAL   , "Ibs"),  /* Diode current */
 OP ( "gbd",   HSM2_GBD,   IF_REAL   , "Gbd"),  /* Diode conductance */
 OP ( "gbs",   HSM2_GBS,   IF_REAL   , "Gbs"),  /* Diode conductance */
 OP ( "capbd", HSM2_CAPBD, IF_REAL   , "Capbd"), /* Diode capacitance */
 OP ( "capbs", HSM2_CAPBS, IF_REAL   , "Capbs") /* Diode capacitance */
};

IFparm HSM2mPTable[] = { /* model parameters */
  IP("nmos", HSM2_MOD_NMOS, IF_FLAG, ""),
  IP("pmos", HSM2_MOD_PMOS, IF_FLAG, ""),
  IOP("level", HSM2_MOD_LEVEL, IF_INTEGER, ""),
  IOP("info", HSM2_MOD_INFO, IF_INTEGER, "information level (for debug, etc.)"),
  IOP("noise", HSM2_MOD_NOISE, IF_INTEGER, "noise model selector"),
  IOP("version", HSM2_MOD_VERSION, IF_INTEGER, "model version 280"),
  IOP("show", HSM2_MOD_SHOW, IF_INTEGER, "show physical value"),
  IOP("corsrd", HSM2_MOD_CORSRD, IF_INTEGER, "solve equations accounting Rs and Rd."),
  IOP("corg", HSM2_MOD_CORG, IF_INTEGER, "solve equations accounting Rg."),
  IOP("coiprv", HSM2_MOD_COIPRV, IF_INTEGER, "use ids_prv as initial guess of Ids (internal flag)"),
  IOP("copprv", HSM2_MOD_COPPRV, IF_INTEGER, "use ps{0/l}_prv as initial guess of Ps{0/l} (internal flag)"),
  IOP("coadov", HSM2_MOD_COADOV, IF_INTEGER, "add overlap to intrisic"),
  IOP("coisub", HSM2_MOD_COISUB, IF_INTEGER, "calculate isub"),
  IOP("coiigs", HSM2_MOD_COIIGS, IF_INTEGER, "calculate igate"),
  IOP("cogidl", HSM2_MOD_COGIDL, IF_INTEGER, "calculate igidl"),
  IOP("coovlp", HSM2_MOD_COOVLP, IF_INTEGER, "calculate overlap charge"),
  IOP("coflick", HSM2_MOD_COFLICK, IF_INTEGER, "calculate 1/f noise"),
  IOP("coisti", HSM2_MOD_COISTI, IF_INTEGER, "calculate STI"),
  IOP("conqs", HSM2_MOD_CONQS, IF_INTEGER, "calculate in nqs mode or qs mode"),
  IOP("corbnet", HSM2_MOD_CORBNET, IF_INTEGER, ""),
  IOP("cothrml", HSM2_MOD_COTHRML, IF_INTEGER, "calculate thermal noise"),
  IOP("coign", HSM2_MOD_COIGN, IF_INTEGER, "calculate induced gate noise"),
  IOP("codfm", HSM2_MOD_CODFM, IF_INTEGER, "calculation of model for DFM"),
  IOP("corecip", HSM2_MOD_CORECIP, IF_INTEGER, "capacitance reciprocity takes first priority"),
  IOP("coqy", HSM2_MOD_COQY, IF_INTEGER, "calculate lateral-field-induced charge/capacitance"),
  IOP("coqovsm", HSM2_MOD_COQOVSM, IF_INTEGER, "select smoothing method of Qover"),
  IOP("coerrrep", HSM2_MOD_COERRREP, IF_INTEGER, "selector for error report"),
  IOP("codep", HSM2_MOD_CODEP, IF_INTEGER, "selector for depletion device"),
  IOP("coddlt", HSM2_MOD_CODDLT, IF_INTEGER, "selector for ddlt model "),
  IOP("vmax", HSM2_MOD_VMAX, IF_REAL, "saturation velocity [cm/s]"),
  IOP("bgtmp1", HSM2_MOD_BGTMP1, IF_REAL, "first order temp. coeff. for band gap [V/K]"),
  IOP("bgtmp2", HSM2_MOD_BGTMP2, IF_REAL, "second order temp. coeff. for band gap [V/K^2]"),
  IOP("eg0", HSM2_MOD_EG0, IF_REAL, ""),
  IOP("tox", HSM2_MOD_TOX, IF_REAL, "oxide thickness [m]"),
  IOP("xld", HSM2_MOD_XLD, IF_REAL, "lateral diffusion of S/D under the gate [m]"),
  IOP("lover", HSM2_MOD_LOVER, IF_REAL, "overlap length"),
  IOP("ddltmax", HSM2_MOD_DDLTMAX, IF_REAL, ""), /* Vdseff */
  IOP("ddltslp", HSM2_MOD_DDLTSLP, IF_REAL, ""), /* Vdseff */
  IOP("ddltict", HSM2_MOD_DDLTICT, IF_REAL, ""), /* Vdseff */
  IOP("vfbover", HSM2_MOD_VFBOVER, IF_REAL, ""),
  IOP("nover", HSM2_MOD_NOVER, IF_REAL, ""),
  IOP("xwd", HSM2_MOD_XWD, IF_REAL, "lateral diffusion along the width dir. [m]"),
  IOP("xl", HSM2_MOD_XL, IF_REAL, "gate length offset due to mask/etch effect [m]"),
  IOP("xw", HSM2_MOD_XW, IF_REAL, "gate width offset due to mask/etch effect [m]"),
  IOP("saref", HSM2_MOD_SAREF, IF_REAL, "reference distance from STI edge to Gate edge [m]"),
  IOP("sbref", HSM2_MOD_SBREF, IF_REAL, "reference distance from STI edge to Gate edge [m]"),
  IOP("ll", HSM2_MOD_LL, IF_REAL, "gate length parameter"),
  IOP("lld", HSM2_MOD_LLD, IF_REAL, "gate length parameter"),
  IOP("lln", HSM2_MOD_LLN, IF_REAL, "gate length parameter"),
  IOP("wl", HSM2_MOD_WL, IF_REAL, "gate width parameter"),
  IOP("wl1", HSM2_MOD_WL1, IF_REAL, "gate width parameter"),
  IOP("wl1p", HSM2_MOD_WL1P, IF_REAL, "gate width parameter"),
  IOP("wl2", HSM2_MOD_WL2, IF_REAL, "gate width parameter"),
  IOP("wl2p", HSM2_MOD_WL2P, IF_REAL, "gate width parameter"),
  IOP("wld", HSM2_MOD_WLD, IF_REAL, "gate width parameter"),
  IOP("wln", HSM2_MOD_WLN, IF_REAL, "gate width parameter"),
  IOP("xqy", HSM2_MOD_XQY, IF_REAL, "[m]"),
  IOP("xqy1", HSM2_MOD_XQY1, IF_REAL, "[F m^{XQY2}]"),
  IOP("xqy2", HSM2_MOD_XQY2, IF_REAL, "[-]"),
  IOP("qyrat", HSM2_MOD_QYRAT, IF_REAL, ""),
  IOP("rs", HSM2_MOD_RS, IF_REAL, "source contact resistance [ohm m]"),
  IOP("rd", HSM2_MOD_RD, IF_REAL, "drain contact resistance  [ohm m]"),
  IOP("rsh", HSM2_MOD_RSH, IF_REAL, "source/drain diffusion sheet  resistance  [ohm]"),
  IOP("rshg", HSM2_MOD_RSHG, IF_REAL, "gate-elecrode sheet resistance"),
/*   IOP("ngcon", HSM2_MOD_NGCON, IF_REAL, "number of gate contacts"), */
/*   IOP("xgw", HSM2_MOD_XGW, IF_REAL, "distance from gate contact to channel edge"), */
/*   IOP("xgl", HSM2_MOD_XGL, IF_REAL, "offset of gate length due to variation in patterning"), */
/*   IOP("nf", HSM2_MOD_NF, IF_REAL, "number of fingers"), */
  IOP("vfbc", HSM2_MOD_VFBC, IF_REAL, "constant part of Vfb [V]"),
  IOP("vbi", HSM2_MOD_VBI, IF_REAL, "built-in potential [V]"),
  IOP("nsubc", HSM2_MOD_NSUBC, IF_REAL, "constant part of Nsub [1/cm^3]"),
  IOP("vfbcl",  HSM2_MOD_VFBCL, IF_REAL,  "gate-length dependence of VFBC [um]"),
  IOP("vfbclp", HSM2_MOD_VFBCLP, IF_REAL, "gate-length dependence of VFBC [-]"),
  IOP("parl2", HSM2_MOD_PARL2, IF_REAL, "under diffusion [m]"),
  IOP("lp", HSM2_MOD_LP, IF_REAL, "length of pocket potential [m]"),
  IOP("nsubp", HSM2_MOD_NSUBP, IF_REAL, "[1/cm^3]"),
  IOP("nsubpl", HSM2_MOD_NSUBPL, IF_REAL, "gate-length dependence of NSUBP"),
  IOP("nsubpfac", HSM2_MOD_NSUBPFAC, IF_REAL, "gate-length dependence of NSUBP"),
  IOP("nsubpdlt", HSM2_MOD_NSUBPDLT, IF_REAL, "Delta for nsubp smoothing [-]"),
  IOP("nsubpw", HSM2_MOD_NSUBPW, IF_REAL, "pocket implant parameter"),
  IOP("nsubpwp", HSM2_MOD_NSUBPWP, IF_REAL, "pocket implant parameter"),
  IOP("scp1", HSM2_MOD_SCP1, IF_REAL, "parameter for pocket [-]"),
  IOP("scp2", HSM2_MOD_SCP2, IF_REAL, "parameter for pocket [1/V]"),
  IOP("scp3", HSM2_MOD_SCP3, IF_REAL, "parameter for pocket [m/V]"),
  IOP("sc1", HSM2_MOD_SC1, IF_REAL, "parameter for SCE [-]"),
  IOP("sc2", HSM2_MOD_SC2, IF_REAL, "parameter for SCE [1/V]"),
  IOP("sc3", HSM2_MOD_SC3, IF_REAL, "parameter for SCE [m/V]"),
  IOP("sc4", HSM2_MOD_SC4, IF_REAL, "parameter for SCE []"),
  IOP("pgd1", HSM2_MOD_PGD1, IF_REAL, "parameter for gate-poly depletion [V]"),
  IOP("pgd2", HSM2_MOD_PGD2, IF_REAL, "parameter for gate-poly depletion [V]"),
//IOP("pgd3", HSM2_MOD_PGD3, IF_REAL, "parameter for gate-poly depletion [-]"), 
  IOP("pgd4", HSM2_MOD_PGD4, IF_REAL, "parameter for gate-poly depletion [-]"),
  IOP("ndep", HSM2_MOD_NDEP, IF_REAL, "coeff. of Qbm for Eeff [-]"),
  IOP("ndepl", HSM2_MOD_NDEPL, IF_REAL, "coeff. of Qbm for Eeff [-]"),
  IOP("ndeplp", HSM2_MOD_NDEPLP, IF_REAL, "coeff. of Qbm for Eeff [-]"),
  IOP("ndepw", HSM2_MOD_NDEPW, IF_REAL, "coeff. of Qbm for Eeff [-]"),
  IOP("ndepwp", HSM2_MOD_NDEPWP, IF_REAL, "coeff. of Qbm for Eeff [-]"),
  IOP("ninv", HSM2_MOD_NINV, IF_REAL, "coeff. of Qnm for Eeff [-]"),
  IOP("ninvd", HSM2_MOD_NINVD, IF_REAL, "modification of Vdse dependence on Eeff [1/V]"),
  IOP("ninvdl", HSM2_MOD_NINVDL, IF_REAL, " LG dependence of NINVD "),
  IOP("ninvdlp", HSM2_MOD_NINVDLP, IF_REAL, " LG dependence of NINVD "),
  IOP("muecb0", HSM2_MOD_MUECB0, IF_REAL, "const. part of coulomb scattering [cm^2/Vs]"),
  IOP("muecb1", HSM2_MOD_MUECB1, IF_REAL, "coeff. for coulomb scattering [cm^2/Vs]"),
  IOP("mueph0", HSM2_MOD_MUEPH0, IF_REAL, "power of Eeff for phonon scattering [-]"),
  IOP("mueph1", HSM2_MOD_MUEPH1, IF_REAL, ""),
  IOP("muephw", HSM2_MOD_MUEPHW, IF_REAL, ""),
  IOP("muepwp", HSM2_MOD_MUEPWP, IF_REAL, "phonon scattering parameter"),
  IOP("muepwd", HSM2_MOD_MUEPWD, IF_REAL, "phonon scattering parameter"),
  IOP("muephl", HSM2_MOD_MUEPHL, IF_REAL, "phonon scattering parameter"),
  IOP("mueplp", HSM2_MOD_MUEPLP, IF_REAL, "phonon scattering parameter"),
  IOP("muepld", HSM2_MOD_MUEPLD, IF_REAL, "phonon scattering parameter"),
  IOP("muephs", HSM2_MOD_MUEPHS, IF_REAL, ""),
  IOP("muepsp", HSM2_MOD_MUEPSP, IF_REAL, ""),
  IOP("vtmp", HSM2_MOD_VTMP, IF_REAL, ""),
  IOP("wvth0", HSM2_MOD_WVTH0, IF_REAL, ""),
  IOP("muesr0", HSM2_MOD_MUESR0, IF_REAL, "power of Eeff for S.R. scattering [-]"),
  IOP("muesr1", HSM2_MOD_MUESR1, IF_REAL, "coeff. for S.R. scattering [-]"),
  IOP("muesrl", HSM2_MOD_MUESRL, IF_REAL, "surface roughness parameter"),
  IOP("muesrw", HSM2_MOD_MUESRW, IF_REAL, "change of surface roughness related mobility"),
  IOP("mueswp", HSM2_MOD_MUESWP, IF_REAL, "change of surface roughness related mobility"),
  IOP("mueslp", HSM2_MOD_MUESLP, IF_REAL, "surface roughness parameter"),
  IOP("muetmp", HSM2_MOD_MUETMP, IF_REAL, "parameter for mobility [-]"),
  IOP("bb", HSM2_MOD_BB, IF_REAL, "empirical mobility model coefficient [-]"),
  IOP("sub1", HSM2_MOD_SUB1, IF_REAL, "parameter for Isub [1/V]"),
  IOP("sub2", HSM2_MOD_SUB2, IF_REAL, "parameter for Isub [V]"),
  IOP("svgs", HSM2_MOD_SVGS, IF_REAL, "coefficient for Vg of Psislsat"),
  IOP("svbs", HSM2_MOD_SVBS, IF_REAL, "coefficient for Vbs of Psislsat"),
  IOP("svbsl", HSM2_MOD_SVBSL, IF_REAL, " "),
  IOP("svds", HSM2_MOD_SVDS, IF_REAL, " "),
  IOP("slg", HSM2_MOD_SLG, IF_REAL, " "),
  IOP("sub1l", HSM2_MOD_SUB1L, IF_REAL, " "),
  IOP("sub2l", HSM2_MOD_SUB2L, IF_REAL, " "),
  IOP("svgsl", HSM2_MOD_SVGSL, IF_REAL, " "),
  IOP("svgslp", HSM2_MOD_SVGSLP, IF_REAL, " "),
  IOP("svgswp", HSM2_MOD_SVGSWP, IF_REAL, " "),
  IOP("svgsw", HSM2_MOD_SVGSW, IF_REAL, " "),
  IOP("svbslp", HSM2_MOD_SVBSLP, IF_REAL, " "),
  IOP("slgl", HSM2_MOD_SLGL, IF_REAL, " "),
  IOP("slglp", HSM2_MOD_SLGLP, IF_REAL, " "),
  IOP("sub1lp", HSM2_MOD_SUB1LP, IF_REAL, " "),
  IOP("nsti", HSM2_MOD_NSTI, IF_REAL, "parameter for STI [1/cm^3]"),
  IOP("wsti", HSM2_MOD_WSTI, IF_REAL, "parameter for STI [m]"),
  IOP("wstil", HSM2_MOD_WSTIL, IF_REAL, "parameter for STI [?]"),
  IOP("wstilp", HSM2_MOD_WSTILP, IF_REAL, "parameter for STI [?]"),
  IOP("wstiw", HSM2_MOD_WSTIW, IF_REAL, "parameter for STI [?]"),
  IOP("wstiwp", HSM2_MOD_WSTIWP, IF_REAL, "parameter for STI [?]"),
  IOP("scsti1", HSM2_MOD_SCSTI1, IF_REAL, "parameter for STI [-]"),
  IOP("scsti2", HSM2_MOD_SCSTI2, IF_REAL, "parameter for STI [1/V]"),
  IOP("vthsti", HSM2_MOD_VTHSTI, IF_REAL, "parameter for STI"),
  IOP("vdsti", HSM2_MOD_VDSTI, IF_REAL, "parameter for STI [-]"),
  IOP("muesti1", HSM2_MOD_MUESTI1, IF_REAL, "STI Stress mobility parameter"),
  IOP("muesti2", HSM2_MOD_MUESTI2, IF_REAL, "STI Stress mobility parameter"),
  IOP("muesti3", HSM2_MOD_MUESTI3, IF_REAL, "STI Stress mobility parameter"),
  IOP("nsubpsti1", HSM2_MOD_NSUBPSTI1, IF_REAL, "STI Stress pocket impla parameter"),
  IOP("nsubpsti2", HSM2_MOD_NSUBPSTI2, IF_REAL, "STI Stress pocket impla parameter"),
  IOP("nsubpsti3", HSM2_MOD_NSUBPSTI3, IF_REAL, "STI Stress pocket impla parameter"),
  IOP("nsubcsti1", HSM2_MOD_NSUBCSTI1, IF_REAL, "STI Stress Parameter for Nsubc [-]"),
  IOP("nsubcsti2", HSM2_MOD_NSUBCSTI2, IF_REAL, "STI Stress Parameter for Nsubc [-]"),
  IOP("nsubcsti3", HSM2_MOD_NSUBCSTI3, IF_REAL, "STI Stress Parameter for Nsubc [-]"),
  IOP("lpext", HSM2_MOD_LPEXT, IF_REAL, "Pocket extension"),
  IOP("npext", HSM2_MOD_NPEXT, IF_REAL, "Pocket extension"),
  IOP("npextw", HSM2_MOD_NPEXTW, IF_REAL, "new model parameter NPEXTW"),
  IOP("npextwp", HSM2_MOD_NPEXTWP, IF_REAL, "new model parameter NPEXTWP"),
  IOP("scp22", HSM2_MOD_SCP22, IF_REAL, ""),
  IOP("scp21", HSM2_MOD_SCP21, IF_REAL, ""),
  IOP("bs1", HSM2_MOD_BS1, IF_REAL, ""),
  IOP("bs2", HSM2_MOD_BS2, IF_REAL, ""),
  IOP("cgso", HSM2_MOD_CGSO, IF_REAL, "G-S overlap capacitance per unit W [F/m]"),
  IOP("cgdo", HSM2_MOD_CGDO, IF_REAL, "G-D overlap capacitance per unit W [F/m]"),
  IOP("cgbo", HSM2_MOD_CGBO, IF_REAL, "G-B overlap capacitance per unit L [F/m]"),
  IOP("tpoly", HSM2_MOD_TPOLY, IF_REAL, "hight of poly gate [m]"),
  IOP("js0", HSM2_MOD_JS0, IF_REAL, "Saturation current density [A/m^2]"),
  IOP("js0sw", HSM2_MOD_JS0SW, IF_REAL, "Side wall saturation current density [A/m]"),
  IOP("nj", HSM2_MOD_NJ, IF_REAL, "Emission coefficient [-]"),
  IOP("njsw", HSM2_MOD_NJSW, IF_REAL, "Sidewall emission coefficient"),
  IOP("xti", HSM2_MOD_XTI, IF_REAL, "Junction current temparature exponent coefficient [-]"),
  IOP("cj", HSM2_MOD_CJ, IF_REAL, "Bottom junction capacitance per unit area at zero bias [F/m^2]"),
  IOP("cjsw", HSM2_MOD_CJSW, IF_REAL, "Source/drain sidewall junction capacitance grading coefficient per unit length at zero bias [F/m]"),
  IOP("cjswg", HSM2_MOD_CJSWG, IF_REAL, "Source/drain gate sidewall junction capacitance per unit length at zero bias [F/m]"),
  IOP("mj", HSM2_MOD_MJ, IF_REAL, "Bottom junction capacitance grading coefficient"),
  IOP("mjsw", HSM2_MOD_MJSW, IF_REAL, "Source/drain sidewall junction capacitance grading coefficient"),
  IOP("mjswg", HSM2_MOD_MJSWG, IF_REAL, "Source/drain gate sidewall junction capacitance grading coefficient"),
  IOP("pb", HSM2_MOD_PB, IF_REAL, "Bottom junction build-in potential  [V]"),
  IOP("pbsw", HSM2_MOD_PBSW, IF_REAL, "Source/drain sidewall junction build-in potential [V]"),
  IOP("pbswg", HSM2_MOD_PBSWG, IF_REAL, "Source/drain gate sidewall junction build-in potential [V]"),

  IOP("tcjbd", HSM2_MOD_TCJBD, IF_REAL, "Temperature dependence of czbd"),
  IOP("tcjbs", HSM2_MOD_TCJBS, IF_REAL, "Temperature dependence of czbs"),
  IOP("tcjbdsw", HSM2_MOD_TCJBDSW, IF_REAL, "Temperature dependence of czbdsw"),
  IOP("tcjbssw", HSM2_MOD_TCJBSSW, IF_REAL, "Temperature dependence of czbssw"),
  IOP("tcjbdswg", HSM2_MOD_TCJBDSWG, IF_REAL, "Temperature dependence of czbdswg"),
  IOP("tcjbsswg", HSM2_MOD_TCJBSSWG, IF_REAL, "Temperature dependence of czbsswg"),

  IOP("xti2", HSM2_MOD_XTI2, IF_REAL, " temperature coefficient [-]"),
  IOP("cisb", HSM2_MOD_CISB, IF_REAL, " reverse bias saturation current [-]"),
  IOP("cvb", HSM2_MOD_CVB, IF_REAL, " bias dependence coefficient of cisb [-]"),
  IOP("ctemp", HSM2_MOD_CTEMP, IF_REAL, " temperature coefficient [-]"),
  IOP("cisbk", HSM2_MOD_CISBK, IF_REAL, " reverse bias saturation current [A]"),
  IOP("cvbk", HSM2_MOD_CVBK, IF_REAL, " bias dependence coefficient of cisb [-]"),
  IOP("divx", HSM2_MOD_DIVX, IF_REAL, "  [1/V]"),

  IOP("clm1", HSM2_MOD_CLM1, IF_REAL, "parameter for CLM [-]"),
  IOP("clm2", HSM2_MOD_CLM2, IF_REAL, "parameter for CLM [1/m]"),
  IOP("clm3", HSM2_MOD_CLM3, IF_REAL, "parameter for CLM [-]"),
  IOP("clm5", HSM2_MOD_CLM5, IF_REAL, "parameter for CLM [-]"),
  IOP("clm6", HSM2_MOD_CLM6, IF_REAL, "parameter for CLM [um^{-clm5}]"),
  IOP("vover", HSM2_MOD_VOVER, IF_REAL, "parameter for overshoot [m^{voverp}]"),
  IOP("voverp", HSM2_MOD_VOVERP, IF_REAL, "parameter for overshoot [-]"),
  IOP("vovers", HSM2_MOD_VOVERS, IF_REAL, "parameter for overshoot [-]"),
  IOP("voversp", HSM2_MOD_VOVERSP, IF_REAL, "parameter for overshoot [-]"),

  IOP("wfc", HSM2_MOD_WFC, IF_REAL, "parameter for narrow channel effect [m*F/(cm^2)]"),
  IOP("nsubcw", HSM2_MOD_NSUBCW, IF_REAL, "Parameter for narrow channel effect "),
  IOP("nsubcwp", HSM2_MOD_NSUBCWP, IF_REAL, "Parameter for narrow channel effect "),
  IOP("nsubcmax", HSM2_MOD_NSUBCMAX, IF_REAL, "Parameter for narrow channel effect "),
  IOP("qme1", HSM2_MOD_QME1, IF_REAL, "parameter for quantum effect [mV]"),
  IOP("qme2", HSM2_MOD_QME2, IF_REAL, "parameter for quantum effect [V]"),
  IOP("qme3", HSM2_MOD_QME3, IF_REAL, "parameter for quantum effect [m]"),
  IOP("gidl1", HSM2_MOD_GIDL1, IF_REAL, "parameter for GIDL [?]"),
  IOP("gidl2", HSM2_MOD_GIDL2, IF_REAL, "parameter for GIDL [?]"),
  IOP("gidl3", HSM2_MOD_GIDL3, IF_REAL, "parameter for GIDL [?]"),
  IOP("gidl4", HSM2_MOD_GIDL4, IF_REAL, "parameter for GIDL [?]"),
  IOP("gidl5", HSM2_MOD_GIDL5, IF_REAL, "parameter for GIDL [?]"),
  IOP("gidl6",   HSM2_MOD_GIDL6, IF_REAL,   "parameter for GIDL [-]"),
  IOP("gidl7",   HSM2_MOD_GIDL7, IF_REAL,   "parameter for GIDL [-]"),
  IOP("gleak1", HSM2_MOD_GLEAK1, IF_REAL, "parameter for gate current [A*V^(-3/2)/C]"),
  IOP("gleak2", HSM2_MOD_GLEAK2, IF_REAL, "parameter for gate current [V^(-1/2)/m ]"),
  IOP("gleak3", HSM2_MOD_GLEAK3, IF_REAL, "parameter for gate current [-]"),
  IOP("gleak4", HSM2_MOD_GLEAK4, IF_REAL, "parameter for gate current [1/m]"),
  IOP("gleak5", HSM2_MOD_GLEAK5, IF_REAL, "parameter for gate current [V/m]"),
  IOP("gleak6", HSM2_MOD_GLEAK6, IF_REAL, "parameter for gate current [V]"),
  IOP("gleak7", HSM2_MOD_GLEAK7, IF_REAL, "parameter for gate current [m^2]"),
  IOP("glksd1", HSM2_MOD_GLKSD1, IF_REAL, "parameter for gate current [A*m/V^2]"),
  IOP("glksd2", HSM2_MOD_GLKSD2, IF_REAL, "parameter for gate current [1/(V*m)]"),
  IOP("glksd3", HSM2_MOD_GLKSD3, IF_REAL, "parameter for gate current [1/m]"),
  IOP("glkb1", HSM2_MOD_GLKB1, IF_REAL, "parameter for gate current [A/V^2]"),
  IOP("glkb2", HSM2_MOD_GLKB2, IF_REAL, "parameter for gate current [m/V]"),
  IOP("glkb3", HSM2_MOD_GLKB3, IF_REAL, "parameter for gate current [V]"),
  IOP("egig", HSM2_MOD_EGIG, IF_REAL, "parameter for gate current [V]"),
  IOP("igtemp2", HSM2_MOD_IGTEMP2, IF_REAL, "parameter for gate current [V*k]"),
  IOP("igtemp3", HSM2_MOD_IGTEMP3, IF_REAL, "parameter for gate current [V*k^2]"),
  IOP("vzadd0", HSM2_MOD_VZADD0, IF_REAL, "Vzadd at Vds=0  [V]"),
  IOP("pzadd0", HSM2_MOD_PZADD0, IF_REAL, "Pzadd at Vds=0  [V]"),
  IOP("nftrp", HSM2_MOD_NFTRP, IF_REAL, ""),
  IOP("nfalp", HSM2_MOD_NFALP, IF_REAL, ""),
  IOP("falph", HSM2_MOD_FALPH, IF_REAL, "parameter for 1/f noise"),
  IOP("cit", HSM2_MOD_CIT, IF_REAL, ""),
  IOP("kappa", HSM2_MOD_KAPPA, IF_REAL, "dielectric constant for high-k stacked gate"),
  IOP("vdiffj", HSM2_MOD_VDIFFJ, IF_REAL, "threshold voltage for S/D junction diode [V]"),
  IOP("dly1", HSM2_MOD_DLY1, IF_REAL, "parameter for transit time [-]"),
  IOP("dly2", HSM2_MOD_DLY2, IF_REAL, "parameter for transit time [-]"),
  IOP("dly3", HSM2_MOD_DLY3, IF_REAL, "parameter for trandforming bulk charge [s/F]"),
  IOP("tnom", HSM2_MOD_TNOM, IF_REAL, "nominal temperature [K]"),
  IOP("ovslp", HSM2_MOD_OVSLP, IF_REAL, ""),
  IOP("ovmag", HSM2_MOD_OVMAG, IF_REAL, ""),

  IOP("gbmin", HSM2_MOD_GBMIN, IF_REAL, ""),
  IOP("rbpb", HSM2_MOD_RBPB, IF_REAL, ""),
  IOP("rbpd", HSM2_MOD_RBPD, IF_REAL, ""),
  IOP("rbps", HSM2_MOD_RBPS, IF_REAL, ""),
  IOP("rbdb", HSM2_MOD_RBDB, IF_REAL, ""),
  IOP("rbsb", HSM2_MOD_RBSB, IF_REAL, ""),

  IOP("ibpc1", HSM2_MOD_IBPC1, IF_REAL, "parameter for Impact-Ionization Induced Bulk Potential Change"),
  IOP("ibpc2", HSM2_MOD_IBPC2, IF_REAL, "parameter for Impact-Ionization Induced Bulk Potential Change"),

  IOP("mphdfm", HSM2_MOD_MPHDFM, IF_REAL, "NSUBCDFM dependence of phonon scattering for DFM"),


  IOP("ptl", HSM2_MOD_PTL, IF_REAL, ""),
  IOP("ptp", HSM2_MOD_PTP, IF_REAL, ""),
  IOP("pt2", HSM2_MOD_PT2, IF_REAL, ""),
  IOP("ptlp", HSM2_MOD_PTLP, IF_REAL, ""),
  IOP("gdl", HSM2_MOD_GDL, IF_REAL, ""),
  IOP("gdlp", HSM2_MOD_GDLP, IF_REAL, ""),

  IOP("gdld", HSM2_MOD_GDLD, IF_REAL, ""),
  IOP("pt4", HSM2_MOD_PT4, IF_REAL, ""),
  IOP("pt4p", HSM2_MOD_PT4P, IF_REAL, ""),
  IOP("muephl2", HSM2_MOD_MUEPHL2, IF_REAL, ""),
  IOP("mueplp2", HSM2_MOD_MUEPLP2, IF_REAL, ""),
  IOP("nsubcw2", HSM2_MOD_NSUBCW2, IF_REAL, ""),
  IOP("nsubcwp2", HSM2_MOD_NSUBCWP2, IF_REAL, ""),
  IOP("muephw2", HSM2_MOD_MUEPHW2, IF_REAL, ""),
  IOP("muepwp2", HSM2_MOD_MUEPWP2, IF_REAL, ""),
  
/* WPE */
  IOP("web", HSM2_MOD_WEB, IF_REAL, "Description for the model parameter WPE web"),
  IOP("wec", HSM2_MOD_WEC, IF_REAL, "Description for the model parameter WPE wec"),
  IOP("nsubcwpe" , HSM2_MOD_NSUBCWPE, IF_REAL, "Description for the model parameter WPE nsubcwpe"),
  IOP("npextwpe" , HSM2_MOD_NPEXTWPE, IF_REAL, "Description for the model parameter WPE npextwpe"),
  IOP("nsubpwpe" , HSM2_MOD_NSUBPWPE, IF_REAL, "Description for the model parameter WPE nsubpwpe"),
  IOP("vgsmin", HSM2_MOD_VGSMIN, IF_REAL, "minimal/maximal expected Vgs (NMOS/PMOS) [V]"),
  IOP("sc3vbs", HSM2_MOD_SC3VBS, IF_REAL, "Vbs value for clamping sc3 [V]"),
  IOP("byptol", HSM2_MOD_BYPTOL, IF_REAL, "BYP_TOL_FACTOR for bypass control"),
  IOP("muecb0lp", HSM2_MOD_MUECB0LP, IF_REAL, "L dependence of MUECB0"),
  IOP("muecb1lp", HSM2_MOD_MUECB1LP, IF_REAL, "L dependence of MUECB1"),

  /* Depletion Mode MOSFET */
  IOP("ndepm", HSM2_MOD_NDEPM, IF_REAL, "N- layer concentlation of the depletion MOS model"),
  IOP("ndepml", HSM2_MOD_NDEPML, IF_REAL, "L dependence of NDEPM "),
  IOP("ndepmlp", HSM2_MOD_NDEPMLP, IF_REAL, "L dependence of NDEPM "),
  IOP("tndep", HSM2_MOD_TNDEP, IF_REAL, "N- layer depth of the depletion MOS model"),
  IOP("depleak", HSM2_MOD_DEPLEAK, IF_REAL, "leakage current modification parameter for the depletion MOS model"),
  IOP("depleakl", HSM2_MOD_DEPLEAKL, IF_REAL, "L dependence of DEPLEAK"),
  IOP("depleaklp", HSM2_MOD_DEPLEAKLP, IF_REAL, "L dependence of DEPLEAK"),
  IOP("depeta", HSM2_MOD_DEPETA, IF_REAL, "Vds dependence of threshold voltage for the depletion MOS model"),
  IOP("depmue0", HSM2_MOD_DEPMUE0, IF_REAL, " [-]"),
  IOP("depmue0l", HSM2_MOD_DEPMUE0L, IF_REAL, " [-]"),
  IOP("depmue0lp", HSM2_MOD_DEPMUE0LP, IF_REAL, " [-]"),
  IOP("depmue1", HSM2_MOD_DEPMUE1, IF_REAL, " [-]"),
  IOP("depmue1l", HSM2_MOD_DEPMUE1L, IF_REAL, " [-]"),
  IOP("depmue1lp", HSM2_MOD_DEPMUE1LP, IF_REAL, " [-]"),
  IOP("depmueback0", HSM2_MOD_DEPMUEBACK0, IF_REAL, " [-]"),
  IOP("depmueback0l", HSM2_MOD_DEPMUEBACK0L, IF_REAL, " [-]"),
  IOP("depmueback0lp", HSM2_MOD_DEPMUEBACK0LP, IF_REAL, " [-]"),
  IOP("depmueback1", HSM2_MOD_DEPMUEBACK1, IF_REAL, " [-]"),
  IOP("depmueback1l", HSM2_MOD_DEPMUEBACK1L, IF_REAL, " [-]"),
  IOP("depmueback1lp", HSM2_MOD_DEPMUEBACK1LP, IF_REAL, " [-]"),
  IOP("depmueph0", HSM2_MOD_DEPMUEPH0, IF_REAL, " [-]"),
  IOP("depmueph1", HSM2_MOD_DEPMUEPH1, IF_REAL, " [-]"),
  IOP("depvmax", HSM2_MOD_DEPVMAX, IF_REAL, " [-]"),
  IOP("depvmaxl", HSM2_MOD_DEPVMAXL, IF_REAL, " [-]"),
  IOP("depvmaxlp", HSM2_MOD_DEPVMAXLP, IF_REAL, " [-]"),
  IOP("depvdsef1", HSM2_MOD_DEPVDSEF1, IF_REAL, " [-]"),
  IOP("depvdsef1l", HSM2_MOD_DEPVDSEF1L, IF_REAL, " [-]"),
  IOP("depvdsef1lp", HSM2_MOD_DEPVDSEF1LP, IF_REAL, " [-]"),
  IOP("depvdsef2", HSM2_MOD_DEPVDSEF2, IF_REAL, " [-]"),
  IOP("depvdsef2l", HSM2_MOD_DEPVDSEF2L, IF_REAL, " [-]"),
  IOP("depvdsef2lp", HSM2_MOD_DEPVDSEF2LP, IF_REAL, " [-]"),
  IOP("depbb",   HSM2_MOD_DEPBB, IF_REAL,   " [-]"),
  IOP("depmuetmp", HSM2_MOD_DEPMUETMP, IF_REAL, " [-]"),

  /* binning parameters */
  IOP("lmin", HSM2_MOD_LMIN, IF_REAL, "Minimum length for the model"),
  IOP("lmax", HSM2_MOD_LMAX, IF_REAL, "Maximum length for the model"),
  IOP("wmin", HSM2_MOD_WMIN, IF_REAL, "Minimum width for the model"),
  IOP("wmax", HSM2_MOD_WMAX, IF_REAL, "Maximum width for the model"),
  IOP("lbinn", HSM2_MOD_LBINN, IF_REAL, "L modulation coefficient for binning"),
  IOP("wbinn", HSM2_MOD_WBINN, IF_REAL, "W modulation coefficient for binning"),

  /* Length dependence */
  IOP("lvmax", HSM2_MOD_LVMAX, IF_REAL, "Length dependence of vmax"),
  IOP("lbgtmp1", HSM2_MOD_LBGTMP1, IF_REAL, "Length dependence of bgtmp1"),
  IOP("lbgtmp2", HSM2_MOD_LBGTMP2, IF_REAL, "Length dependence of bgtmp2"),
  IOP("leg0", HSM2_MOD_LEG0, IF_REAL, "Length dependence of eg0"),
  IOP("llover", HSM2_MOD_LLOVER, IF_REAL, "Length dependence of lover"),
  IOP("lvfbover", HSM2_MOD_LVFBOVER, IF_REAL, "Length dependence of vfbover"),
  IOP("lnover", HSM2_MOD_LNOVER, IF_REAL, "Length dependence of nover"),
  IOP("lwl2", HSM2_MOD_LWL2, IF_REAL, "Length dependence of wl2"),
  IOP("lvfbc", HSM2_MOD_LVFBC, IF_REAL, "Length dependence of vfbc"),
  IOP("lnsubc", HSM2_MOD_LNSUBC, IF_REAL, "Length dependence of nsubc"),
  IOP("lnsubp", HSM2_MOD_LNSUBP, IF_REAL, "Length dependence of nsubp"),
  IOP("lscp1", HSM2_MOD_LSCP1, IF_REAL, "Length dependence of scp1"),
  IOP("lscp2", HSM2_MOD_LSCP2, IF_REAL, "Length dependence of scp2"),
  IOP("lscp3", HSM2_MOD_LSCP3, IF_REAL, "Length dependence of scp3"),
  IOP("lsc1", HSM2_MOD_LSC1, IF_REAL, "Length dependence of sc1"),
  IOP("lsc2", HSM2_MOD_LSC2, IF_REAL, "Length dependence of sc2"),
  IOP("lsc3", HSM2_MOD_LSC3, IF_REAL, "Length dependence of sc3"),
  IOP("lsc4", HSM2_MOD_LSC4, IF_REAL, "Length dependence of sc4"),
  IOP("lpgd1", HSM2_MOD_LPGD1, IF_REAL, "Length dependence of pgd1"),
//IOP("lpgd3", HSM2_MOD_LPGD3, IF_REAL, "Length dependence of pgd3"),
  IOP("lndep", HSM2_MOD_LNDEP, IF_REAL, "Length dependence of ndep"),
  IOP("lninv", HSM2_MOD_LNINV, IF_REAL, "Length dependence of ninv"),
  IOP("lmuecb0", HSM2_MOD_LMUECB0, IF_REAL, "Length dependence of muecb0"),
  IOP("lmuecb1", HSM2_MOD_LMUECB1, IF_REAL, "Length dependence of muecb1"),
  IOP("lmueph1", HSM2_MOD_LMUEPH1, IF_REAL, "Length dependence of mueph1"),
  IOP("lvtmp", HSM2_MOD_LVTMP, IF_REAL, "Length dependence of vtmp"),
  IOP("lwvth0", HSM2_MOD_LWVTH0, IF_REAL, "Length dependence of wvth0"),
  IOP("lmuesr1", HSM2_MOD_LMUESR1, IF_REAL, "Length dependence of muesr1"),
  IOP("lmuetmp", HSM2_MOD_LMUETMP, IF_REAL, "Length dependence of muetmp"),
  IOP("lsub1", HSM2_MOD_LSUB1, IF_REAL, "Length dependence of sub1"),
  IOP("lsub2", HSM2_MOD_LSUB2, IF_REAL, "Length dependence of sub2"),
  IOP("lsvds", HSM2_MOD_LSVDS, IF_REAL, "Length dependence of svds"),
  IOP("lsvbs", HSM2_MOD_LSVBS, IF_REAL, "Length dependence of svbs"),
  IOP("lsvgs", HSM2_MOD_LSVGS, IF_REAL, "Length dependence of svgs"),
  IOP("lnsti", HSM2_MOD_LNSTI, IF_REAL, "Length dependence of nsti"),
  IOP("lwsti", HSM2_MOD_LWSTI, IF_REAL, "Length dependence of wsti"),
  IOP("lscsti1", HSM2_MOD_LSCSTI1, IF_REAL, "Length dependence of scsti1"),
  IOP("lscsti2", HSM2_MOD_LSCSTI2, IF_REAL, "Length dependence of scsti2"),
  IOP("lvthsti", HSM2_MOD_LVTHSTI, IF_REAL, "Length dependence of vthsti"),
  IOP("lmuesti1", HSM2_MOD_LMUESTI1, IF_REAL, "Length dependence of muesti1"),
  IOP("lmuesti2", HSM2_MOD_LMUESTI2, IF_REAL, "Length dependence of muesti2"),
  IOP("lmuesti3", HSM2_MOD_LMUESTI3, IF_REAL, "Length dependence of muesti3"),
  IOP("lnsubpsti1", HSM2_MOD_LNSUBPSTI1, IF_REAL, "Length dependence of nsubpsti1"),
  IOP("lnsubpsti2", HSM2_MOD_LNSUBPSTI2, IF_REAL, "Length dependence of nsubpsti2"),
  IOP("lnsubpsti3", HSM2_MOD_LNSUBPSTI3, IF_REAL, "Length dependence of nsubpsti3"),
  IOP("lnsubcsti1", HSM2_MOD_LNSUBCSTI1, IF_REAL, "Length dependence of nsubcsti1"),
  IOP("lnsubcsti2", HSM2_MOD_LNSUBCSTI2, IF_REAL, "Length dependence of nsubcsti2"),
  IOP("lnsubcsti3", HSM2_MOD_LNSUBCSTI3, IF_REAL, "Length dependence of nsubcsti3"),
  IOP("lcgso", HSM2_MOD_LCGSO, IF_REAL, "Length dependence of cgso"),
  IOP("lcgdo", HSM2_MOD_LCGDO, IF_REAL, "Length dependence of cgdo"),
  IOP("ljs0", HSM2_MOD_LJS0, IF_REAL, "Length dependence of js0"),
  IOP("ljs0sw", HSM2_MOD_LJS0SW, IF_REAL, "Length dependence of js0sw"),
  IOP("lnj", HSM2_MOD_LNJ, IF_REAL, "Length dependence of nj"),
  IOP("lcisbk", HSM2_MOD_LCISBK, IF_REAL, "Length dependence of cisbk"),
  IOP("lclm1", HSM2_MOD_LCLM1, IF_REAL, "Length dependence of clm1"),
  IOP("lclm2", HSM2_MOD_LCLM2, IF_REAL, "Length dependence of clm2"),
  IOP("lclm3", HSM2_MOD_LCLM3, IF_REAL, "Length dependence of clm3"),
  IOP("lwfc", HSM2_MOD_LWFC, IF_REAL, "Length dependence of wfc"),
  IOP("lgidl1", HSM2_MOD_LGIDL1, IF_REAL, "Length dependence of gidl1"),
  IOP("lgidl2", HSM2_MOD_LGIDL2, IF_REAL, "Length dependence of gidl2"),
  IOP("lgleak1", HSM2_MOD_LGLEAK1, IF_REAL, "Length dependence of gleak1"),
  IOP("lgleak2", HSM2_MOD_LGLEAK2, IF_REAL, "Length dependence of gleak2"),
  IOP("lgleak3", HSM2_MOD_LGLEAK3, IF_REAL, "Length dependence of gleak3"),
  IOP("lgleak6", HSM2_MOD_LGLEAK6, IF_REAL, "Length dependence of gleak6"),
  IOP("lglksd1", HSM2_MOD_LGLKSD1, IF_REAL, "Length dependence of glksd1"),
  IOP("lglksd2", HSM2_MOD_LGLKSD2, IF_REAL, "Length dependence of glksd2"),
  IOP("lglkb1", HSM2_MOD_LGLKB1, IF_REAL, "Length dependence of glkb1"),
  IOP("lglkb2", HSM2_MOD_LGLKB2, IF_REAL, "Length dependence of glkb2"),
  IOP("lnftrp", HSM2_MOD_LNFTRP, IF_REAL, "Length dependence of nftrp"),
  IOP("lnfalp", HSM2_MOD_LNFALP, IF_REAL, "Length dependence of nfalp"),
  IOP("lvdiffj", HSM2_MOD_LVDIFFJ, IF_REAL, "Length dependence of vdiffj"),
  IOP("libpc1", HSM2_MOD_LIBPC1, IF_REAL, "Length dependence of ibpc1"),
  IOP("libpc2", HSM2_MOD_LIBPC2, IF_REAL, "Length dependence of ibpc2"),

  /* Width dependence */
  IOP("wvmax", HSM2_MOD_WVMAX, IF_REAL, "Width dependence of vmax"),
  IOP("wbgtmp1", HSM2_MOD_WBGTMP1, IF_REAL, "Width dependence of bgtmp1"),
  IOP("wbgtmp2", HSM2_MOD_WBGTMP2, IF_REAL, "Width dependence of bgtmp2"),
  IOP("weg0", HSM2_MOD_WEG0, IF_REAL, "Width dependence of eg0"),
  IOP("wlover", HSM2_MOD_WLOVER, IF_REAL, "Width dependence of lover"),
  IOP("wvfbover", HSM2_MOD_WVFBOVER, IF_REAL, "Width dependence of vfbover"),
  IOP("wnover", HSM2_MOD_WNOVER, IF_REAL, "Width dependence of nover"),
  IOP("wwl2", HSM2_MOD_WWL2, IF_REAL, "Width dependence of wl2"),
  IOP("wvfbc", HSM2_MOD_WVFBC, IF_REAL, "Width dependence of vfbc"),
  IOP("wnsubc", HSM2_MOD_WNSUBC, IF_REAL, "Width dependence of nsubc"),
  IOP("wnsubp", HSM2_MOD_WNSUBP, IF_REAL, "Width dependence of nsubp"),
  IOP("wscp1", HSM2_MOD_WSCP1, IF_REAL, "Width dependence of scp1"),
  IOP("wscp2", HSM2_MOD_WSCP2, IF_REAL, "Width dependence of scp2"),
  IOP("wscp3", HSM2_MOD_WSCP3, IF_REAL, "Width dependence of scp3"),
  IOP("wsc1", HSM2_MOD_WSC1, IF_REAL, "Width dependence of sc1"),
  IOP("wsc2", HSM2_MOD_WSC2, IF_REAL, "Width dependence of sc2"),
  IOP("wsc3", HSM2_MOD_WSC3, IF_REAL, "Width dependence of sc3"),
  IOP("wsc4", HSM2_MOD_WSC4, IF_REAL, "Width dependence of sc4"),
  IOP("wpgd1", HSM2_MOD_WPGD1, IF_REAL, "Width dependence of pgd1"),
//IOP("wpgd3", HSM2_MOD_WPGD3, IF_REAL, "Width dependence of pgd3"),
  IOP("wndep", HSM2_MOD_WNDEP, IF_REAL, "Width dependence of ndep"),
  IOP("wninv", HSM2_MOD_WNINV, IF_REAL, "Width dependence of ninv"),
  IOP("wmuecb0", HSM2_MOD_WMUECB0, IF_REAL, "Width dependence of muecb0"),
  IOP("wmuecb1", HSM2_MOD_WMUECB1, IF_REAL, "Width dependence of muecb1"),
  IOP("wmueph1", HSM2_MOD_WMUEPH1, IF_REAL, "Width dependence of mueph1"),
  IOP("wvtmp", HSM2_MOD_WVTMP, IF_REAL, "Width dependence of vtmp"),
  IOP("wwvth0", HSM2_MOD_WWVTH0, IF_REAL, "Width dependence of wvth0"),
  IOP("wmuesr1", HSM2_MOD_WMUESR1, IF_REAL, "Width dependence of muesr1"),
  IOP("wmuetmp", HSM2_MOD_WMUETMP, IF_REAL, "Width dependence of muetmp"),
  IOP("wsub1", HSM2_MOD_WSUB1, IF_REAL, "Width dependence of sub1"),
  IOP("wsub2", HSM2_MOD_WSUB2, IF_REAL, "Width dependence of sub2"),
  IOP("wsvds", HSM2_MOD_WSVDS, IF_REAL, "Width dependence of svds"),
  IOP("wsvbs", HSM2_MOD_WSVBS, IF_REAL, "Width dependence of svbs"),
  IOP("wsvgs", HSM2_MOD_WSVGS, IF_REAL, "Width dependence of svgs"),
  IOP("wnsti", HSM2_MOD_WNSTI, IF_REAL, "Width dependence of nsti"),
  IOP("wwsti", HSM2_MOD_WWSTI, IF_REAL, "Width dependence of wsti"),
  IOP("wscsti1", HSM2_MOD_WSCSTI1, IF_REAL, "Width dependence of scsti1"),
  IOP("wscsti2", HSM2_MOD_WSCSTI2, IF_REAL, "Width dependence of scsti2"),
  IOP("wvthsti", HSM2_MOD_WVTHSTI, IF_REAL, "Width dependence of vthsti"),
  IOP("wmuesti1", HSM2_MOD_WMUESTI1, IF_REAL, "Width dependence of muesti1"),
  IOP("wmuesti2", HSM2_MOD_WMUESTI2, IF_REAL, "Width dependence of muesti2"),
  IOP("wmuesti3", HSM2_MOD_WMUESTI3, IF_REAL, "Width dependence of muesti3"),
  IOP("wnsubpsti1", HSM2_MOD_WNSUBPSTI1, IF_REAL, "Width dependence of nsubpsti1"),
  IOP("wnsubpsti2", HSM2_MOD_WNSUBPSTI2, IF_REAL, "Width dependence of nsubpsti2"),
  IOP("wnsubpsti3", HSM2_MOD_WNSUBPSTI3, IF_REAL, "Width dependence of nsubpsti3"),
  IOP("wnsubcsti1", HSM2_MOD_WNSUBCSTI1, IF_REAL, "Wength dependence of nsubcsti1"),
  IOP("wnsubcsti2", HSM2_MOD_WNSUBCSTI2, IF_REAL, "Wength dependence of nsubcsti2"),
  IOP("wnsubcsti3", HSM2_MOD_WNSUBCSTI3, IF_REAL, "Wength dependence of nsubcsti3"),
  IOP("wcgso", HSM2_MOD_WCGSO, IF_REAL, "Width dependence of cgso"),
  IOP("wcgdo", HSM2_MOD_WCGDO, IF_REAL, "Width dependence of cgdo"),
  IOP("wjs0", HSM2_MOD_WJS0, IF_REAL, "Width dependence of js0"),
  IOP("wjs0sw", HSM2_MOD_WJS0SW, IF_REAL, "Width dependence of js0sw"),
  IOP("wnj", HSM2_MOD_WNJ, IF_REAL, "Width dependence of nj"),
  IOP("wcisbk", HSM2_MOD_WCISBK, IF_REAL, "Width dependence of cisbk"),
  IOP("wclm1", HSM2_MOD_WCLM1, IF_REAL, "Width dependence of clm1"),
  IOP("wclm2", HSM2_MOD_WCLM2, IF_REAL, "Width dependence of clm2"),
  IOP("wclm3", HSM2_MOD_WCLM3, IF_REAL, "Width dependence of clm3"),
  IOP("wwfc", HSM2_MOD_WWFC, IF_REAL, "Width dependence of wfc"),
  IOP("wgidl1", HSM2_MOD_WGIDL1, IF_REAL, "Width dependence of gidl1"),
  IOP("wgidl2", HSM2_MOD_WGIDL2, IF_REAL, "Width dependence of gidl2"),
  IOP("wgleak1", HSM2_MOD_WGLEAK1, IF_REAL, "Width dependence of gleak1"),
  IOP("wgleak2", HSM2_MOD_WGLEAK2, IF_REAL, "Width dependence of gleak2"),
  IOP("wgleak3", HSM2_MOD_WGLEAK3, IF_REAL, "Width dependence of gleak3"),
  IOP("wgleak6", HSM2_MOD_WGLEAK6, IF_REAL, "Width dependence of gleak6"),
  IOP("wglksd1", HSM2_MOD_WGLKSD1, IF_REAL, "Width dependence of glksd1"),
  IOP("wglksd2", HSM2_MOD_WGLKSD2, IF_REAL, "Width dependence of glksd2"),
  IOP("wglkb1", HSM2_MOD_WGLKB1, IF_REAL, "Width dependence of glkb1"),
  IOP("wglkb2", HSM2_MOD_WGLKB2, IF_REAL, "Width dependence of glkb2"),
  IOP("wnftrp", HSM2_MOD_WNFTRP, IF_REAL, "Width dependence of nftrp"),
  IOP("wnfalp", HSM2_MOD_WNFALP, IF_REAL, "Width dependence of nfalp"),
  IOP("wvdiffj", HSM2_MOD_WVDIFFJ, IF_REAL, "Width dependence of vdiffj"),
  IOP("wibpc1", HSM2_MOD_WIBPC1, IF_REAL, "Width dependence of ibpc1"),
  IOP("wibpc2", HSM2_MOD_WIBPC2, IF_REAL, "Width dependence of ibpc2"),

  /* Cross-term dependence */
  IOP("pvmax", HSM2_MOD_PVMAX, IF_REAL, "Cross-term dependence of vmax"),
  IOP("pbgtmp1", HSM2_MOD_PBGTMP1, IF_REAL, "Cross-term dependence of bgtmp1"),
  IOP("pbgtmp2", HSM2_MOD_PBGTMP2, IF_REAL, "Cross-term dependence of bgtmp2"),
  IOP("peg0", HSM2_MOD_PEG0, IF_REAL, "Cross-term dependence of eg0"),
  IOP("plover", HSM2_MOD_PLOVER, IF_REAL, "Cross-term dependence of lover"),
  IOP("pvfbover", HSM2_MOD_PVFBOVER, IF_REAL, "Cross-term dependence of vfbover"),
  IOP("pnover", HSM2_MOD_PNOVER, IF_REAL, "Cross-term dependence of nover"),
  IOP("pwl2", HSM2_MOD_PWL2, IF_REAL, "Cross-term dependence of wl2"),
  IOP("pvfbc", HSM2_MOD_PVFBC, IF_REAL, "Cross-term dependence of vfbc"),
  IOP("pnsubc", HSM2_MOD_PNSUBC, IF_REAL, "Cross-term dependence of nsubc"),
  IOP("pnsubp", HSM2_MOD_PNSUBP, IF_REAL, "Cross-term dependence of nsubp"),
  IOP("pscp1", HSM2_MOD_PSCP1, IF_REAL, "Cross-term dependence of scp1"),
  IOP("pscp2", HSM2_MOD_PSCP2, IF_REAL, "Cross-term dependence of scp2"),
  IOP("pscp3", HSM2_MOD_PSCP3, IF_REAL, "Cross-term dependence of scp3"),
  IOP("psc1", HSM2_MOD_PSC1, IF_REAL, "Cross-term dependence of sc1"),
  IOP("psc2", HSM2_MOD_PSC2, IF_REAL, "Cross-term dependence of sc2"),
  IOP("psc3", HSM2_MOD_PSC3, IF_REAL, "Cross-term dependence of sc3"),
  IOP("psc4", HSM2_MOD_PSC4, IF_REAL, "Cross-term dependence of sc4"),
  IOP("ppgd1", HSM2_MOD_PPGD1, IF_REAL, "Cross-term dependence of pgd1"),
//IOP("ppgd3", HSM2_MOD_PPGD3, IF_REAL, "Cross-term dependence of pgd3"),
  IOP("pndep", HSM2_MOD_PNDEP, IF_REAL, "Cross-term dependence of ndep"),
  IOP("pninv", HSM2_MOD_PNINV, IF_REAL, "Cross-term dependence of ninv"),
  IOP("pmuecb0", HSM2_MOD_PMUECB0, IF_REAL, "Cross-term dependence of muecb0"),
  IOP("pmuecb1", HSM2_MOD_PMUECB1, IF_REAL, "Cross-term dependence of muecb1"),
  IOP("pmueph1", HSM2_MOD_PMUEPH1, IF_REAL, "Cross-term dependence of mueph1"),
  IOP("pvtmp", HSM2_MOD_PVTMP, IF_REAL, "Cross-term dependence of vtmp"),
  IOP("pwvth0", HSM2_MOD_PWVTH0, IF_REAL, "Cross-term dependence of wvth0"),
  IOP("pmuesr1", HSM2_MOD_PMUESR1, IF_REAL, "Cross-term dependence of muesr1"),
  IOP("pmuetmp", HSM2_MOD_PMUETMP, IF_REAL, "Cross-term dependence of muetmp"),
  IOP("psub1", HSM2_MOD_PSUB1, IF_REAL, "Cross-term dependence of sub1"),
  IOP("psub2", HSM2_MOD_PSUB2, IF_REAL, "Cross-term dependence of sub2"),
  IOP("psvds", HSM2_MOD_PSVDS, IF_REAL, "Cross-term dependence of svds"),
  IOP("psvbs", HSM2_MOD_PSVBS, IF_REAL, "Cross-term dependence of svbs"),
  IOP("psvgs", HSM2_MOD_PSVGS, IF_REAL, "Cross-term dependence of svgs"),
  IOP("pnsti", HSM2_MOD_PNSTI, IF_REAL, "Cross-term dependence of nsti"),
  IOP("pwsti", HSM2_MOD_PWSTI, IF_REAL, "Cross-term dependence of wsti"),
  IOP("pscsti1", HSM2_MOD_PSCSTI1, IF_REAL, "Cross-term dependence of scsti1"),
  IOP("pscsti2", HSM2_MOD_PSCSTI2, IF_REAL, "Cross-term dependence of scsti2"),
  IOP("pvthsti", HSM2_MOD_PVTHSTI, IF_REAL, "Cross-term dependence of vthsti"),
  IOP("pmuesti1", HSM2_MOD_PMUESTI1, IF_REAL, "Cross-term dependence of muesti1"),
  IOP("pmuesti2", HSM2_MOD_PMUESTI2, IF_REAL, "Cross-term dependence of muesti2"),
  IOP("pmuesti3", HSM2_MOD_PMUESTI3, IF_REAL, "Cross-term dependence of muesti3"),
  IOP("pnsubpsti1", HSM2_MOD_PNSUBPSTI1, IF_REAL, "Cross-term dependence of nsubpsti1"),
  IOP("pnsubpsti2", HSM2_MOD_PNSUBPSTI2, IF_REAL, "Cross-term dependence of nsubpsti2"),
  IOP("pnsubpsti3", HSM2_MOD_PNSUBPSTI3, IF_REAL, "Cross-term dependence of nsubpsti3"),
  IOP("pnsubcsti1", HSM2_MOD_PNSUBCSTI1, IF_REAL, "Cross-term dependence of nsubcsti1"),
  IOP("pnsubcsti2", HSM2_MOD_PNSUBCSTI2, IF_REAL, "Cross-term dependence of nsubcsti2"),
  IOP("pnsubcsti3", HSM2_MOD_PNSUBCSTI3, IF_REAL, "Cross-term dependence of nsubcsti3"),
  IOP("pcgso", HSM2_MOD_PCGSO, IF_REAL, "Cross-term dependence of cgso"),
  IOP("pcgdo", HSM2_MOD_PCGDO, IF_REAL, "Cross-term dependence of cgdo"),
  IOP("pjs0", HSM2_MOD_PJS0, IF_REAL, "Cross-term dependence of js0"),
  IOP("pjs0sw", HSM2_MOD_PJS0SW, IF_REAL, "Cross-term dependence of js0sw"),
  IOP("pnj", HSM2_MOD_PNJ, IF_REAL, "Cross-term dependence of nj"),
  IOP("pcisbk", HSM2_MOD_PCISBK, IF_REAL, "Cross-term dependence of cisbk"),
  IOP("pclm1", HSM2_MOD_PCLM1, IF_REAL, "Cross-term dependence of clm1"),
  IOP("pclm2", HSM2_MOD_PCLM2, IF_REAL, "Cross-term dependence of clm2"),
  IOP("pclm3", HSM2_MOD_PCLM3, IF_REAL, "Cross-term dependence of clm3"),
  IOP("pwfc", HSM2_MOD_PWFC, IF_REAL, "Cross-term dependence of wfc"),
  IOP("pgidl1", HSM2_MOD_PGIDL1, IF_REAL, "Cross-term dependence of gidl1"),
  IOP("pgidl2", HSM2_MOD_PGIDL2, IF_REAL, "Cross-term dependence of gidl2"),
  IOP("pgleak1", HSM2_MOD_PGLEAK1, IF_REAL, "Cross-term dependence of gleak1"),
  IOP("pgleak2", HSM2_MOD_PGLEAK2, IF_REAL, "Cross-term dependence of gleak2"),
  IOP("pgleak3", HSM2_MOD_PGLEAK3, IF_REAL, "Cross-term dependence of gleak3"),
  IOP("pgleak6", HSM2_MOD_PGLEAK6, IF_REAL, "Cross-term dependence of gleak6"),
  IOP("pglksd1", HSM2_MOD_PGLKSD1, IF_REAL, "Cross-term dependence of glksd1"),
  IOP("pglksd2", HSM2_MOD_PGLKSD2, IF_REAL, "Cross-term dependence of glksd2"),
  IOP("pglkb1", HSM2_MOD_PGLKB1, IF_REAL, "Cross-term dependence of glkb1"),
  IOP("pglkb2", HSM2_MOD_PGLKB2, IF_REAL, "Cross-term dependence of glkb2"),
  IOP("pnftrp", HSM2_MOD_PNFTRP, IF_REAL, "Cross-term dependence of nftrp"),
  IOP("pnfalp", HSM2_MOD_PNFALP, IF_REAL, "Cross-term dependence of nfalp"),
  IOP("pvdiffj", HSM2_MOD_PVDIFFJ, IF_REAL, "Cross-term dependence of vdiffj"),
  IOP("pibpc1", HSM2_MOD_PIBPC1, IF_REAL, "Cross-term dependence of ibpc1"),
  IOP("pibpc2", HSM2_MOD_PIBPC2, IF_REAL, "Cross-term dependence of ibpc2"),

  IOP("vgs_max", HSM2_MOD_VGS_MAX, IF_REAL, "maximum voltage G-S branch"),
  IOP("vgd_max", HSM2_MOD_VGD_MAX, IF_REAL, "maximum voltage G-D branch"),
  IOP("vgb_max", HSM2_MOD_VGB_MAX, IF_REAL, "maximum voltage G-B branch"),
  IOP("vds_max", HSM2_MOD_VDS_MAX, IF_REAL, "maximum voltage D-S branch"),
  IOP("vbs_max", HSM2_MOD_VBS_MAX, IF_REAL, "maximum voltage B-S branch"),
  IOP("vbd_max", HSM2_MOD_VBD_MAX, IF_REAL, "maximum voltage B-D branch")

};

char *HSM2names[] = {
    "Drain",                    /* 1 */
    "Gate",                     /* 2 */
    "Source",                   /* 3 */
    "Bulk"                      /* 4 */
};

int	HSM2nSize = NUMELEMS(HSM2names);
int	HSM2pTSize = NUMELEMS(HSM2pTable);
int	HSM2mPTSize = NUMELEMS(HSM2mPTable);
int	HSM2iSize = sizeof(HSM2instance);
int	HSM2mSize = sizeof(HSM2model);

