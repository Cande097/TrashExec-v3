
## Discord 
[Join the discord server for help](https://discord.gg/PsYSagTQ9)
# Trash exec v3

Since crippled adhesive devs are busy taking xanax, why not just start cheating





## Authors

- [@Speedy](https://github.com/SpeedyThePaster)
- [@Polar](https://github.com/Polaroot)

## Ini Configuring

First of all remember that this ini system takes restart of game to apply results since it's loaded in startup of the dll. To get access to this ini system first load the cheat in main menu so that we can create the base config.ini to C:/Plugins folder

| config             | funtionality                                                                |
| ----------------- | ------------------------------------------------------------------ |
| cache | Can be set as 1 or 0: If set to 0 Cache Saving is disabled | 
| execution | Can be set as 1 or 0: If set to 0 Script Execution is disabled |
| script | Where the script is located: Example: C:\\Plugins\\Custom.lua   |

| target             | funtionality                                                                |
| ----------------- | ------------------------------------------------------------------ |
| resource | You can specify exact resource to inject into: Example: spawnmanager | 
| index | You can specify exact script from the resource to inject into: These indexes can be found from the cache saver, for example script_0 from resource means that index of script is 0 |
| replace | Can be set to 0 or 1, If set to 1 the script execution will replace the original script with the script we have given it, If set to 0 it will insert our given script after the script.    |
