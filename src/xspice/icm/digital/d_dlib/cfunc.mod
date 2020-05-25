/*.......1.........2.........3.........4.........5.........6.........7.........8
================================================================================

FILE d_pil/cfunc.mod

Copyright 2020
Rhard

AUTHORS                      

    25 Mai 2020     Rhard


SUMMARY

    This file contains the interface specification file for the 
    hybrid dlib code model.
    
===============================================================================*/

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

typedef struct {
    void    *data;
    uint8_t N_din, N_dout;          // number of inputs/outputs bytes   
    Digital_State_t dout_old[256];  // max possible storage to track output changes
} Instance_t;


void cm_dlib(ARGS) {

    int                            i;   /* generic loop counter index */
               
    Digital_State_t           *reset,   /* storage for clock value  */
                          *reset_old,   /* previous clock value     */
                                *clk,   /* storage for clock value  */
                            *clk_old;   /* previous clock value     */

    Instance_t             *instance;   /* Structure containing process vars */
    
    if (INIT) {  /*** Test for INIT == TRUE. If so, allocate storage, etc. ***/
        
        /* Allocate storage for reset and clc inputs */
        cm_event_alloc(0,sizeof(Digital_State_t));
        cm_event_alloc(1,sizeof(Digital_State_t));
        
        /* Assign the storage for reset and clc inputs */
        clk   = clk_old   = (Digital_State_t *)cm_event_get_ptr(0,0);
        reset = reset_old = (Digital_State_t *)cm_event_get_ptr(1,0);
        
        /* Create block instance data */
        STATIC_VAR(dlib_instance) = malloc(sizeof(Instance_t));        
        instance = STATIC_VAR(dlib_instance);
        instance->N_din = PORT_SIZE(din);
        instance->N_dout = PORT_SIZE(dout);
        
        /* Set d-inputs load */
        for (i=0; i<PORT_SIZE(din); i++) {
            LOAD(din[i]) = PARAM(d_input_load);
        }              
    
        LOAD(clk) = PARAM(clk_load);
        
        if ( !PORT_NULL(reset) )
            LOAD(reset) = PARAM(reset_load);
        
        cm_message_printf("DLIB: Block initialized\n");

    } else {    /*** This is not an initialization pass...retrieve storage
                   addresses and calculate new outputs, if required. ***/
        
        /* Get block instance data */
        instance = STATIC_VAR(dlib_instance);
        
        /* Get reset and clc input storage */
        clk = (Digital_State_t *) cm_event_get_ptr(0,0);
        clk_old = (Digital_State_t *) cm_event_get_ptr(0,1);

        reset = (Digital_State_t *) cm_event_get_ptr(1,0);
        reset_old = (Digital_State_t *) cm_event_get_ptr(1,1);
        
    }
                            

    if ( 0.0 == TIME ) {    /****** DC analysis...output w/o delays ******/
    
        for (i=0; i<PORT_SIZE(dout); i++) {
            instance->dout_old[i]       = UNKNOWN;
            OUTPUT_STATE(dout[i])       = UNKNOWN;
            OUTPUT_STRENGTH(dout[i])    = HI_IMPEDANCE;
        }
        
        for (i=0; i<PORT_SIZE(aout); i++) {
            OUTPUT(aout[i]) = 0.0;
        }
        
    } else {  /*** TIME == 0.0 => set outputs to input value... ***/

        *clk = INPUT_STATE(clk);

        if ( PORT_NULL(reset) ) {
            *reset = *reset_old = ZERO;
        }
        else {
            *reset = INPUT_STATE(reset);
        }

        if (*clk != *clk_old && ONE == *clk) {
        
            uint8_t _dout[instance->N_dout];                    
            uint8_t _din[instance->N_din];
            memset(_din, 0, PORT_SIZE(din));

            for (i=0; i<PORT_SIZE(din); i++) {
                switch(INPUT_STATE(din[i])) {
                    case ZERO: _din[i] = 0; break;
                    case ONE:  _din[i] = 1; break;
                    default:   _din[i] = random() & 1; break;
                }
            }

            //dprocess_exchangedata(local_process, (ONE != *reset) ? TIME : -TIME, din, dout);
            
            for (int i=0; i<PORT_SIZE(dout); i++) {
                
                Digital_State_t new_state = _dout[i] ? ONE : ZERO;
                
                if (new_state != instance->dout_old[i]) {
                    OUTPUT_STATE(dout[i])    = new_state;
                    OUTPUT_STRENGTH(dout[i]) = STRONG;
                    OUTPUT_DELAY(dout[i])    = PARAM(clk_delay);
                    instance->dout_old[i]    = new_state;
                }
                else {
                    OUTPUT_CHANGED(dout[i]) = FALSE;
                }                                    
            }
            
        } else {
        
            for (i=0; i<PORT_SIZE(dout); i++) {
                OUTPUT_CHANGED(dout[i]) = FALSE;
            }
            
        }
    }
}



