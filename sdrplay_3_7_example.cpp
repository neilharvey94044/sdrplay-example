// sdrplay_api_sample_app.c : Simple console application showing the use of the API
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include "sdrplay_api.h"
int masterInitialised = 0;
int slaveUninitialised = 0;
sdrplay_api_DeviceT* chosenDevice = NULL;


void StreamACallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params, unsigned int numSamples, unsigned int reset, void* cbContext)
{
	if (reset)
		printf("sdrplay_api_StreamACallback: numSamples=%d\n", numSamples);
	// Process stream callback data here
	return;
}

void StreamBCallback(short* xi, short* xq, sdrplay_api_StreamCbParamsT* params, unsigned int numSamples, unsigned int reset, void* cbContext)
{
	if (reset)
		printf("sdrplay_api_StreamBCallback: numSamples=%d\n", numSamples);
	// Process stream callback data here - this callback will only be used in dual tuner mode
	return;
}

void EventCallback(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT* params, void* cbContext)
{
	switch (eventId)
	{
	case sdrplay_api_GainChange:
		printf("sdrplay_api_EventCb: %s, tuner=%s gRdB=%d lnaGRdB=%d systemGain=%.2f\n",
			"sdrplay_api_GainChange", (tuner == sdrplay_api_Tuner_A) ? "sdrplay_api_Tuner_A" :
			"sdrplay_api_Tuner_B", params->gainParams.gRdB, params->gainParams.lnaGRdB,
			params->gainParams.currGain);
		break;
	case sdrplay_api_PowerOverloadChange:
		printf("sdrplay_api_PowerOverloadChange: tuner=%s powerOverloadChangeType=%s\n",
			(tuner == sdrplay_api_Tuner_A) ? "sdrplay_api_Tuner_A" : "sdrplay_api_Tuner_B",
			(params->powerOverloadParams.powerOverloadChangeType ==
				sdrplay_api_Overload_Detected) ? "sdrplay_api_Overload_Detected" :
			"sdrplay_api_Overload_Corrected");
		// Send update message to acknowledge power overload message received
		sdrplay_api_Update(chosenDevice->dev, tuner, sdrplay_api_Update_Ctrl_OverloadMsgAck,
			sdrplay_api_Update_Ext1_None);
		break;

	case sdrplay_api_RspDuoModeChange:
		printf("sdrplay_api_EventCb: %s, tuner=%s modeChangeType=%s\n",
			"sdrplay_api_RspDuoModeChange", (tuner == sdrplay_api_Tuner_A) ?
			"sdrplay_api_Tuner_A" : "sdrplay_api_Tuner_B",
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterInitialised) ?
			"sdrplay_api_MasterInitialised" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveAttached) ?
			"sdrplay_api_SlaveAttached" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveDetached) ?
			"sdrplay_api_SlaveDetached" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveInitialised) ?
			"sdrplay_api_SlaveInitialised" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveUninitialised) ?
			"sdrplay_api_SlaveUninitialised" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterDllDisappeared) ?
			"sdrplay_api_MasterDllDisappeared" :
			(params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveDllDisappeared) ?
			"sdrplay_api_SlaveDllDisappeared" : "unknown type");
		if (params->rspDuoModeParams.modeChangeType == sdrplay_api_MasterInitialised)
			masterInitialised = 1;
		if (params->rspDuoModeParams.modeChangeType == sdrplay_api_SlaveUninitialised)
			slaveUninitialised = 1;
		break;
	case sdrplay_api_DeviceRemoved:
		printf("sdrplay_api_EventCb: %s\n", "sdrplay_api_DeviceRemoved");
		break;
	default:
		printf("sdrplay_api_EventCb: %d, unknown event\n", eventId);
		break;
	}
}

void usage(void)
{
	printf("Usage: sdrplay_3_7_example.exe\n");
	exit(1);
}

int main(int argc, char* argv[])
{
	sdrplay_api_DeviceT devs[6];
	unsigned int ndev;
	int i;
	float ver = 0.0;
	sdrplay_api_ErrT err;
	sdrplay_api_DeviceParamsT* deviceParams = NULL;
	sdrplay_api_CallbackFnsT cbFns;
sdrplay_api_RxChannelParamsT* chParams;
int reqTuner = 0;
int master_slave = 0;
char c = '!';
unsigned int chosenIdx = 0;


printf("sizeof sdrplay_api_DevParamsT: %d\n", (int)sizeof(sdrplay_api_DevParamsT));
printf("sizeof ppm: %d\n", (int)sizeof(deviceParams->devParams->ppm));
printf("sizeof sdrplay_api_FsFreqT: %d\n", (int)sizeof(sdrplay_api_FsFreqT));
printf("sizeof sdrplay_api_SyncUpdateT: %d\n", (int)sizeof(sdrplay_api_SyncUpdateT));
printf("sizeof sdrplay_api_ResetFlagsT: %d\n", (int)sizeof(sdrplay_api_ResetFlagsT));
printf("sizeof mode: %d\n", (int)sizeof(deviceParams->devParams->mode));
printf("sizeof sdrplay_api_TransferModeT: %d\n", (int)sizeof(sdrplay_api_TransferModeT));
printf("sizeof sdrplay_api_Rsp1aParamsT: %d\n", (int)sizeof(sdrplay_api_Rsp1aParamsT));
printf("sizeof sdrplay_api_Rsp2ParamsT: %d\n", (int)sizeof(sdrplay_api_Rsp2ParamsT));
printf("sizeof sdrplay_api_RspduoParamsT: %d\n", (int)sizeof(sdrplay_api_RspDuoParamsT));
printf("sizeof sdrplay_api_RspDxParamsT: %d\n", (int)sizeof(sdrplay_api_RspDxParamsT));




printf("sizeof sdrplay_api_RxChannelParamsT: %d\n", (int)sizeof(sdrplay_api_RxChannelParamsT));
printf("sizeof sdrplay_api_TunerParamsT: %d\n", (int)sizeof(sdrplay_api_TunerParamsT));
printf("sizeof sdrplay_api_ControlParamsT: %d\n", (int)sizeof(sdrplay_api_ControlParamsT));
printf("sizeof sdrplay_api_Rsp1aTunerParamsT: %d\n", (int)sizeof(sdrplay_api_Rsp1aTunerParamsT));
printf("sizeof sdrplay_api_Rsp2TunerParamsT: %d\n", (int)sizeof(sdrplay_api_Rsp2TunerParamsT));
printf("sizeof sdrplay_api_RspDuoTunerParamsT: %d\n", (int)sizeof(sdrplay_api_RspDuoTunerParamsT));
printf("sizeof sdrplay_api_RspDxTunerParamsT: %d\n", (int)sizeof(sdrplay_api_RspDxTunerParamsT));

printf("sizeof sdrplay_api_GainT: %d\n", (int)sizeof(sdrplay_api_GainT));
printf("sizeof sdrplay_api_RfFreqT: %d\n", (int)sizeof(sdrplay_api_RfFreqT));
printf("sizeof sdrplay_api_DcOffsetTunerT: %d\n", (int)sizeof(sdrplay_api_DcOffsetTunerT));
printf("sizeof sdrplay_api_DcOffsetT: %d\n", (int)sizeof(sdrplay_api_DcOffsetT));



printf("sizeof sdrplay_api_DecimationT: %d\n", (int)sizeof(sdrplay_api_DecimationT));
printf("sizeof sdrplay_api_AgcT: %d\n", (int)sizeof(sdrplay_api_AgcT));
printf("sizeof sdrplay_api_AdsbModeT: %d\n", (int)sizeof(sdrplay_api_AdsbModeT));



// Open API
if ((err = sdrplay_api_Open()) != sdrplay_api_Success)
{
	printf("sdrplay_api_Open failed %s\n", sdrplay_api_GetErrorString(err));
	exit(1);
}
// Enable debug logging output
if ((err = sdrplay_api_DebugEnable(NULL, sdrplay_api_DbgLvl_Verbose)) != sdrplay_api_Success)
{
	printf("sdrplay_api_DebugEnable failed %s\n", sdrplay_api_GetErrorString(err));
}
// Check API versions match
if ((err = sdrplay_api_ApiVersion(&ver)) != sdrplay_api_Success)
{
	printf("sdrplay_api_ApiVersion failed %s\n", sdrplay_api_GetErrorString(err));
}
if (ver != SDRPLAY_API_VERSION)
{
	printf("API version don't match (local=%.2f dll=%.2f)\n", SDRPLAY_API_VERSION, ver);
	goto CloseApi;
}
// Lock API while device selection is performed
sdrplay_api_LockDeviceApi();
// Fetch list of available devices
if ((err = sdrplay_api_GetDevices(devs, &ndev, sizeof(devs) /
	sizeof(sdrplay_api_DeviceT))) != sdrplay_api_Success)
{
	printf("sdrplay_api_GetDevices failed %s\n", sdrplay_api_GetErrorString(err));
	goto UnlockDeviceAndCloseApi;
}
printf("MaxDevs=%d NumDevs=%d\n", sizeof(devs) / sizeof(sdrplay_api_DeviceT), ndev);
if (ndev > 0)
{
	for (i = 0; i < (int)ndev; i++)
	{
		if (devs[i].hwVer == SDRPLAY_RSPduo_ID)
			printf("Dev%d: SerNo=%s hwVer=%d tuner=0x%.2x rspDuoMode=0x%.2x\n", i,
				devs[i].SerNo, devs[i].hwVer, devs[i].tuner, devs[i].rspDuoMode);
		else
			printf("Dev%d: SerNo=%s hwVer=%d tuner=0x%.2x\n", i, devs[i].SerNo,
				devs[i].hwVer, devs[i].tuner);
	}
	// Choose the first device, if there is one
	if ((int)ndev > 0)
	{
		chosenIdx = 0;
		printf("chosenDevice = %d\n", chosenIdx);
		chosenDevice = &devs[chosenIdx];
	}
	else
	{
		printf("Couldn't find a suitable device to open - exiting\n");
		goto UnlockDeviceAndCloseApi;
	}

	// Disable Heartbeat for debugging
	//err = sdrplay_api_DisableHeartbeat();
	//printf("sdrplay_api_DiableHeartbeat: %s\n", sdrplay_api_GetErrorString(err));

		// Select chosen device
		if ((err = sdrplay_api_SelectDevice(chosenDevice)) != sdrplay_api_Success)
		{
			printf("sdrplay_api_SelectDevice failed %s\n", sdrplay_api_GetErrorString(err));
			goto UnlockDeviceAndCloseApi;
		}
		// Unlock API now that device is selected
		sdrplay_api_UnlockDeviceApi();

		// Retrieve device parameters so they can be changed if wanted
		if ((err = sdrplay_api_GetDeviceParams(chosenDevice->dev, &deviceParams)) !=
			sdrplay_api_Success)
		{
			printf("sdrplay_api_GetDeviceParams failed %s\n",
				sdrplay_api_GetErrorString(err));
			goto CloseApi;
		}
		// Check for NULL pointers before changing settings
		if (deviceParams == NULL)
		{
			printf("sdrplay_api_GetDeviceParams returned NULL deviceParams pointer\n");
			goto CloseApi;
		}
		// Configure dev parameters
		if (deviceParams->devParams != NULL)
		{
			// Change from default Fs to 8MHz
			deviceParams->devParams->fsFreq.fsHz = 8000000.0;
		}
		// Configure tuner parameters (depends on selected Tuner which parameters to use)
		chParams =  deviceParams->rxChannelA;
		if (chParams != NULL)
		{
			chParams->tunerParams.rfFreq.rfHz = 700000000.0;
			chParams->tunerParams.ifType = sdrplay_api_IF_Zero;
			chParams->tunerParams.gain.gRdB = 20;
			chParams->tunerParams.gain.LNAstate = 5;
			// Disable AGC
			chParams->ctrlParams.agc.enable = sdrplay_api_AGC_DISABLE;
		}
		else
		{
			printf("sdrplay_api_GetDeviceParams returned NULL chParams pointer\n");
			goto CloseApi;
		}
		// Assign callback functions to be passed to sdrplay_api_Init()
		cbFns.StreamACbFn = StreamACallback;
		cbFns.StreamBCbFn = StreamBCallback;
		cbFns.EventCbFn = EventCallback;

		// Now we're ready to start by calling the initialisation function
		// This will configure the device and start streaming
		if ((err = sdrplay_api_Init(chosenDevice->dev, &cbFns, NULL)) != sdrplay_api_Success)
		{
			printf("sdrplay_api_Init failed %s\n", sdrplay_api_GetErrorString(err));
			sdrplay_api_ErrorInfoT* errInfo = sdrplay_api_GetLastError(NULL);
			if (errInfo != NULL)
				printf("Error in %s: %s(): line %d: %s\n", errInfo->file, errInfo->function, errInfo->line, errInfo->message);
			goto CloseApi;
		}
		while (c != 'q') // Small loop allowing user to control gain reduction in +/-1dB steps using keyboard keys
		{
			if (_kbhit())
			{
				c = _getch();
				switch (c)
				{
					case 'q':
						break;
					case 'u':
					{
						//Increase gain reduction
						chParams->tunerParams.gain.gRdB += 1;
						// Limit it to a maximum of 59dB
						if (chParams->tunerParams.gain.gRdB > 59)
							chParams->tunerParams.gain.gRdB = 20;
						if ((err = sdrplay_api_Update(chosenDevice->dev, chosenDevice->tuner,
							sdrplay_api_Update_Tuner_Gr, sdrplay_api_Update_Ext1_None)) != sdrplay_api_Success)
						{
							printf("sdrplay_api_Update sdrplay_api_Update_Tuner_Gr failed %s\n",
								sdrplay_api_GetErrorString(err));
						}
						break;
					}
					case 'd':
					{
						// Reduce Gain Reduction
						chParams->tunerParams.gain.gRdB -= 1;
						// Limit it to a minimum of 20dB
						if (chParams->tunerParams.gain.gRdB < 20)
							chParams->tunerParams.gain.gRdB = 59;
						if ((err = sdrplay_api_Update(chosenDevice->dev, chosenDevice->tuner,
							sdrplay_api_Update_Tuner_Gr, sdrplay_api_Update_Ext1_None)) != sdrplay_api_Success)
						{
							printf("sdrplay_api_Update sdrplay_api_Update_Tuner_Gr failed %s\n",
								sdrplay_api_GetErrorString(err));
						}
						break;
					}
					case 'n':
					{
						boolean bRfNotchSwitch = deviceParams->devParams->rspDxParams.rfNotchEnable == 1;
						deviceParams->devParams->rspDxParams.rfNotchEnable = bRfNotchSwitch ? 0 : 1;
						if ((err = sdrplay_api_Update(chosenDevice->dev, chosenDevice->tuner,
							sdrplay_api_Update_None, sdrplay_api_Update_RspDx_RfNotchControl)) != sdrplay_api_Success)
						{
							printf("sdrplay_api_Update sdrplay_api_Update_Tuner_Gr failed %s\n",
								sdrplay_api_GetErrorString(err));
						}
						break;
					}
					case 'l':
					{
						// Set LNA state to 5
						chParams->tunerParams.gain.LNAstate = 6;
						if ((err = sdrplay_api_Update(chosenDevice->dev, chosenDevice->tuner,
							sdrplay_api_Update_Tuner_Gr, sdrplay_api_Update_Ext1_None)) != sdrplay_api_Success)
						{
							printf("sdrplay_api_Update LNAstate failed %s\n",
								sdrplay_api_GetErrorString(err));
						}
						break;
					}
				}
				Sleep(100);
			}
		}
		// Finished with device so uninitialise it
		if ((err = sdrplay_api_Uninit(chosenDevice->dev)) != sdrplay_api_Success)
		{
			printf("sdrplay_api_Uninit failed %s\n", sdrplay_api_GetErrorString(err));
			goto CloseApi;
		}
		// Release device (make it available to other applications)
		sdrplay_api_ReleaseDevice(chosenDevice);

UnlockDeviceAndCloseApi:
// Unlock API
sdrplay_api_UnlockDeviceApi();
CloseApi:
// Close API
sdrplay_api_Close();
	}
return 0;
}