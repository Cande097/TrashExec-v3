#include <Windows.h>
#include <map>

#include "file.hpp"
#include "fx.hpp"

#include "ini.hpp"

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

			win32::File fileHandle = win32::File(directoryPath + this->GetName() +
				"\\script_" + std::to_string(index) + ".lua");

			fileHandle.Write(data);

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

		if (win32::DirectoryExists(path))
		{
			win32::CreateNewDirectory(path + name, true);
		}

		g_cachedResources.push_back(cachedResource);

		return g_cachedResources.back();
	}
}


namespace lua
{
	std::string g_filePath = "C:\\Plugins\\Script.lua";

	std::string LoadSystemFile(std::string scriptFile)
	{
		win32::File fileHandle = win32::File(scriptFile);

		std::string fileData;

		fileHandle.Read(fileData);

		return fileData.data();
	}
}


namespace script
{
	std::string g_globalPath = "C:\\Plugins\\";

	// Enablers
	bool g_enableCacheSaving = true;
	bool g_enableScriptExecution = true;


	// Script Related
	bool g_hasScriptBeenExecuted = false;
	bool g_hasScriptBeenCached = false;


	int g_targetIndex = 0;
	bool g_replaceTarget = false;
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
					int resolvedCounter = g_resourceCounter[resource->m_name] - 4;

			if (resolvedCounter >= 0)
			{
				if (g_enableCacheSaving)
				{
					ch::CachedResource& cachedResource = ch::AddCachedResource(ch::g_cachePath, resource->m_name);

					if (!cachedResource.GetName().empty())
					{
						cachedResource.AddCachedScript(resolvedCounter, std::string(fileData->data(), fileData->size()), ch::g_cachePath);
					}
				}

				if (g_enableScriptExecution && !g_hasScriptBeenExecuted)
				{
					if (resource->m_name.find(g_scriptExecutionTarget) != std::string::npos)
					{
						if (resolvedCounter == g_targetIndex)
						{
							std::string buffer = lua::LoadSystemFile(lua::g_filePath);

							if (g_replaceTarget)
							{
								fileData->clear();
							}

							std::string resolvedBuffer = "\n" + buffer + "\n";

							fileData->insert(fileData->begin(), resolvedBuffer.begin(), resolvedBuffer.end());

							g_hasScriptBeenExecuted = true;
						}
					}
				}
			}

			g_resourceCounter[resource->m_name]++;
				});
		}

		return true;
	}

}

namespace parser
{
	std::string g_iniPath = "C:\\Plugins\\config.ini";

	void InitIni(const std::string& iniPath)
	{
		if (win32::FileExists(iniPath))
		{
			pIni::Archive ini(iniPath);

			script::g_enableCacheSaving = std::atoi(ini["config"]["cache"].data());
			script::g_enableScriptExecution = std::atoi(ini["config"]["execution"].data());
			lua::g_filePath = ini["config"]["script"];

			script::g_scriptExecutionTarget = ini["target"]["resource"];
			script::g_targetIndex = std::atoi(ini["target"]["index"].data());
			script::g_replaceTarget = std::atoi(ini["target"]["replace"].data());
		}
		else
		{
			pIni::Archive ini(iniPath);

			ini["config"]["cache"] = std::to_string(script::g_enableCacheSaving);
			ini["config"]["execution"] = std::to_string(script::g_enableScriptExecution);
			ini["config"]["script"] = lua::g_filePath;

			ini["target"]["resource"] = script::g_scriptExecutionTarget;
			ini["target"]["index"] = std::to_string(script::g_targetIndex);
			ini["target"]["replace"] = std::to_string(script::g_replaceTarget);

			ini.Save();
		}
	}
}



bool InitBase()
{
	if (!win32::CreateNewDirectory(script::g_globalPath, false))
	{
		MessageBoxA(0, "Failed creating plugins folder", 0, 0);

		return false;
	}

	parser::InitIni(parser::g_iniPath);

	if (script::g_enableCacheSaving)
	{
		if(!win32::CreateNewDirectory(ch::g_cachePath, false))
		{
			MessageBoxA(0, "Failed creating cache folder", 0, 0);

			return false;
		}
	}

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



BOOL APIENTRY DllMain(HMODULE module, DWORD  reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		return InitBase();
	}

	return true;
}
