/*************
 * Header file for gnuplot.c
 * 2008 Stefano Pedretti
 ************/

#ifndef ngspice_GNUPLOT_H
#define ngspice_GNUPLOT_H

void ft_gnuplot(double *xlims, double *ylims,
        const char *filename, const char *title,
        const char *xlabel, const char *ylabel,
        GRIDTYPE gridtype, PLOTTYPE plottype,
        struct dvec *vecs);


void ft_writesimple(double *xlims, double *ylims,
        const char *filename, const char *title,
        const char *xlabel, const char *ylabel,
        GRIDTYPE gridtype, PLOTTYPE plottype,
        struct dvec *vecs);

#endif
