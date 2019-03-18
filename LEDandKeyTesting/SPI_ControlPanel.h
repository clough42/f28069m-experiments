#ifndef __SPI_CONTROLPANEL_H
#define __SPI_CONTROLPANEL_H

#include "DSP28x_Project.h"

void InitControlPanel();
void SendControlPanelData(Uint16 data[], Uint16 ledMask);



#endif // __SPI_CONTROLPANEL_H
