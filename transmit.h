#ifndef TRANSMIT
#define TRANSMIT

void SetupTransmit(char* hostnm, char* p, List* l);
void CancelTransmit();
void CloseTransmit();
void SignalTransmit();

#endif