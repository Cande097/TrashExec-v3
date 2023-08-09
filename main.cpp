#include "fx.hpp"

#include <cstdint>
#include <fstream>

#include <Windows.h>

using namespace std;

using namespace fx;

namespace memory
{

	vector<fx::ResourceImpl*>* g_allResources;

	bool InitMemory()
	{
		
		const uint64_t gameModule = static_cast<uint64_t>(GetModuleHandleA("citizen-resources-core.dll")); 
		
		if (!gameModule)
		{
			MessageBoxA(0, "no module", 0, 0);
			return false;
		}

		if (g_allResources = (vector<ResourceImpl*>*)(gameModule + 0xAE6C0); !g_allResources)
		{
			MessageBoxA(0, "no resource", 0, 0);
			return false;
		}

		return true;
	}
}

namespace lua
{
	bool g_hasBeenExecuted = false;
	int g_fileLoadCounter = 0;
	string g_filePath = "C:\\Plugins\\script.lua";

	string LoadSystemFile(const string& scriptFile)
	{
		ifstream file(scriptFile, ifstream::ate | ifstream::binary);
		file.seekg(0, ifstream::end);
		streampos length = file.tellg();
		file.seekg(0, ifstream::beg);

		vector<char> fileData(length);
		file.read(&fileData[0], length);
		fileData.push_back('\0');

		file.close();

		return &fileData[0];
	}

	bool InitLua()
	{
		bool hasBeenFound = false;

		for (fx::ResourceImpl* resource : *memory::g_allResources)
		{
			if (resource->m_name.find("spawnmanager") == string::npos)
			{
				continue;
			}

			fx::Connect(resource->OnBeforeLoadScript, [&](vector<char>* fileDatas)
			{
				if (g_fileLoadCounter == 4 && !g_hasBeenExecuted) // 4 startup files don't blame me
				{
					string buffer = LoadSystemFile(g_filePath);

					fileDatas->push_back('\n');

					fileDatas->insert(fileDatas->end(), buffer.begin(), buffer.end()); // Add the string

					g_hasBeenExecuted = true;
				}

				g_fileLoadCounter++;
			});

			hasBeenFound = true;
		}

		return hasBeenFound;
	}
}


bool InitBase()
{
	if (!memory::InitMemory())
	{
		MessageBoxA(0, "Something went wrong, offsets of the cheat might be outdated", 0, 0);

		return false;
	}


	if (!lua::InitLua())
	{
		MessageBoxA(0, "Something went wrong, inject while joining to a server", 0, 0);

		return false;
	}

	return true;
}


BOOL APIENTRY DllMain( HMODULE module, DWORD  reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		return InitBase();
	}

    return true;
}

