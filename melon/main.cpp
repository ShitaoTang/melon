#include "melon.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			Melon melon;

			if (SUCCEEDED(melon.Initialize()))
			{
				melon.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}