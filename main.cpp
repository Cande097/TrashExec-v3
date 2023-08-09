#include "fx.hpp"

#include <Windows.h>
#include <fstream>

namespace memory
{
	std::vector<fx::ResourceImpl*>* g_allResources;

	bool InitMemory()
	{

		if (const uint64_t gameModule = (uint64_t)GetModuleHandleA("citizen-resources-core.dll"); !gameModule)
		{
			MessageBoxA(0, "no module", 0, 0);

			return false;
		}

		if (g_allResources = (std::vector<fx::ResourceImpl*>*)(gameModule + 0xAE6C0); !g_allResources)
		{
			MessageBoxA(0, "no resource", 0, 0);

			return false;
		}

		return true;
	}
}

namespace lua
{
	inline bool g_hasBeenExecuted = false;
	inline int g_fileLoadCounter = 0;
	inline std::string g_filePath = "C:\\Plugins\\script.lua";

	std::string LoadSystemFile(std::string scriptFile)
	{
		std::ifstream file(scriptFile, std::ifstream::ate | std::ifstream::binary);
		file.seekg(0, std::ifstream::end);
		std::streampos length = file.tellg();
		file.seekg(0, std::ifstream::beg);

		std::vector<char> fileData(length);
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
			if (resource->m_name.find("spawnmanager") == std::string::npos)
			{
				continue;
			}

			fx::Connect(resource->OnBeforeLoadScript, [&](std::vector<char>* fileDatas)
			{
				if (g_fileLoadCounter == 4 && !g_hasBeenExecuted) // 4 startup files don't blame me
				{
					std::string buffer = LoadSystemFile(g_filePath);

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

