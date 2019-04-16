#ifndef CanTp_Cfg_H_
#define CanTp_Cfg_H_


#include "CanTp_Types.h"             // Farag's edition





#define CANTP_NSDU_CONFIG_LIST_SIZE		4
#define CANTP_NSDU_RUNTIME_LIST_SIZE	4


extern CanTp_ConfigType CanTp_Config;
extern const CanTp_TxNSduType CanTpTxNSdu[];
extern const CanTp_RxNSduType CanTpRxNSdu[];


#define CANTP_CONVERT_MS_TO_MAIN_CYCLES(x) 	(x)/CanTp_Config.CanTpMainFunctionPeriod



#endif		/* CANTP_CFG_H_ */
