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

#include <stdlib.h>

typedef struct {
    void    *data;                      // dll data
    double  *dll_ain;                   // dll analog inputs
    double  *dll_aout;                  // dll analog outputs
    uint8_t *dll_din;                   // dll digital inputs
    uint8_t *dll_dout;                  // dll digital outputs
    Digital_State_t *dout_old;          // max possible storage to track output changes
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
        instance->dll_ain = malloc(PORT_SIZE(ain) * sizeof(double));
        instance->dll_aout = malloc(PORT_SIZE(aout) * sizeof(double));
        instance->dll_din = malloc(PORT_SIZE(din) * sizeof(uint8_t));
        instance->dll_dout = malloc(PORT_SIZE(dout) * sizeof(uint8_t));        
        instance->dout_old = malloc(PORT_SIZE(dout) * sizeof(Digital_State_t));

        for (i=0; i<PORT_SIZE(aout); i++) {
           instance->dll_aout[i] = 0.0;
        }
        
        /* Set d-inputs load */
        for (i=0; i<PORT_SIZE(din); i++) {
            LOAD(din[i]) = PARAM(d_input_load);
        }              
    
        LOAD(clk) = PARAM(clk_load);
        
        if ( !PORT_NULL(reset) )
            LOAD(reset) = PARAM(reset_load);
        
        cm_message_printf("DLIB: Block has %i, %i, %i, %i ports\n", PORT_SIZE(ain), PORT_SIZE(aout), PORT_SIZE(din), PORT_SIZE(dout));
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


    for (i=0; i<PORT_SIZE(ain); i++) { /****** Read current analog values ******/
        instance->dll_ain[i] = INPUT(ain[i]);
    }                        

    if ( 0.0 == TIME ) {    /****** DC analysis...output w/o delays ******/
    
        for (i=0; i<PORT_SIZE(dout); i++) {
            instance->dout_old[i]       = UNKNOWN;
            OUTPUT_STATE(dout[i])       = UNKNOWN;
            OUTPUT_STRENGTH(dout[i])    = HI_IMPEDANCE;
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

            //cm_message_printf("DLIB: Step call\n");
            //cm_message_printf("DLIB: Digital inputs are:\n");
        
            for (i=0; i<PORT_SIZE(din); i++) {
                switch(INPUT_STATE(din[i])) {
                    case ZERO: instance->dll_din[i] = 0; break;
                    case ONE:  instance->dll_din[i] = 1; break;
                    default:   instance->dll_din[i] = rand() & 1; break;
                }
                //cm_message_printf("DLIB: Din %i - %i\n", i, instance->dll_din[i]);
            }

            //cm_message_printf("DLIB:Analog inputs are:\n");
            for (i=0; i<PORT_SIZE(ain); i++) {
                //cm_message_printf("DLIB: Ain %i - %f\n", i, instance->dll_ain[i]);
            }
            
            for (i=0; i<PORT_SIZE(dout); i++) {                
                
                Digital_State_t new_state = instance->dll_dout[i] ? ONE : ZERO;
                
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

    for (i=0; i<PORT_SIZE(aout); i++) {
        OUTPUT(aout[i]) = instance->dll_aout[i];
    }
}



