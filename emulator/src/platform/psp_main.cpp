#ifdef PLATFORM_PSP

#include "emulator.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

#include <pspkernel.h>
#include <pspdisplay.h>

static int exitRequest  = 1;

int isRunning() 
{
	return exitRequest;
}

int exitCallback(int arg1, int arg2, void *common)
 {
	exitRequest = 0; 
	return 0;
} 

int callbackThread(SceSize args, void *argp) 
{
	int callbackID = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(callbackID);
	sceKernelSleepThreadCB();
	return 0;
} 

int setupExitCallback() 
{
	int threadID = sceKernelCreateThread("Callback Update Thread", callbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if (threadID >= 0)
	{
		sceKernelStartThread(threadID, 0, 0);
	}
	return threadID; 
}

PSP_MODULE_INFO("gnw_emu", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER); 
PSP_HEAP_SIZE_MAX();

int main() 
{       
	setupExitCallback();

	//while (isRunning())
	//{
	//	pspDebugScreenSetXY(0, 0);
	//	printf("Hello World!\n");
	//	printf("Hello World!\n");
	//	printf("Hello World!\n");
	//	sceDisplayWaitVblankStart();
	//}

	Emulator emulator;
	emulator.Init();
	emulator.Loop();
	emulator.Free();

	sceKernelExitGame();	
	return 0;
}

//#ifdef __cplusplus
//}
//#endif
#endif
