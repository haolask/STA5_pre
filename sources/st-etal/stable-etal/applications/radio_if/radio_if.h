//!
//!  \file    radio_if.h
//!  \brief   <i><b> Radio interface main function entry point </b></i>
//!  \details Global shared context.
//!  \author  David Pastor
//!

#ifndef RADIO_IF_H
#define RADIO_IF_H

#ifdef __cplusplus
extern "C" {
#endif

extern STECI_UART_paramsTy rif_SteciUartParams;

///
// Exported functions
///
extern tVoid rif_stop_rif(tVoid);

#ifdef __cplusplus
}
#endif

#endif // RADIO_IF_H

// End of file
