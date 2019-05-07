/////*
//// ============================================================================
//// Name        : cantp.c
//// Author      : CAN_TP_Team
//// Version     :
//// Copyright   : Your copyright notice
//// Description : The CanTp Main Task is Segmentation and reassembling
//// ============================================================================
//// */
//
/* add this line to test github bash */
#include <CanTP/CanTpRuntime.h>
#include <CanTP/Helper_Functions.h>
#include <stdio.h>
#include <stdlib.h>
#include "CanTp_Types.h"
#include "CanTp_Cfg.h"
#include "CanTp.h"
//#include "Helper_Functions.h"
#include "CanTp_Cbk.h"
#include "SchM_CanTp.h"
#include "MemMap.h"
#include "Det.h"


extern CanTp_RunTimeDataType CanTpRunTimeData;
extern CanTp_ConfigType CanTp_Config;
void initTx15765RuntimeData(CanTp_ChannelPrivateType *txRuntimeParams)
{
    txRuntimeParams->iso15765.state = IDLE;
    txRuntimeParams->iso15765.NasNarPending = FALSE;
    txRuntimeParams->iso15765.framesHandledCount = 0;
    txRuntimeParams->iso15765.nextFlowControlCount = 0;
    txRuntimeParams->pdurBufferCount = 0;
    txRuntimeParams->transferTotal = 0;
    txRuntimeParams->transferCount = 0;
    txRuntimeParams->mode = CANTP_TX_WAIT;
    txRuntimeParams->pdurBuffer = NULL;
}

void initRx15765RuntimeData(CanTp_ChannelPrivateType *rxRuntimeParams)
{

    rxRuntimeParams->iso15765.state = IDLE;
    rxRuntimeParams->iso15765.NasNarPending = FALSE;
    rxRuntimeParams->iso15765.framesHandledCount = 0;
    rxRuntimeParams->iso15765.nextFlowControlCount = 0;
    rxRuntimeParams->pdurBufferCount = 0;
    rxRuntimeParams->transferTotal = 0;
    rxRuntimeParams->transferCount = 0;
    rxRuntimeParams->mode = CANTP_RX_WAIT;
    rxRuntimeParams->pdurBuffer = NULL;
    rxRuntimeParams->Buffersize = 0;
}

/************************************************** Stup Functions ***********************************************************/

void PduR_CanTpRxIndication(PduIdType CanTpRxPduId,NotifResultType Result)
{
	;

}

BufReq_ReturnType PduR_CanTpStartOfReception(PduIdType id,PduInfoType *info,PduLengthType length,PduLengthType *Buffersize)
{
	*Buffersize = 40;
	return BUFREQ_OK;
}

BufReq_ReturnType PduR_CanTpCopyRxData(PduIdType id,PduInfoType *info,PduLengthType *Buffersize)
{

	*Buffersize = 30;
	return BUFREQ_OK;
}

//
//Std_ReturnType CanIf_Transmit(const CanTp_RxNSduType, PduInfoPtr)
//{
//
//}


void CanTp_RxIndication(PduIdType CanTpRxPduId,const PduInfoType *CanTpRxPduPtr)
{

	const CanTp_RxNSduType *rxConfigParams;			 // Params reside in ROM.
	const CanTp_TxNSduType *txConfigParams;
	CanTp_AddressingFormatType addressingFormat; 	 // Configured
	CanTp_ChannelPrivateType *runtimeParams = NULL;  // Params reside in RAM.
	ISO15765FrameType frameType;
	PduIdType CanTpTxNSduId, CanTpRxNSduId;
	CanTpRxNSduId = CanTpRxPduId;
	uint16 TxIndex, RxIndex;
//	TxIndex = getindex();
	RxIndex = CanTpRxPduId;   /* Omar_Emad tells me that id = index */

	//Check if PduId is valid          /* we know that the value of id we recieve is already exist */
//	if (CanTpRxPduId >= CANTP_RXID_LIST_SIZE)
//	{
//		return;
//	}

	addressingFormat = CanTp_Config.CanTpChannel.CanTpRxNSdu[RxIndex].CanTpRxAddressingFormat;

	/* TODO John - Use a different indication of not set than 0xFFFF? */
	frameType = getFrameType(&addressingFormat, CanTpRxPduPtr);



				/* in case of tx */

	if( frameType == FLOW_CONTROL_CTS_FRAME )
	{
		/* by check an al index mazboot */
		if( TxIndex != 0xFFFF )
		{
//	//		CanTpTxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex;
//			//txConfigParams = &CanTpConfig.CanTpNSduList[CanTpTxNSduId].configData.CanTpTxNSdu;
//			txConfigParams = CanTp_Config.CanTpChannel.CanTpTxNSdu[CanTpTxNSduId];
//			runtimeParams = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
		}
		else
		{
			//Invalid FC received
			return;
		}
		rxConfigParams = NULL;
	}

		/* in case of rx */

	else   	 /*{SF,FF,CF}*/
	{
		if( RxIndex != 0xFFFF )
		{
		//	CanTpRxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex; /*CanTpRxPDUID = CanTpRxSDUID  so we don't need this line */
			rxConfigParams = &CanTp_Config.CanTpChannel.CanTpRxNSdu[RxIndex];
			runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel];	/* Question: Do we need CanTpRxChannel ??  */
		}
		else
		{
			//Invalid Frame received
			return;
		}
		txConfigParams = NULL;
	}



	switch (frameType)
	{
	case SINGLE_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}

		break;
	}

	case FIRST_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}

	case CONSECUTIVE_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleConsecutiveFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}

	case FLOW_CONTROL_CTS_FRAME:
	{
		if (txConfigParams != NULL)
		{
			handleFlowControlFrame(txConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}
	case INVALID_FRAME:
	{
		break;
	}

	default:
		break;
	}
}




///****************************************************** Primitive Functions *********************************************************///

///* This Function initate the global parameters of the CanTp Module and move the state to CanTp_ON if there is No Error *///
void CanTp_Init( const CanTp_ConfigType* CfgPtr )
{


		CanTp_ChannelPrivateType *runtimeData;
		//uint8 TxChannel;
		//uint8 RxChannel;


		uint8 i;
		for (i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++)
			{



				if ( CfgPtr->CanTpChannel.direction == IS015765_TRANSMIT )
				{
										/* For Tx */

					/* this if handle if CanTpTxChannel < Runtime_list_size
					 *then access the  CanTpTxChannel element in the runtimeDataList
					 * else access the last item in the runtimeDataList
					 */
							if ((uint8)CfgPtr->CanTpChannel.CanTpTxNSdu[i].CanTpTxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE)
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CfgPtr->CanTpChannel.CanTpTxNSdu[i].CanTpTxChannel];
							}
							 else
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
							}

							initTx15765RuntimeData( runtimeData );

				}

				else
				{
										/* For Rx */
					/* this if handle if CanTpTxChannel < Runtime_list_size
					 *then access the  CanTpTxChannel element in the runtimeDataList
					 * else access the last item in the runtimeDataList
					 */

							if (CfgPtr->CanTpChannel.CanTpRxNSdu[i].CanTpRxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE)
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CfgPtr->CanTpChannel.CanTpRxNSdu[i].CanTpRxChannel];
							}
							else
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
							}
							initRx15765RuntimeData( runtimeData );

				}
			}

		CanTpRunTimeData.internalState = CANTP_ON;    /* if the initfunc finished correcltly without errors ,then move into CANTP_ON state */

}

//Std_ReturnType CanTp_Transmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr )      		 // our sws
//Std_ReturnType CanTp_Transmit( PduIdType CanTpTxSduId, const PduInfoType* PduInfoPtr )  		 // their sws


/* this function used to copy the Data Length of the data required to be sent from  PDURBuffer to the CanIF Buffer in FF or SF */
Std_ReturnType CanTp_Transmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr )
{
	const CanTp_TxNSduType *txConfig = NULL;
	CanTp_ChannelPrivateType *txRuntime = NULL;
	Std_ReturnType ret = 0;
	PduIdType CanTp_InternalTxNSduId;							/* Unused Variable Now */

	/*Here we should make a fuction get the Txid and return the correct index in the cfg.c*/

		txConfig	=  &CanTp_Config.CanTpChannel.CanTpTxNSdu[TxPduId];        		             // habd mny :D

//		CanTp_InternalTxNSduId = CanTpConfig.CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex;    	 // lsa hnshofha bokra

//		txConfig =&CanTpConfig.CanTpNSduList[CanTp_InternalTxNSduId].configData.CanTpTxNSdu;  	 // lsa hnshofha bokra

		txRuntime = &CanTpRunTimeData.runtimeDataList[txConfig->CanTpTxChannel]; 				 // Runtime data.

		if (txRuntime->iso15765.state == IDLE)
		{

			ISO15765FrameType iso15765Frame;
			txRuntime->canFrameBuffer.byteCount = 0;
			txRuntime->pdurBuffer = NULL;                   // Farag change this from 0 to NULL
			txRuntime->transferCount = 0;
			txRuntime->iso15765.framesHandledCount = 0;
			txRuntime->transferTotal = PduInfoPtr->SduLength;     /* copy SduLength to transferTotal */
//			txRuntime->pdurBuffer = PduInfoPtr->SduDataPtr
			txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNcs);
			txRuntime->mode = CANTP_TX_PROCESSING;

			iso15765Frame = calcRequiredProtocolFrameType(txConfig, txRuntime);

			if (txConfig->CanTpTxAddressingFormat == CANTP_EXTENDED)
			{
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
					 txConfig->CanTpNTa.CanTpNTa;					// putting CanTpNTa in the first byte of data in case of extended 		 // Target address.
			}
			switch(iso15765Frame)
			{
			/* This only put the Data length in the correct position in pci */

			/*TODO: We Will support id Can_DL > 8 */
			case SINGLE_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_SF | (uint8)(txRuntime->transferTotal);   /* Can_DL < 8 so it will represent in 8 bits ( putting the SduLength at byte 0 )*/

				/*	Note that :	PduLengthType  is  16 bit */

				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;   /* Activate a TP Task */
				break;


			case FIRST_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_FF | (uint8)((txRuntime->transferTotal & 0xf00) >> 8);  /* putting the  Byte 1  */


				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						(uint8)(txRuntime->transferTotal & 0xff);						      /* putting the  Byte 2  */

				// setup block size so that state machine waits for flow control after first frame
				txRuntime->iso15765.nextFlowControlCount = 1;
				txRuntime->iso15765.BS = 1;						/* setting the block size and we can also set STmin  */
				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
				break;

			default:
				ret = E_NOT_OK;
			}
		}
		else							/* if state not idle make  ret = E_NOT_OK */
		{
			ret = E_NOT_OK;
		}

	return ret; 		// CAN level error code.
}


/* This Function move the state to CanTp_OFF*/
void CanTp_Shutdown(void)
{

	CanTpRunTimeData.internalState = CANTP_OFF;

}


void CanTp_MainFunction(void)
{
	BufReq_ReturnType ret;         		    /* enum = {BUFREQ_OK ,BUFREQ_NOT_OK,BUFREQ_BUSY,BUFREQ_OVFL }*/

	PduLengthType bytesWrittenToSduRBuffer;

	CanTp_ChannelPrivateType *txRuntimeListItem = NULL;
	CanTp_ChannelPrivateType *rxRuntimeListItem = NULL;

	const CanTp_TxNSduType *txConfigListItem = NULL;
	const CanTp_RxNSduType *rxConfigListItem = NULL;


	uint8 i = 0;
	for( i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++ )
	{
				/* in case of TX */

		if ( CanTp_Config.CanTpChannel.direction == IS015765_TRANSMIT )
		{
			txConfigListItem = (CanTp_TxNSduType*)&CanTp_Config.CanTpChannel.CanTpTxNSdu[i];
			txRuntimeListItem = &CanTpRunTimeData.runtimeDataList[txConfigListItem->CanTpTxChannel];

			switch (txRuntimeListItem->iso15765.state)
			{

			case TX_WAIT_STMIN:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount); 		// Make sure that STmin timer has expired.
				if (txRuntimeListItem->iso15765.stateTimeoutCount != 0)
				{
					break;
				}

				txRuntimeListItem->iso15765.state = TX_WAIT_TRANSMIT;


			case TX_WAIT_TRANSMIT:
			{
				ret = sendNextTxFrame(txConfigListItem, txRuntimeListItem);

				if ( ret == BUFREQ_OK )
				{
					// successfully sent frame
				}

				else if( ret == BUFREQ_BUSY )
				{
					// check N_Cs timeout
					TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
					if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
					{
						txRuntimeListItem->iso15765.state = IDLE;
						txRuntimeListItem->mode = CANTP_TX_WAIT;
						//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);    			 // it is not important in the core Now
					}
					else
					{
						// For MISRA Rules only as i remember
					}
				}

				else
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); 				 // it is not important in the core Now
				}
				break;
			}

			case TX_WAIT_FLOW_CONTROL:

				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); 			    // it is not important in the core Now
				}
				break;

			case TX_WAIT_TX_CONFIRMATION:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);
				}
				break;

			default:
				break;

			}
		}



			/* in case of RX */

		else
		{
			rxConfigListItem =(CanTp_RxNSduType*)&CanTp_Config.CanTpChannel.CanTpRxNSdu[i];
			rxRuntimeListItem = &CanTpRunTimeData.runtimeDataList [ rxConfigListItem->CanTpRxChannel ];

			switch (rxRuntimeListItem->iso15765.state)
			{
			case RX_WAIT_CONSECUTIVE_FRAME:
			{
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);

				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				}
				break;
			}

			case RX_WAIT_SDU_BUFFER:
			{
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				/* We end up here if we have requested a buffer from the
				 * PDUR but the response have been BUSY. We assume
				 * we have data in our local buffer and we are expected
				 * to send a flow-control clear to send (CTS).
				 */
				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);			  /* CanTp_00214 */
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				}


				else			//	stateTimeoutCount != 0
				{
					PduLengthType bytesRemaining = 0;

					 /* copies from local buffer to PDUR buffer. */
					ret = copySegmentToPduRRxBuffer(rxConfigListItem,rxRuntimeListItem,rxRuntimeListItem->canFrameBuffer.data,rxRuntimeListItem->canFrameBuffer.byteCount
							);
					bytesRemaining = rxRuntimeListItem->transferTotal -  rxRuntimeListItem->transferCount;
					if (bytesRemaining > 0)
					{
						sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, ret ); 			/* (Busy or CTS) */
					}

					if (ret == BUFREQ_OK)
					{
						if ( bytesRemaining > 0 )
						{
							rxRuntimeListItem->iso15765.stateTimeoutCount = (rxConfigListItem->CanTpNcr);  //UH
							rxRuntimeListItem->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
						}
						else
						{
							//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_OK);			  /* CanTp_00214 */
							rxRuntimeListItem->iso15765.state = IDLE;
							rxRuntimeListItem->mode = CANTP_RX_WAIT;
						}
					}
					else if (ret == BUFREQ_NOT_OK )
					{
						//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);        /* CanTp_00214 */
						rxRuntimeListItem->iso15765.state = IDLE;
						rxRuntimeListItem->mode = CANTP_RX_WAIT;
					}
					else if ( ret == BUFREQ_BUSY )
					{

					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

///****************************************************** int main *********************************************************/
//
//
////int main()
////{
////	//uint8 Array[2] = {'S','A'};
////	PduInfoType test;
////	test.SduDataPtr[0] = 0x06;
////	test.SduDataPtr[1] = 'S';
////	test.SduDataPtr[2] = 'A';
////	test.SduDataPtr[3] = 'L';
////	test.SduDataPtr[4] = 'M';
////	test.SduDataPtr[5] = 'A';
////
////
////	test.SduLength=6;
////	CanTp_Init(&CanTp_Config);
//////	CanTp_Transmit(1, &test );
////
////	CanTp_RxIndication(1,&test);
////
////	return EXIT_SUCCESS;
////
////}
//
////int main(void) {
////	int x = 10;
////	int z= x+ 1;
////	puts("Mohamed");
////	printf("%d\n",x);
////
////	printf("%d",z);
////
////
////
////	return EXIT_SUCCESS;
////}
