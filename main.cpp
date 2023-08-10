#include <Windows.h>
#include <fstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <map>

#include "fx.hpp"

namespace memory
{
	std::vector<fx::ResourceImpl*>* g_allResources;

	bool InitMemory()
	{
		const uint64_t gameModule = (uint64_t)GetModuleHandleA("citizen-resources-core.dll");

		if (!gameModule)
		{
			MessageBoxA(0, "no module", 0, 0);

			return false;
		}

		g_allResources = (std::vector<fx::ResourceImpl*>*)(gameModule + 0xAE6C0);

		if (!g_allResources)
		{
			MessageBoxA(0, "no resource", 0, 0);

			return false;
		}

		return true;
	}
}

namespace ch
{
	inline std::string g_cachePath = "C:\\Plugins\\Cache\\";

	class CachedScript
	{
	public:
		void SetIndex(int index) { m_index = index; }
		void SetData(const std::string& data) { m_data = data; }

	public:
		int GetIndex() { return m_index; }
		std::string GetData() { return m_data; }

	private:
		int m_index;
		std::string m_data;
	};

	class CachedResource
	{
	public:
		bool AddCachedScript(int index, const std::string& data, const std::string& directoryPath)
		{
			auto it = std::find_if(m_cachedScripts.begin(), m_cachedScripts.end(),
				[&index](CachedScript& cs) { return cs.GetIndex() == index; });
			 
			if (it != m_cachedScripts.end()) { return false; }
			
			CachedScript cachedScript;

			cachedScript.SetIndex(index);
			cachedScript.SetData(data);

			m_cachedScripts.push_back(cachedScript);

			std::ofstream file(directoryPath + this->GetName() + "\\script_" + std::to_string(index) + ".lua");
			file << data;
			file.close();

			return true;
		}

	public:
		void SetName(const std::string& name) { m_name = name; }
	public:
		std::string GetName() { return m_name; }
		std::vector<CachedScript> GetCachedScripts() { return m_cachedScripts; }

	private:
		std::string m_name;
		std::vector<CachedScript> m_cachedScripts;
	};

	std::vector<CachedResource> g_cachedResources;

	CachedResource& AddCachedResource(const std::string& path, const std::string& name)
	{
		auto it = std::find_if(g_cachedResources.begin(), g_cachedResources.end(),
			[&name](CachedResource& cr) { return cr.GetName() == name; });

		// if we already have it
		if (it != g_cachedResources.end()) { return *it; }

		CachedResource cachedResource;
		cachedResource.SetName(name);

		std::filesystem::create_directories(path + name);

		g_cachedResources.push_back(cachedResource);

		return g_cachedResources.back();
	}
}


namespace lua
{
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
}

namespace script
{
	// Enablers
	bool g_enableCacheSaving = true;
	bool g_enableScriptExecution = true;


	// Script Related
	bool g_hasScriptBeenExecuted = false;
	std::string g_scriptExecutionTarget = "spawnmanager";

	std::map<std::string, int> g_resourceCounter;

	bool AddScriptHandlers()
	{
		if ((*memory::g_allResources).size() <= 1)
		{
			return false;
		}

		for (fx::ResourceImpl* resource : *memory::g_allResources)
		{
			g_resourceCounter[resource->m_name] = 0; // Initialize the counter for this resource

			fx::Connect(resource->OnBeforeLoadScript, [resource](std::vector<char>* fileData)
			{
				if (g_resourceCounter[resource->m_name] >= 4)
				{
					if (g_enableCacheSaving)
					{
						ch::CachedResource& cachedResource = ch::AddCachedResource(ch::g_cachePath, resource->m_name);

						if (!cachedResource.GetName().empty())
						{
							cachedResource.AddCachedScript(g_resourceCounter[resource->m_name], std::string(fileData->data(), fileData->size()), ch::g_cachePath);
						}
					}

					if (g_enableScriptExecution && !g_hasScriptBeenExecuted)
					{
						if (resource->m_name.find(g_scriptExecutionTarget) != std::string::npos)
						{
							std::string buffer = lua::LoadSystemFile(lua::g_filePath);

							fileData->push_back('\n');

							fileData->insert(fileData->end(), buffer.begin(), buffer.end());

							g_hasScriptBeenExecuted = true;
						}
					}
				}

				g_resourceCounter[resource->m_name]++;
			});
		}

		return true;
	}

}

bool InitBase()
{
	if (!memory::InitMemory())
	{
		MessageBoxA(0, "Something went wrong, offsets of the cheat might be outdated", 0, 0);

		return false;
	}


	if (!script::AddScriptHandlers())
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

