/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
Modified: 2000 AlansFIxes
**********/

#include "ngspice/ngspice.h"
#include "ngspice/devdefs.h"
#include "ngspice/ifsim.h"
#include "mos2defs.h"
#include "ngspice/suffix.h"

IFparm MOS2pTable[] = { /* parameters */ 
 IOPU("m",       MOS2_M,     IF_REAL   , "Multiplier"),
 IOPU("l",       MOS2_L,     IF_REAL   , "Length"),
 IOPU("w",       MOS2_W,     IF_REAL   , "Width"),
 IOPU("ad",      MOS2_AD,    IF_REAL   , "Drain area"),
 IOPU("as",      MOS2_AS,    IF_REAL   , "Source area"),
 IOPU("pd",      MOS2_PD,    IF_REAL   , "Drain perimeter"),
 IOPU("ps",      MOS2_PS,    IF_REAL   , "Source perimeter"),
 OP( "id",          MOS2_CD,          IF_REAL,"Drain current"),
 OPR( "cd",          MOS2_CD,          IF_REAL,""),
 OP( "ibd",     MOS2_CBD,   IF_REAL,    "B-D junction current"),
 OP( "ibs",     MOS2_CBS,   IF_REAL,    "B-S junction current"),
 OP( "is",      MOS2_CS,    IF_REAL,    "Source current "),
 OP( "ig",      MOS2_CG,    IF_REAL,    "Gate current "),
 OP( "ib",      MOS2_CB,    IF_REAL,    "Bulk current "),
 OP( "vgs",     MOS2_VGS,   IF_REAL,    "Gate-Source voltage"),
 OP( "vds",     MOS2_VDS,   IF_REAL,    "Drain-Source voltage"),
 OP( "vbs",     MOS2_VBS,   IF_REAL,    "Bulk-Source voltage"),
 OPU( "vbd",     MOS2_VBD,   IF_REAL,    "Bulk-Drain voltage"),
 IOPU("nrd",     MOS2_NRD,   IF_REAL   , "Drain squares"),
 IOPU("nrs",     MOS2_NRS,   IF_REAL   , "Source squares"),
 IP("off",      MOS2_OFF,   IF_FLAG   , "Device initially off"),
 IOPAU("icvds",     MOS2_IC_VDS,IF_REAL   , "Initial D-S voltage"),
 IOPAU("icvgs",     MOS2_IC_VGS,IF_REAL   , "Initial G-S voltage"),
 IOPAU("icvbs",     MOS2_IC_VBS,IF_REAL   , "Initial B-S voltage"),
 IOPU("temp",    MOS2_TEMP,  IF_REAL    ,"Instance operating temperature"),
 IOPU("dtemp",   MOS2_DTEMP, IF_REAL   , "Instance temperature difference"),
 IP( "ic",      MOS2_IC,    IF_REALVEC, "Vector of D-S, G-S, B-S voltages"),
 IP( "sens_l",  MOS2_L_SENS,IF_FLAG,  "flag to request sensitivity WRT length"),
 IP( "sens_w",  MOS2_W_SENS,IF_FLAG,  "flag to request sensitivity WRT width"),
 /*
 OP( "cgs",     MOS2_CGS,   IF_REAL   , "Gate-Source capacitance"),
 OP( "cgd",     MOS2_CGD,   IF_REAL   , "Gate-Drain capacitance"),
 */
 OPU( "dnode",   MOS2_DNODE, IF_INTEGER, "Number of drain node"),
 OPU( "gnode",   MOS2_GNODE, IF_INTEGER, "Number of gate node"),
 OPU( "snode",   MOS2_SNODE, IF_INTEGER, "Number of source node"),
 OPU( "bnode",   MOS2_BNODE, IF_INTEGER, "Number of bulk node"),
 OPU( "dnodeprime", MOS2_DNODEPRIME, IF_INTEGER, 
        "Number of internal drain node"),
 OPU( "snodeprime", MOS2_SNODEPRIME, IF_INTEGER, 
        "Number of internal source node"),
 OP( "von",               MOS2_VON,           IF_REAL,    " "),
 OP( "vdsat",             MOS2_VDSAT,       IF_REAL,"Saturation drain voltage"),
 OPU( "sourcevcrit", MOS2_SOURCEVCRIT, IF_REAL,"Critical source voltage"),
 OPU( "drainvcrit",  MOS2_DRAINVCRIT,  IF_REAL,"Critical drain voltage"),
 OP( "rs", MOS2_SOURCERESIST, IF_REAL, "Source resistance"),
 OPU( "sourceconductance", MOS2_SOURCECONDUCT, IF_REAL, "Source conductance"),
 OP( "rd",  MOS2_DRAINRESIST,  IF_REAL, "Drain resistance"),
 OPU( "drainconductance",  MOS2_DRAINCONDUCT,  IF_REAL, "Drain conductance"),
 OP( "gm",      MOS2_GM,    IF_REAL,    "Transconductance"),
 OP( "gds",     MOS2_GDS,   IF_REAL,    "Drain-Source conductance"),
 OP( "gmb",    MOS2_GMBS,  IF_REAL,    "Bulk-Source transconductance"),
 OPR( "gmbs",    MOS2_GMBS,  IF_REAL,    ""),
 OPU( "gbd",     MOS2_GBD,   IF_REAL,    "Bulk-Drain conductance"),
 OPU( "gbs",     MOS2_GBS,   IF_REAL,    "Bulk-Source conductance"),
 OP( "cbd",   MOS2_CAPBD, IF_REAL,    "Bulk-Drain capacitance"),
 OP( "cbs",   MOS2_CAPBS, IF_REAL,    "Bulk-Source capacitance"),
 OP( "cgs",   MOS2_CAPGS, IF_REAL,    "Gate-Source capacitance"),
 OP( "cgd",   MOS2_CAPGD, IF_REAL,    "Gate-Drain capacitance"),
 OP( "cgb",   MOS2_CAPGB, IF_REAL,    "Gate-Bulk capacitance"),

 OPU( "cbd0", MOS2_CAPZEROBIASBD, IF_REAL,"Zero-Bias B-D junction capacitance"),
 OPU( "cbdsw0",MOS2_CAPZEROBIASBDSW,IF_REAL, " "),
 OPU( "cbs0", MOS2_CAPZEROBIASBS, IF_REAL,"Zero-Bias B-S junction capacitance"),
 OPU( "cbssw0", MOS2_CAPZEROBIASBSSW,IF_REAL," "),
 OPU("cqgs",MOS2_CQGS,IF_REAL,"Capacitance due to gate-source charge storage"),
 OPU("cqgd",MOS2_CQGD,IF_REAL,"Capacitance due to gate-drain charge storage"),
 OPU("cqgb",MOS2_CQGB,IF_REAL,"Capacitance due to gate-bulk charge storage"),
 OPU("cqbd",MOS2_CQBD,IF_REAL,"Capacitance due to bulk-drain charge storage"),
 OPU("cqbs",MOS2_CQBS,IF_REAL,"Capacitance due to bulk-source charge storage"),
 OPU( "qgs",     MOS2_QGS,   IF_REAL,    "Gate-Source charge storage"),
 OPU( "qgd",     MOS2_QGD,   IF_REAL,    "Gate-Drain charge storage"),
 OPU( "qgb",     MOS2_QGB,   IF_REAL,    "Gate-Bulk charge storage"),
 OPU( "qbd",     MOS2_QBD,   IF_REAL,    "Bulk-Drain charge storage"),
 OPU( "qbs",     MOS2_QBS,   IF_REAL,    "Bulk-Source charge storage"),

 OPU( "p",       MOS2_POWER, IF_REAL,    "Instantaneous power "),
 OPU( "sens_l_dc",         MOS2_L_SENS_DC,       IF_REAL,    
        "dc sensitivity wrt length"),
 OPU( "sens_l_real",       MOS2_L_SENS_REAL,     IF_REAL,    
        "real part of ac sensitivity wrt length"),
 OPU( "sens_l_imag",       MOS2_L_SENS_IMAG,     IF_REAL,    
        "imag part of ac sensitivity wrt length"),
 OPU( "sens_l_cplx",       MOS2_L_SENS_CPLX,     IF_COMPLEX, 
        "ac sensitivity wrt length"),
 OPU( "sens_l_mag",        MOS2_L_SENS_MAG,      IF_REAL,    
        "sensitivity wrt l of ac magnitude"),
 OPU( "sens_l_ph",         MOS2_L_SENS_PH,       IF_REAL,    
        "sensitivity wrt l of ac phase"),
 OPU( "sens_w_dc",         MOS2_W_SENS_DC,       IF_REAL,    
        "dc sensitivity wrt width"),
 OPU( "sens_w_real",       MOS2_W_SENS_REAL,     IF_REAL,    
        "dc sensitivity and real part of ac sensitivity wrt width"),
 OPU( "sens_w_imag",       MOS2_W_SENS_IMAG,     IF_REAL,    
        "imag part of ac sensitivity wrt width"),
 OPU( "sens_w_mag",        MOS2_W_SENS_MAG,      IF_REAL,    
        "sensitivity wrt w of ac magnitude"),
 OPU( "sens_w_ph",         MOS2_W_SENS_PH,       IF_REAL,    
        "sensitivity wrt w of ac phase"),
 OPU( "sens_w_cplx",       MOS2_W_SENS_CPLX,     IF_COMPLEX,    
        "ac sensitivity wrt width")
};

IFparm MOS2mPTable[] = { /* model parameters */
 OP("type",   MOS2_MOD_TYPE,   IF_STRING   ,"N-channel or P-channel MOS"),
 IOP("vto",   MOS2_MOD_VTO,   IF_REAL   ,"Threshold voltage"),
 IOPR("vt0",   MOS2_MOD_VTO,   IF_REAL   ,"Threshold voltage"),
 IOP("kp",    MOS2_MOD_KP,    IF_REAL   ,"Transconductance parameter"),
 IOP("gamma", MOS2_MOD_GAMMA, IF_REAL   ,"Bulk threshold parameter"),
 IOP("phi",   MOS2_MOD_PHI,   IF_REAL   ,"Surface potential"),
 IOP("lambda",MOS2_MOD_LAMBDA,IF_REAL   ,"Channel length modulation"),
 IOP("rd",    MOS2_MOD_RD,    IF_REAL   ,"Drain ohmic resistance"),
 IOP("rs",    MOS2_MOD_RS,    IF_REAL   ,"Source ohmic resistance"),
 IOP("cbd",   MOS2_MOD_CBD,   IF_REAL   ,"B-D junction capacitance"),
 IOP("cbs",   MOS2_MOD_CBS,   IF_REAL   ,"B-S junction capacitance"),
 IOP("is",    MOS2_MOD_IS,    IF_REAL   ,"Bulk junction sat. current"),
 IOP("pb",    MOS2_MOD_PB,    IF_REAL   ,"Bulk junction potential"),
 IOPA("cgso",  MOS2_MOD_CGSO,  IF_REAL   ,"Gate-source overlap cap."),
 IOPA("cgdo",  MOS2_MOD_CGDO,  IF_REAL   ,"Gate-drain overlap cap."),
 IOPA("cgbo",  MOS2_MOD_CGBO,  IF_REAL   ,"Gate-bulk overlap cap."),
 IOP("rsh",   MOS2_MOD_RSH,   IF_REAL   ,"Sheet resistance"),
 IOPA("cj",    MOS2_MOD_CJ,    IF_REAL   ,"Bottom junction cap per area"),
 IOP("mj",    MOS2_MOD_MJ,    IF_REAL   ,"Bottom grading coefficient"),
 IOPA("cjsw",  MOS2_MOD_CJSW,  IF_REAL   ,"Side junction cap per area"),
 IOP("mjsw",  MOS2_MOD_MJSW,  IF_REAL   ,"Side grading coefficient"),
 IOP("js",    MOS2_MOD_JS,    IF_REAL   ,"Bulk jct. sat. current density"),
 IOP("tox",   MOS2_MOD_TOX,   IF_REAL   ,"Oxide thickness"),
 IOP("ld",    MOS2_MOD_LD,    IF_REAL   ,"Lateral diffusion"),
 IOP("u0",    MOS2_MOD_U0,    IF_REAL   ,"Surface mobility"),
 IOPR("uo",    MOS2_MOD_U0,    IF_REAL   ,"Surface mobility"),
 IOP("fc",    MOS2_MOD_FC,    IF_REAL   ,"Forward bias jct. fit parm."),
 IP("nmos",   MOS2_MOD_NMOS,  IF_FLAG   ,"N type MOSfet model"),
 IP("pmos",   MOS2_MOD_PMOS,  IF_FLAG   ,"P type MOSfet model"),
 IOP("nsub",  MOS2_MOD_NSUB,  IF_REAL   ,"Substrate doping"),
 IOP("tpg",   MOS2_MOD_TPG,   IF_INTEGER,"Gate type"),
 IOP("nss",   MOS2_MOD_NSS,   IF_REAL   ,"Surface state density"),
 IOP("delta", MOS2_MOD_DELTA, IF_REAL   ,"Width effect on threshold"),
 IOP("uexp",  MOS2_MOD_UEXP,  IF_REAL   ,"Crit. field exp for mob. deg."),
 IOP("ucrit", MOS2_MOD_UCRIT, IF_REAL   ,"Crit. field for mob. degradation"),
 IOP("vmax",  MOS2_MOD_VMAX,  IF_REAL   ,"Maximum carrier drift velocity"),
 IOP("xj",    MOS2_MOD_XJ,    IF_REAL   ,"Junction depth"),
 IOP("neff",  MOS2_MOD_NEFF,  IF_REAL   ,"Total channel charge coeff."),
 IOP("nfs",   MOS2_MOD_NFS,   IF_REAL   ,"Fast surface state density"),
 IOPU("tnom",  MOS2_MOD_TNOM,  IF_REAL   ,"Parameter measurement temperature"),
 IOP("kf",     MOS2_MOD_KF,    IF_REAL   ,"Flicker noise coefficient"),
 IOP("af",     MOS2_MOD_AF,    IF_REAL   ,"Flicker noise exponent")
};

char *MOS2names[] = {
    "Drain",                    /* 1 */
    "Gate",                     /* 2 */
    "Source",                   /* 3 */
    "Bulk"                      /* 4 */
};

int	MOS2nSize = NUMELEMS(MOS2names);
int	MOS2pTSize = NUMELEMS(MOS2pTable);
int	MOS2mPTSize = NUMELEMS(MOS2mPTable);
int	MOS2iSize = sizeof(MOS2instance);
int	MOS2mSize = sizeof(MOS2model);
